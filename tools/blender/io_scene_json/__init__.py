bl_info = {
	"name": "Weep json scene format",
	"author": "Sampsa Vierros",
	"blender": (2, 70, 0),
	"location": "File > Import-Export",
	"description": "",
	"warning": "",
	"wiki_url": "",
	"tracker_url": "",
	"version": (0, 2),
	"category": "Import-Export"}

import bpy
from bpy_extras.io_utils import ExportHelper
from bpy.props import *
import mathutils

from collections import OrderedDict
import json
import struct
import string
import math
import ctypes

# Known bugs:
# - Group name and object name must be same

def B2GL(obj):
	# Convert Blender coordinates to weep
	if isinstance(obj, mathutils.Quaternion):
		return [obj.x, obj.z, -obj.y, obj.w]
	elif isinstance(obj, mathutils.Vector) and len(obj) == 3:
		return [obj[0], obj[2], -obj[1]]
	else:
		return obj

def is_all_one(vec):
	for elem in vec:
		if abs(elem-1.0) > 1e-6:
			return False
	return True

def is_all_zero(vec):
	if vec.length > 1e-6:
		return False
	return True


class BlenderEncoder(json.JSONEncoder):
	def default(self, obj):
		if isinstance(obj, mathutils.Vector):
			return [elem for elem in obj]
		if isinstance(obj, mathutils.Color):
			return [elem for elem in obj]
		if isinstance(obj, mathutils.Quaternion):
			return [obj[0], obj[1], obj[2], obj[3]]
		return json.JSONEncoder.default(self, obj)


def gatherGroupInfo(group, prefabs):
	prefab = OrderedDict()
	for obj in filter(lambda x: isinstance(x, bpy.types.Object), bpy.data.libraries[0].users_id):
		if obj.name != group or obj.type != 'MESH':
			continue
		mat = obj.active_material
		material = OrderedDict()
		prefab["material"] = material
		# Regular color channels and flags
		material["diffuse"] = mat.diffuse_color*mat.diffuse_intensity
		material["specular"] = mat.specular_color*mat.specular_intensity
		material["shininess"] = mat.specular_hardness
		material["alphaTest"] = mat.use_transparency
		material["reflectivity"] = mat.raytrace_mirror.reflect_factor
		material["castShadow"] = mat.use_cast_shadows
		material["receiveShadow"] = mat.use_shadows
		# These do not have their own color in blender internal
		material["emissive"] = mat.emit*mat.diffuse_color
		material["ambient"] = mat.ambient*mat.diffuse_color
		# Parse textures
		for slot in filter(lambda x: x != None, mat.texture_slots):
			if not slot.use:
				continue
			# Resolve image path
			texture = slot.texture
			if texture.type != 'IMAGE':
				continue
			image = texture.image
			filename = ''
			if image:
				filename = image.filepath
			# Resolve channel
			if slot.use_map_diffuse or slot.use_map_color_diffuse:
				material["diffuseMap"] = filename
				if slot.use_map_alpha:
					material["alphaTest"] = True # Force alpha on
				# Use diffuse channel for texture scaling and offsetting.
				if not is_all_zero(slot.offset):
					material["uvOffset"] = slot.offset[:2]
				repeat = [texture.repeat_x, texture.repeat_y]
				if not is_all_one(repeat):
					material["uvRepeat"] = repeat
			elif slot.use_map_color_spec:
				material["specularMap"] = filename
			elif slot.use_map_normal or texture.use_normal_map:
				material["normalMap"] = filename
			elif slot.use_map_ambient:
				material["aoMap"] = filename
			elif slot.use_map_displacement:
				material["heightMap"] = filename
				material["parallax"] = slot.displacement_factor
			elif slot.use_map_emit:
				material["emissionMap"] = filename
			elif slot.use_map_mirror or slot.use_map_raymir:
				material["reflectionMap"] = filename
		# Physics info
		if obj.rigid_body:
			body = obj.rigid_body
			types = {'BOX' : 'box', \
				'SPHERE' : 'sphere', \
				'CYLINDER' : 'cylinder', \
				'CONE' : 'cone', \
				'CAPSULE' : 'capsule', \
				'CONVEX_HULL' : 'hull', \
				'MESH' : 'trimesh'}
			prefab["body"] = {'shape' : types[body.collision_shape], \
				'mass' : body.mass, 'friction' : body.friction}
	prefabs[group] = prefab

def gatherLightInfo(lamp, entity):
	entity["light"] = OrderedDict()
	light = entity["light"]
	light["color"] = lamp.color
	# Shadows (mostly unused)
	castsShadows = False
	if lamp.shadow_method != 'NOSHADOW':
		castsShadows = True
		shadowColor = lamp.shadow_color
	if lamp.shadow_method == 'BUFFER_SHADOW':
		near = lamp.shadow_buffer_clip_start
		far = lamp.shadow_buffer_clip_end
	# Light variety
	if lamp.type == 'POINT':
		light["type"] = 'point'
		light["distance"] = lamp.distance
		# No easy way to include decay
	elif lamp.type == 'SUN':
		# Directional light
		pass
	elif lamp.type == 'SPOT':
		spot_angle = lamp.spot_size

def exportScene(context, operator, filepath):
	# Switch out of editmode
	mode = bpy.context.mode
	if mode == 'EDIT_MESH':
		bpy.ops.object.editmode_toggle()

	# Prefill weep scene header
	hierarchy = OrderedDict()
	hierarchy["include"] = "prefabs.json"
	hierarchy["objects"] = []
	objects = hierarchy["objects"]
	# Group specific data is saved separately to a library
	prefabs = OrderedDict()

	scene = context.scene
	for obj in scene.objects:
		loc, rot, scale = obj.matrix_world.decompose()
		entity = OrderedDict()
		# Blender axis x z -y
		entity['position'] = B2GL(loc)
		entity['rotation'] = B2GL(rot)
		if not is_all_one(scale):
			entity['scale'] = B2GL(scale)
		# Skip local geometry
		if obj.type == 'MESH':
			continue
		# Empty objects carry linked geometry
		elif obj.type == 'EMPTY':
			if obj.dupli_type == 'NONE':
				# Skip empties that do not link to a group
				continue
			if obj.dupli_type == 'GROUP':
				group = obj.dupli_group.name
				entity['prefab'] = group
				if group not in prefabs:
					gatherGroupInfo(group, prefabs)
				objects.append(entity)
		elif obj.type == 'LAMP':
			lamp = obj.data
			gatherLightInfo(lamp, entity)
			objects.append(entity)
		elif obj.type == 'CAMERA':
			# Placeholder
			camera = obj.data
			entity['prefab'] = "camera"
			variety = camera.type
			fov = camera.angle
			near = camera.clip_start
			far = camera.clip_end
			objects.append(entity)
		elif obj.type == 'CURVE':
			# Not yet in use (could be useful for waypoints later on)
			curve = obj.data
			for spline in curve.splines:
				if spline.type != 'NURBS':
					continue
				points = [point.co for point in spline.points]

	# Scene writeback
	data = json.dumps(hierarchy, sort_keys=False, indent=4, separators=(',', ': '), cls=BlenderEncoder)
	fp = open(filepath, 'w')
	fp.write(data)
	fp.close()

	# Library writeback
	data = json.dumps({"prefabs" : prefabs}, sort_keys=False, indent=4, separators=(',', ': '), cls=BlenderEncoder)
	fp = open(filepath+'.library', 'w')
	fp.write(data)
	fp.close()

################################################################################
class ExportWeepJson(bpy.types.Operator, ExportHelper):
	bl_idname = "export_scene.json"
	bl_desciption = ''
	bl_label = "Export scene to Weep scene description"
	bl_space_type = "Properties"
	bl_region_type = "WINDOW"

	filename_ext = ".json"
	filter_glob = StringProperty(default="*.json", options={'HIDDEN'})
	filepath = bpy.props.StringProperty(name="File path", default="~/")

	def execute(self, context):
		exportScene(context, self, self.properties.filepath);
		return {'FINISHED'}

	def invoke(self, context, event):
		context.window_manager.fileselect_add(self)
		return {'RUNNING_MODAL'}

def menu_func(self, context):
    self.layout.operator(ExportWeepJson.bl_idname, text="Export scene (.json)")


def register():
	bpy.utils.register_class(ExportWeepJson)
	bpy.types.INFO_MT_file_export.append(menu_func)

def unregister():
	bpy.utils.unregister_class(ExportWeepJson)
	bpy.types.INFO_MT_file_export.remove(menu_func)

if __name__ == "__main__":
	register()



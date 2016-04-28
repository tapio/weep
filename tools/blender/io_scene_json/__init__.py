bl_info = {
	"name": "Weep JSON Scene Format",
	"author": "Sampsa Vierros",
	"blender": (2, 70, 0),
	"location": "File > Import-Export",
	"description": "Export scene to Weep Engine's JSON format",
	"warning": "",
	"wiki_url": "",
	"tracker_url": "",
	"version": (0, 31),
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
import os.path

MAX_LAYERS = 20

# Known bugs:
# - Group name and object name must be same
# - Particle system "pick random" feature does not work, must create separate
#   system for each object
# - Automatic mesh exporting does not work for non-local groups
# - Unsaved files save geometry to home path

def B2GL(obj, scale=False):
	# Convert Blender coordinates to weep
	if isinstance(obj, mathutils.Quaternion):
		return [obj.x, obj.z, -obj.y, obj.w]
	elif isinstance(obj, mathutils.Vector) and len(obj) == 3:
		if scale != True:
			return [obj[0], obj[2], -obj[1]]
		else:
			return [obj[0], obj[2], obj[1]]
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

def gatherObjectInfo(obj, prefab, export_mesh = False, workdir=''):
	# Argument prefab can be Weep object block or prefab block
	# Grab geometry source from custom string property, otherwise guess
	if "geometry" in obj.items():
		prefab["geometry"] = str(obj["geometry"])
	else:
		prefab["geometry"] = obj.name+'.obj'
	# Export geometry to Wavefront .obj file if appropriate
	if export_mesh:
		# This blob moves object to origin, removes rotation and scale, and
		# moves the object to layer one. Used to negate certain aspects of
		# .obj exporter.
		obj.select = True
		transform = obj.matrix_world.copy()
		loc, rot, scale = obj.location.copy(), obj.rotation_quaternion.copy(), obj.scale.copy()
		obj.location = mathutils.Vector([0, 0, 0])
		obj.scale = mathutils.Vector([1, 1, 1])
		obj.rotation_quaternion = mathutils.Quaternion([1, 0, 0, 0])
		moved = False
		if not obj.layers[0]: 
			moved = True
			obj.layers[0] = True
		bpy.ops.export_scene.obj(filepath=workdir+'/'+prefab["geometry"], \
			axis_forward='-Z', axis_up='Y', use_selection=True,  \
			use_materials=False, use_triangles=True)
		obj.select = False
		obj.location, obj.rotation_quaternion, obj.scale = loc, rot, scale
		if moved:
			obj.layers[0] = False
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
		if obj.rigid_body.enabled != True:
			prefab["body"]["mass"] = 0
	if not obj.active_material:
		return 
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
			filename = bpy.path.basename(image.filepath)
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

def gatherGroupInfo(group, prefabs, export_mesh=False, workdir=''):
	prefab = OrderedDict()
	for obj in filter(lambda x: isinstance(x, bpy.types.Object), bpy.data.objects):
		if obj.name != group or obj.type != 'MESH':
			continue
		gatherObjectInfo(obj, prefab, export_mesh, workdir)
		prefabs[group] = prefab
		return
	for obj in filter(lambda x: isinstance(x, bpy.types.Object), bpy.data.libraries[0].users_id):
		if obj.name != group or obj.type != 'MESH':
			continue
		gatherObjectInfo(obj, prefab, export_mesh, workdir)
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

def exportScene(context, operator, filepath, export_library, export_meshes):
	workdir = os.path.dirname(bpy.data.filepath)
	# Switch out of editmode
	mode = bpy.context.mode
	if mode == 'EDIT_MESH':
		bpy.ops.object.editmode_toggle()
	selected = context.selected_objects
	bpy.ops.object.select_all(action='DESELECT')
	
	# Prefill weep scene header
	hierarchy = OrderedDict()
	hierarchy["include"] = "debugprefabs.json"
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
			entity['scale'] = B2GL(scale, True)
		# Local geometry goes to scene file and can carry nice particle systems
		if obj.type == 'MESH':
			if obj.hide_render == True:
				continue
			gatherObjectInfo(obj, entity, export_meshes, workdir)
			objects.append(entity)
			# Handle particle system based object instancing
			if len(obj.particle_systems) != 0:
				for system in obj.particle_systems:
					settings = system.settings
					if settings.render_type != 'GROUP':
						continue
					group = settings.dupli_group.name
					for particle in system.particles:
						if particle.alive_state != 'ALIVE':
							continue
						entity = OrderedDict()
						entity['position'] = B2GL(particle.location)
						entity['rotation'] = B2GL(particle.rotation) # not quaternion
						entity['scale'] = [particle.size]*3
						entity['prefab'] = group
						objects.append(entity)
					if group not in prefabs:
						gatherGroupInfo(group, prefabs, export_meshes, workdir)
		# Empty objects carry linked geometry
		elif obj.type == 'EMPTY':
			if obj.dupli_type == 'NONE':
				# Skip empties that do not link to a group
				continue
			if obj.dupli_type == 'GROUP':
				group = obj.dupli_group.name
				entity['prefab'] = group
				if group not in prefabs:
					gatherGroupInfo(group, prefabs, export_meshes, workdir)
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

	idx = filepath.rfind('.json')
	if  idx != -1: libpath = filepath[:idx] + '.library.json'
	else: libpath = filepath + '.library'
	hierarchy["include"] = bpy.path.basename(libpath)
	# Scene writeback
	data = json.dumps(hierarchy, sort_keys=False, indent=4, separators=(',', ': '), cls=BlenderEncoder)
	data = data.replace("    ", "\t");
	fp = open(filepath, 'w')
	fp.write(data)
	fp.close()

	# Library writeback
	if export_library:
		data = json.dumps({"prefabs" : prefabs}, sort_keys=False, indent=4, separators=(',', ': '), cls=BlenderEncoder)
		data = data.replace("    ", "\t");
		fp = open(libpath, 'w')
		fp.write(data)
		fp.close()

################################################################################
class ExportWeepJson(bpy.types.Operator, ExportHelper):
	bl_idname = "export_scene.json"
	bl_desciption = ''
	bl_label = "Export Weep JSON"
	bl_space_type = "Properties"
	bl_region_type = "WINDOW"

	filename_ext = ".json"
	filter_glob = StringProperty(default="*.json", options={'HIDDEN'})
	filepath = bpy.props.StringProperty(name="File path", default="~/")
	export_library = bpy.props.BoolProperty(name="Export library", \
		description="Export prefabs to library file.", default=False)
	export_meshes = bpy.props.BoolProperty(name="Export meshes", \
		description="Export all geometry.", default=False)
	
	def execute(self, context):
		exportScene(context, self, self.properties.filepath, \
			self.properties.export_library, self.properties.export_meshes)
		return {'FINISHED'}

	def invoke(self, context, event):
		context.window_manager.fileselect_add(self)
		return {'RUNNING_MODAL'}

def menu_func(self, context):
    self.layout.operator(ExportWeepJson.bl_idname, text="Weep Engine (.json)")

def register():
	bpy.utils.register_class(ExportWeepJson)
	bpy.types.INFO_MT_file_export.append(menu_func)

def unregister():
	bpy.utils.unregister_class(ExportWeepJson)
	bpy.types.INFO_MT_file_export.remove(menu_func)

if __name__ == "__main__":
	register()




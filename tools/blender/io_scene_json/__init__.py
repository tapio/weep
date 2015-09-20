bl_info = {
	"name": "Weep json scene format",
	"author": "Sampsa Vierros",
	"blender": (2, 70, 0),
	"location": "File > Import-Export",
	"description": "",
	"warning": "",
	"wiki_url": "",
	"tracker_url": "",
	"version": (0, 1),
	"category": "Import-Export"}

import bpy
from bpy_extras.io_utils import ExportHelper
from bpy.props import *
import mathutils

import struct
import string
import math
import ctypes


class WeepSceneWriter:
	def __init__(self, filePath):
		self.stream = open(filePath, 'w')
		self.indentationLevel = 0
	def __del__(self):
		self.stream.close()
	def indent(self):
		self.indentationLevel += 1
	def deindent(self):
		self.indentationLevel -= 1
	def writeLine(self, line):
		tmp = ''
		for i in range(self.indentationLevel):
			tmp += '\t'
		tmp += line
		self.stream.write(tmp)
	def writeObject(self, prefab, position, orientation, scale):
		self.writeLine('"prefab": "%s",\n' % prefab)
		self.writeLine('"position": [ %f, %f, %f ],\n' % (position[0], position[2], -position[1]))
		#self.writeLine('"rotation": [ %f, %f, %f, %f ],\n' % (orientation[0], orientation[1], orientation[3], -orientation[2]))
		self.writeLine('"scale": [ %f, %f, %f ]\n' % (scale[0], scale[2], scale[1]))


def exportScene(context, operator, filepath):
	# Switch out of editmode
	mode = bpy.context.mode
	if mode == 'EDIT_MESH':
		bpy.ops.object.editmode_toggle()

	entities = []
	scene = context.scene
	for obj in scene.objects:
		loc, rot, scale = obj.matrix_world.decompose()
		entity = {}
		# Blender axis x z -y
		entity['pos'] = loc # vec3
		entity['rot'] = rot # vec4 (quat)
		entity['scale'] = scale	# vec3
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
				entities.append(entity)
		elif obj.type == 'LAMP':
			lamp = obj.data
			# Commons
			color = lamp.color                   # vec3
			energy = lamp.energy                 # float
			# Shadows	
			castsShadows = False
			if lamp.shadow_method != 'NOSHADOW':
				castsShadows = True
				shadowColor = lamp.shadow_color      # vec3
			if lamp.shadow_method == 'BUFFER_SHADOW':
				near = lamp.shadow_buffer_clip_start # float
				far = lamp.shadow_buffer_clip_end    # float
			# Falloffs (floats)
			#constant = 0.0
			#linear = lamp.linear_attenuation
			#quadratic = lamp.quadratic_attenuation
			if lamp.type == 'POINT':
				variety = 'point'				
				distance = lamp.distance             # float
			elif lamp.type == 'SUN':
				variety = 'directional'
			elif lamp.type == 'SPOT':
				variety = 'spot'
				distance = lamp.distance             # float
				spot_angle = lamp.spot_size          # float (in radians)
				inner_spot_angle = spot_angle*lamp.spot_blend # INCORRECT
		elif obj.type == 'CAMERA':
			camera = obj.data
			variety = camera.type
			fov = camera.angle # float (in radians)
			near = camera.clip_start
			far = camera.clip_end
			if variety != 'PERSP':
				orto_scale = camera.orto_scale
		elif obj.type == 'CURVE':
			curve = obj.data
			for spline in curve.splines:
				if spline.type != 'NURBS':
					continue
				points = [point.co for point in spline.points]
			
	# Writeback
	writer = WeepSceneWriter(filepath)
	writer.writeLine('{\n')
	writer.indent()
	writer.writeLine('"objects": [\n')
	writer.indent()
	writer.writeLine('{\n')
	for entity in entities:
		writer.indent()
		writer.writeObject(entity['prefab'], entity['pos'], entity['rot'], entity['scale'])
		writer.deindent()
		writer.writeLine('},{\n')
	writer.deindent()
	writer.writeLine(']\n')
	writer.deindent()
	writer.writeLine('}\n')

		
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
		


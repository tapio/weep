#include "glutil.hpp"


const char* glutil::getErrorString(uint error)
{
	switch (error) {
		case GL_NO_ERROR:          return "no error";
		case GL_INVALID_ENUM:      return "invalid enum";
		case GL_INVALID_VALUE:     return "invalid value";
		case GL_INVALID_OPERATION: return "invalid operation";
		//case GL_STACK_OVERFLOW:    return "stack overflow";
		//case GL_STACK_UNDERFLOW:   return "stack underflow";
		case GL_OUT_OF_MEMORY:     return "out of memory";
		case GL_INVALID_FRAMEBUFFER_OPERATION: return "invalid framebuffer operation";
	}
	logError("Unknown OpenGL error %u", error);
	return "unknown error";
}

GLenum glutil::toGL(ShaderType type)
{
	switch (type) {
		case VERTEX_SHADER: return GL_VERTEX_SHADER;
		case FRAGMENT_SHADER: return GL_FRAGMENT_SHADER;
		case GEOMETRY_SHADER: return GL_GEOMETRY_SHADER;
		case TESS_CONTROL_SHADER: return GL_TESS_CONTROL_SHADER;
		case TESS_EVALUATION_SHADER: return GL_TESS_EVALUATION_SHADER;
		case COMPUTE_SHADER: return GL_COMPUTE_SHADER;
	}
	logError("Invalid ShaderType %d", type);
	return 0;
}

uint glutil::getTypeSize(GLenum type)
{
	switch (type) {
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
			return 1;
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
			return 2;
		case GL_FLOAT:
		case GL_INT:
		case GL_UNSIGNED_INT:
			return 4;
		case GL_DOUBLE:
			return 8;
	}
	ASSERT(!"Unknown gl type queried for size");
	return 0;
}

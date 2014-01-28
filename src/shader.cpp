#include "shader.hpp"

#include <GL/glcorearb.h>

GLenum convertToGL(ShaderType type)
{
	switch (type) {
		case VERTEX_SHADER: return GL_VERTEX_SHADER;
		case FRAGMENT_SHADER: return GL_FRAGMENT_SHADER;
		case GEOMETRY_SHADER: return GL_GEOMETRY_SHADER;
		case TESS_CONTROL_SHADER: return GL_TESS_CONTROL_SHADER;
		case TESS_EVALUATION_SHADER: return GL_TESS_EVALUATION_SHADER;
	}
	return 0;
}

ShaderProgram::ShaderProgram()
{
}


ShaderProgram::~ShaderProgram()
{
	for (auto i : shaderIds) {
		if (i > 0) glDeleteShader(i);
	}
	glDeleteProgram(id);
}

void ShaderProgram::compile(ShaderType type, const string& text)
{
	GLsizei lengths[] = { (GLsizei)text.length() };
	const GLchar* strings[] = { (const GLchar*)text.c_str() };
	unsigned& shaderId = shaderIds[type];
	shaderId = glCreateShader(convertToGL(type));
	glShaderSource(shaderId, 1, strings, lengths);
	glCompileShader(shaderId);
}

void ShaderProgram::link()
{
	id = glCreateProgram();

	for (auto i : shaderIds) {
		if (i > 0) glAttachShader(id, i);
	}

	glLinkProgram(id);
}

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

bool ShaderProgram::compile(ShaderType type, const string& text)
{
	GLsizei lengths[] = { (GLsizei)text.length() };
	const GLchar* strings[] = { (const GLchar*)text.c_str() };
	uint& shaderId = shaderIds[type];
	shaderId = glCreateShader(convertToGL(type));
	glShaderSource(shaderId, 1, strings, lengths);
	glCompileShader(shaderId);

	// Get shader log
	string infoLog;
	GLint infoLogLength;
	glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 1) {
		infoLog.resize(infoLogLength);
		glGetShaderInfoLog(shaderId, infoLogLength, NULL, &infoLog[0]);
	}
	// Check compilation status
	GLint status;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
	if (!status) {
		logError("Failed to compile shader: %s", infoLog.c_str());
		return false;
	}
	if (!infoLog.empty()) {
		logWarning("Warning(s) compiling shader:\n%s", infoLog.c_str());
	}

	return true;
}

bool ShaderProgram::link()
{
	id = glCreateProgram();
	for (auto i : shaderIds) {
		if (i > 0) glAttachShader(id, i);
	}
	glLinkProgram(id);

	// Get program log
	string infoLog;
	GLint infoLogLength;
	glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 1) {
		infoLog.resize(infoLogLength);
		glGetProgramInfoLog(id, infoLogLength, NULL, &infoLog[0]);
	}
	// Check linking status
	int status;
	glGetProgramiv(id, GL_LINK_STATUS, &status);
	if (!status) {
		logError("Failed to link program:\n%s", infoLog.c_str());
		return false;
	}
	if (!infoLog.empty()) {
		logWarning("Warning(s) linking program:\n%s", infoLog.c_str());
	}

	return true;
}

void ShaderProgram::use() const
{
	glUseProgram(id);
}

#include "shader.hpp"
#include "glutil.hpp"

ShaderProgram::ShaderProgram()
{
}

ShaderProgram::~ShaderProgram()
{
	// TODO: Somehow loadShaders emplace_back calls destructor
	//destroy();
}

bool ShaderProgram::compile(ShaderType type, const string& text, const std::string& defines)
{
	GLsizei lengths[] = { (GLsizei)defines.length(), (GLsizei)text.length() };
	const GLchar* strings[] = { (const GLchar*)defines.c_str(), (const GLchar*)text.c_str() };
	uint& shaderId = m_shaderIds[type];
	shaderId = glCreateShader(glutil::toGL(type));
	ASSERT(shaderId);
	glShaderSource(shaderId, 2, strings, lengths);
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
	ASSERT(id);
	for (auto i : m_shaderIds) {
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
	// Check validation status
	glValidateProgram(id);
	glGetProgramiv(id, GL_VALIDATE_STATUS, &status);
	if (!status) {
		logError("Failed to validate program:\n%s", infoLog.c_str());
		return false;
	}
	if (!infoLog.empty()) {
		logWarning("Warning(s) linking program:\n%s", infoLog.c_str());
	}

	return true;
}

bool ShaderProgram::has(ShaderType type) const
{
	return m_shaderIds[type] > 0;
}

void ShaderProgram::use() const
{
	ASSERT(id);
	glUseProgram(id);
}

void ShaderProgram::destroy()
{
	logDebug("Shader destructing");
	for (auto& i : m_shaderIds) {
		if (i > 0) { glDeleteShader(i); i = 0; }
	}
	glDeleteProgram(id);
	id = 0;
}

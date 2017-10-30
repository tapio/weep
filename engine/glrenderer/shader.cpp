#include "shader.hpp"
#include "glutil.hpp"

//#define DUMP_SHADERS

constexpr GLenum s_shaderTypes[] = {
	GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER,
	GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_COMPUTE_SHADER
};

constexpr const char* s_shaderTypeNames[] = {
	"vertex", "fragment", "geometry", "tess control", "tess eval", "compute"
};
static_assert(countof(s_shaderTypeNames) == countof(s_shaderTypes), "Array size mismatch");

ShaderProgram::ShaderProgram(const std::string& debugName)
	: name(debugName)
{
}

ShaderProgram::~ShaderProgram()
{
	// TODO: Somehow loadShaders emplace_back calls destructor
	//destroy();
}

bool ShaderProgram::compile(ShaderType type, const string& text, const std::string& defines)
{
#ifdef DUMP_SHADERS
	constexpr const char* s_shaderTypeExtensions[] = { ".vert", ".frag", ".geom", ".tesc", ".tese", ".comp" };
	static_assert(countof(s_shaderTypeExtensions) == countof(s_shaderTypes), "Array size mismatch");
	writeFile("/tmp/" + name + s_shaderTypeExtensions[type], defines + text);
#endif

	GLsizei lengths[] = { (GLsizei)defines.length(), (GLsizei)text.length() };
	const GLchar* strings[] = { (const GLchar*)defines.c_str(), (const GLchar*)text.c_str() };
	uint& shaderId = m_shaderIds[type];
	shaderId = glCreateShader(s_shaderTypes[type]);
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
		logError("Failed to compile %s shader \"%s\":\n%s", s_shaderTypeNames[type], name.c_str(), infoLog.c_str());
		return false;
	}
	if (!infoLog.empty()) {
		logWarning("Warning(s) compiling %s shader \"%s\":\n%s", s_shaderTypeNames[type], name.c_str(), infoLog.c_str());
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
		logError("Failed to link program \"%s\":\n%s", name.c_str(), infoLog.c_str());
		return false;
	}
	// Check validation status
	glValidateProgram(id);
	glGetProgramiv(id, GL_VALIDATE_STATUS, &status);
	if (!status) {
		logError("Failed to validate program \"%s\":\n%s", name.c_str(), infoLog.c_str());
		return false;
	}
	if (!infoLog.empty()) {
		logWarning("Warning(s) linking program \"%s\":\n%s", name.c_str(), infoLog.c_str());
	}
	logDebug("Linked shader %d: \"%s\" (shader mask: %c%c%c%c%c%c)",
		id, name.c_str(),
		has(VERTEX_SHADER) ? 'v' : '-',
		has(FRAGMENT_SHADER) ? 'f' : '-',
		has(GEOMETRY_SHADER) ? 'g' : '-',
		has(TESS_CONTROL_SHADER) ? 'c' : '-',
		has(TESS_EVALUATION_SHADER) ? 'e' : '-',
		has(COMPUTE_SHADER) ? 'C' : '-'
	);
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

void ShaderProgram::compute(uint x_size, uint y_size, uint z_size) const
{
	ASSERT(id && has(COMPUTE_SHADER));
	glDispatchCompute(x_size, y_size, z_size);
}

void ShaderProgram::destroy()
{
	//logDebug("Shader %s destructing", name.c_str());
	for (auto& i : m_shaderIds) {
		if (i > 0) { glDeleteShader(i); i = 0; }
	}
	if (id)
		glDeleteProgram(id);
	id = 0;
}

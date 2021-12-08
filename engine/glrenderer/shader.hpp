#pragma once
#include "common.hpp"

enum ShaderType
{
	VERTEX_SHADER,
	FRAGMENT_SHADER,
	GEOMETRY_SHADER,
	TESS_CONTROL_SHADER,
	TESS_EVALUATION_SHADER,
	COMPUTE_SHADER
};

struct ShaderProgram
{
	ShaderProgram(const string& debugName = "unknown");
	~ShaderProgram();

	NONCOPYABLE(ShaderProgram);

	bool compile(ShaderType type, const string& text, const string& defines = "");
	bool link();
	bool has(ShaderType type) const;
	void use() const;
	void compute(uint x_size, uint y_size = 1, uint z_size = 1) const;
	void destroy();

	uint id = 0;
	string name;

private:
	uint m_shaderIds[6] = { 0, 0, 0, 0, 0, 0 };
};

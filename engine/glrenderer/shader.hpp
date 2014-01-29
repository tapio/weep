#pragma once
#include "common.hpp"

enum ShaderType
{
	VERTEX_SHADER,
	FRAGMENT_SHADER,
	GEOMETRY_SHADER,
	TESS_CONTROL_SHADER,
	TESS_EVALUATION_SHADER
};

struct ShaderProgram
{
	ShaderProgram();
	~ShaderProgram();

	NONCOPYABLE(ShaderProgram);

	bool compile(ShaderType type, const string& text);
	bool link();
	void use() const;

	uint id = 0;

private:
	std::vector<uint> shaderIds = { 0, 0, 0, 0, 0 };
};

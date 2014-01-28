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

	void compile(ShaderType type, const string& text);
	void link();

	unsigned id = 0;

private:
	std::vector<unsigned> shaderIds = { 0, 0, 0, 0, 0 };
};

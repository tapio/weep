#pragma once
#include "common.hpp"
#include "shader.hpp"
#ifdef _WIN32
#include "glad/glad.h"
#else
#include "GL/glcorearb.h"
#endif

namespace glutil
{

int getInt(GLenum pname);

const char* getErrorString(uint error);

bool checkGL(const char* format, ...);

GLenum toGL(ShaderType type);

}

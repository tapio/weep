#pragma once
#include "common.hpp"
#include "shader.hpp"
#include <GL/glcorearb.h>

namespace glutil
{

const char* getErrorString(uint error);

bool checkGL(const char* format, ...);

GLenum toGL(ShaderType type);

}

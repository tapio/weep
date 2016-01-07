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

uint getTypeSize(GLenum type);

const char* getErrorString(uint error);

}

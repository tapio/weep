#pragma once

#include <vector>
#include <memory>
#include <string>
#include <cstring>
#include <cstdarg>
#include <json11/json11.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_CXX11
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/normal.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/component_wise.hpp>
#define SDL_ASSERT_LEVEL 2
#include <SDL2/SDL_assert.h>

using std::string;
using json11::Json;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat2;
using glm::mat3;
using glm::mat4;
using glm::quat;
typedef unsigned uint;
typedef uint64_t uint64;
typedef int64_t int64;

static const vec3 xaxis(1, 0, 0);
static const vec3 yaxis(0, 1, 0);
static const vec3 zaxis(0, 0, 1);

string vlformat(const char* format, va_list vl);

void logDebug(const char* format, ...);
void logInfo(const char* format, ...);
void logWarning(const char* format, ...);
void logError(const char* format, ...);
void panic(const char* format, ...);

string readFile(const string& path);
string replace(string str, const string& search, const string& replace);

#define ASSERT SDL_assert

#define NONCOPYABLE(T) \
	T(T&&) = default; \
	T(const T&) = delete; \
	T& operator=(const T&) = delete

inline vec3 toVec3(const Json& arr) {
	ASSERT(arr.is_array());
	return vec3(arr[0].number_value(), arr[1].number_value(), arr[2].number_value());
}
inline vec3 colorToVec3(const Json& color) {
	if (color.is_number()) return vec3(color.number_value());
	//if (color.is_string()) return TODO;
	return toVec3(color);
}

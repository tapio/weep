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
#include <glm/gtc/random.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/normal.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/component_wise.hpp>
#include <id/id.hpp>
#define SDL_ASSERT_LEVEL 2
#include <SDL2/SDL_assert.h>
#define ASSERT SDL_assert
#include <SDL2/SDL_timer.h>
#define ECS_ASSERT ASSERT
#include <ecs/ecs.hpp>

using namespace ecs;
using std::string;
using json11::Json;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat2;
using glm::mat3;
using glm::mat4;
using glm::quat;
typedef uint8_t uint8;
typedef unsigned uint;
typedef uint64_t uint64;
typedef int64_t int64;

string vlformat(const char* format, va_list vl);

void logDebug(const char* format, ...);
void logInfo(const char* format, ...);
void logWarning(const char* format, ...);
void logError(const char* format, ...);
void panic(const char* format, ...);

string readFile(const string& path);
string replace(string str, const string& search, const string& replace);
uint timestamp(const string& path);

void sleep(uint ms);

template <typename T, std::size_t N>
constexpr std::size_t countof(T const (&)[N]) noexcept { return N; }


#define NONCOPYABLE(T) \
	T(T&&) = default; \
	T(const T&) = delete; \
	T& operator=(const T&) = delete

#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif

// TODO: Dummy versions for non-debug build
#define START_MEASURE(var_name) \
	float var_name = 0.f; { uint64 _t0_##var_name = SDL_GetPerformanceCounter();
#define END_MEASURE(var_name) \
	uint64 _t1_##var_name = SDL_GetPerformanceCounter(); \
	var_name = (_t1_##var_name - _t0_##var_name) / (double)SDL_GetPerformanceFrequency() * 1000.0; }


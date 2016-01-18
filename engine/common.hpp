#pragma once

#include <vector>
#include <string>
#include <cstdarg>
#include <json11/json11.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_CXX11
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#define SDL_ASSERT_LEVEL 2
#include <SDL_assert.h>
#define ASSERT SDL_assert
#include <SDL_timer.h>
#define ECS_ASSERT ASSERT
#include <ecs/ecs.hpp>

using namespace ecs;
using std::string;
using json11::Json;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::u8vec4;
using glm::mat2;
using glm::mat3;
using glm::mat4;
using glm::mat3x4;
using glm::quat;
typedef uint8_t uint8;
typedef unsigned short ushort;
typedef unsigned int uint;
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
bool endsWith(const string& str, const string& ending);
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

#define USE_PROFILER 1
#ifdef USE_PROFILER
	#define RMT_USE_OPENGL 1
	#include "remotery/Remotery.h"
	#define BEGIN_CPU_SAMPLE(name) rmt_BeginCPUSample(name);
	#define END_CPU_SAMPLE(name) rmt_EndCPUSample();
	#define SCOPED_CPU_SAMPLE(name) rmt_ScopedCPUSample(name);
	#define BEGIN_GPU_SAMPLE(name) rmt_BeginOpenGLSample(name);
	#define END_GPU_SAMPLE() rmt_EndOpenGLSample();
	#define SCOPED_GPU_SAMPLE(name) rmt_ScopedOpenGLSample(name);
	#define PROFILER_LOG(text) rmt_LogText(text);
#else
	#define BEGIN_CPU_SAMPLE(name)
	#define END_CPU_SAMPLE(name)
	#define SCOPED_CPU_SAMPLE(name)
	#define BEGIN_GPU_SAMPLE(name)
	#define END_GPU_SAMPLE()
	#define SCOPED_GPU_SAMPLE(name)
	#define PROFILER_LOG(text)
#endif

// TODO: Dummy versions for non-debug build?
#define START_MEASURE(var_name) \
	float var_name = 0.f; uint64 _t0_##var_name = SDL_GetPerformanceCounter(); \
	BEGIN_CPU_SAMPLE(var_name)
#define END_MEASURE(var_name) \
	var_name = (SDL_GetPerformanceCounter() - _t0_##var_name) / (double)SDL_GetPerformanceFrequency() * 1000.0; \
	END_CPU_SAMPLE(var_name)

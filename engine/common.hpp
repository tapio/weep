#pragma once

#include <vector>
#include <string>
#include <cstdarg>
#define GLM_FORCE_CXX11
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/norm.hpp>
#define SDL_ASSERT_LEVEL 2
#include <SDL_assert.h>
#define ASSERT SDL_assert
#include <SDL_timer.h>

#define ECS_ASSERT ASSERT

using std::string;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::ivec2;
using glm::ivec3;
using glm::ivec4;
using glm::uvec2;
using glm::uvec3;
using glm::uvec4;
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

#define quat_identity quat(1.f, 0.f, 0.f, 0.f)
#define forward_axis vec3(0, 0, -1)
#define up_axis vec3(0, 1, 0)
#define left_axis vec3(-1, 0, 0)
#define right_axis vec3(1, 0, 0)

#define NONCOPYABLE(T) \
	T(T&&) = default; \
	T(const T&) = delete; \
	T& operator=(const T&) = delete


#ifndef EMBED_MODULES
	#define EMBED_MODULES 0
#endif
#if EMBED_MODULES
	#define MODULE_EXPORT
	#define WEEP_API
#elif defined(_WIN32)
	#define MODULE_EXPORT extern "C" __declspec(dllexport)
	#ifdef MODULE_NAME
		#define WEEP_API __declspec(dllimport)
	#else
		#define WEEP_API __declspec(dllexport)
	#endif
#else
	#define MODULE_EXPORT extern "C"
	#define WEEP_API
#endif

#ifndef MODULE_FUNC_NAME
#define MODULE_FUNC_NAME ModuleFunc
#endif


string vlformat(const char* format, va_list vl);

void logDebug(const char* format, ...);
void logInfo(const char* format, ...);
void logWarning(const char* format, ...);
void logError(const char* format, ...);
void panic(const char* format, ...);

struct CVarBase {
	bool tryParseFrom(const std::string& newValue);
	typedef float ValueType;
	ValueType value = 0;
	std::string name;
	static CVarBase* getCVar(const string& name);
	static std::vector<string> getMatchingCVarNames(const std::string& prefix);
protected:
	static void registerCVar(const string& name, struct CVarBase* cvar);
	CVarBase(const std::string& name_, ValueType defaultValue): name(name_), value(defaultValue) { registerCVar(name_, this); }
};
template<typename T>
struct CVar: public CVarBase {
	CVar(const std::string& name, T defaultValue = {}): CVarBase(name, (CVarBase::ValueType)defaultValue) { }
	T operator()() const { return (T)value; }
	void operator=(T v) { value = (CVarBase::ValueType)v; }
};


template <typename T, std::size_t N>
constexpr std::size_t countof(T const (&)[N]) noexcept { return N; }

struct id
{
	constexpr static uint fnv1a(const char* str, uint h = 2166136261u) {
		return !str[0] ? h : fnv1a(str + 1, (h ^ str[0]) * 16777619u);
	}
	static uint hash(const char* str) {
		ASSERT(str);
		uint ret = 2166136261u;
		while (*str) {
			ret = (ret ^ str[0]) * 16777619u;
			str++;
		}
		return ret;
	}
	static uint hash(const std::string& str) {
		return hash(str.c_str());
	}
};
#define $id(...) id::fnv1a(#__VA_ARGS__)


#define COMBINE1(X,Y) X##Y
#define COMBINE(X,Y) COMBINE1(X,Y)

//#define USE_PROFILER 1 // From CMake
#ifdef USE_PROFILER
	#define RMT_USE_OPENGL 1
	#include "remotery/Remotery.h"
	#define BEGIN_CPU_SAMPLE(name) rmt_BeginCPUSample(name, 0);
	#define END_CPU_SAMPLE() rmt_EndCPUSample();
	#define SCOPED_CPU_SAMPLE(name) rmt_ScopedCPUSample(name, 0);
	#define BEGIN_GPU_SAMPLE(name) rmt_BeginOpenGLSample(name);
	#define END_GPU_SAMPLE() rmt_EndOpenGLSample();
	#define SCOPED_GPU_SAMPLE(name) rmt_ScopedOpenGLSample(name);
	#define PROFILER_LOG(text) rmt_LogText(text);
	#define BEGIN_GPU_SAMPLE_STRING(nameStr) \
		RMT_OPTIONAL(RMT_USE_OPENGL, { _rmt_BeginOpenGLSample((nameStr), nullptr); }) // Hash cache disabled as would need unique variables for all different strings
	#define SCOPED_GPU_SAMPLE_STRING(nameStr) \
		BEGIN_GPU_SAMPLE_STRING(nameStr) RMT_OPTIONAL(RMT_USE_OPENGL, rmt_EndOpenGLSampleOnScopeExit rmt_ScopedOpenGLSample##__LINE__);
#else
	#define BEGIN_CPU_SAMPLE(name)
	#define END_CPU_SAMPLE()
	#define SCOPED_CPU_SAMPLE(name)
	#define BEGIN_GPU_SAMPLE(name)
	#define END_GPU_SAMPLE()
	#define SCOPED_GPU_SAMPLE(name)
	#define PROFILER_LOG(text)
#endif

// TODO: Dummy versions for non-debug build?
#define START_MEASURE(var_name) \
	uint64 _t0_##var_name = SDL_GetPerformanceCounter(); \
	BEGIN_CPU_SAMPLE(var_name)
#define END_MEASURE(var_name) \
	float var_name = (SDL_GetPerformanceCounter() - _t0_##var_name) / (double)SDL_GetPerformanceFrequency() * 1000.0; \
	END_CPU_SAMPLE()

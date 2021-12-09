#pragma once
#include "common.hpp"
#include "../../data/shaders/uniforms.glsl"

// Generic Buffer Object
struct BufferObjectBase
{
	virtual ~BufferObjectBase() { destroy(); }

	NONCOPYABLE(BufferObjectBase);

protected:
	BufferObjectBase() {}
	void create(uint type, uint binding, uint size, const void* data);
	void upload(uint type, uint size, const void* data);

public:
	void destroy();

	uint id = 0;
};


// Uniform Buffer Object
template<typename T>
struct UBO : public BufferObjectBase
{
	UBO() {}

	NONCOPYABLE(UBO);

	void create() { BufferObjectBase::create(/*GL_UNIFORM_BUFFER*/ 0x8A11, uniforms.binding, sizeof(uniforms), &uniforms); }
	void upload() { BufferObjectBase::upload(/*GL_UNIFORM_BUFFER*/ 0x8A11, sizeof(uniforms), &uniforms); }

	T uniforms = T();
};


// Shader Storage Buffer Object
template<typename T>
struct SSBO : public BufferObjectBase
{
	SSBO(uint binding): binding(binding) {}

	NONCOPYABLE(SSBO);

	void create() { BufferObjectBase::create(/*GL_SHADER_STORAGE_BUFFER*/ 0x90D2, binding, sizeof(T) * buffer.size(), buffer.data()); }
	void upload(bool preserveData = false) {
		BufferObjectBase::upload(/*GL_SHADER_STORAGE_BUFFER*/ 0x90D2, sizeof(T) * buffer.size(), buffer.data());
		if (!preserveData) {
			buffer.clear();
			buffer.shrink_to_fit();
		}
	}

	uint binding = 0;
	std::vector<T> buffer;
};

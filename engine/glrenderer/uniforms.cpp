#include "uniforms.hpp"
#include "glutil.hpp"

template<typename T, uint N>
void UBO<T, N>::create(uint binding)
{
	glGenBuffers(1, &id);
	glBindBuffer(GL_UNIFORM_BUFFER, id);
	glBindBufferBase(GL_UNIFORM_BUFFER, binding, id);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(uniforms) * N, (const GLvoid*)&uniforms, GL_DYNAMIC_DRAW);
}

template<typename T, uint N>
void UBO<T, N>::upload()
{
	glBindBuffer(GL_UNIFORM_BUFFER, id);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(uniforms) * N, (const GLvoid*)&uniforms, GL_DYNAMIC_DRAW);
}

template<typename T, uint N>
void UBO<T, N>::destroy()
{
	if (id) {
		glDeleteBuffers(1, &id);
		id = 0;
	}
}

template class UBO<UniformCommonBlock, 1>;
template class UBO<UniformColorBlock, 1>;
template class UBO<UniformLightBlock, MAX_LIGHTS>;

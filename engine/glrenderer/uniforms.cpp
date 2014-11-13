#include "uniforms.hpp"
#include "glutil.hpp"

template<typename T>
void UBO<T>::create(uint binding)
{
	glGenBuffers(1, &id);
	glBindBuffer(GL_UNIFORM_BUFFER, id);
	glBindBufferBase(GL_UNIFORM_BUFFER, binding, id);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(uniforms), (const GLvoid*)&uniforms, GL_DYNAMIC_DRAW);
}

template<typename T>
void UBO<T>::upload()
{
	glBindBuffer(GL_UNIFORM_BUFFER, id);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(uniforms), (const GLvoid*)&uniforms, GL_DYNAMIC_DRAW);
}

template<typename T>
void UBO<T>::destroy()
{
	if (id) {
		glDeleteBuffers(1, &id);
		id = 0;
	}
}

template class UBO<UniformCommonBlock>;
template class UBO<UniformColorBlock>;
template class UBO<UniformLightBlock>;

#include "uniforms.hpp"
#include "glutil.hpp"

template<typename T>
UBO<T>::~UBO()
{
	destroy();
}

template<typename T>
void UBO<T>::create()
{
	glGenBuffers(1, &id);
	glBindBuffer(GL_UNIFORM_BUFFER, id);
	glBindBufferBase(GL_UNIFORM_BUFFER, uniforms.binding, id);
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

template struct UBO<UniformCommonBlock>;
template struct UBO<UniformObjectBlock>;
template struct UBO<UniformMaterialBlock>;
template struct UBO<UniformLightBlock>;
template struct UBO<UniformCubeShadowBlock>;
template struct UBO<UniformSkinningBlock>;
template struct UBO<UniformPostProcessBlock>;

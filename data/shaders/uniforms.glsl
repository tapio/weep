
#define MAX_LIGHTS 4

#ifdef __cplusplus
#define UBO_PREFIX(bindpoint) struct
#define UBO_SUFFIX(name)
#else
#define UBO_PREFIX(bindpoint) layout(binding = bindpoint, std140) uniform
#define UBO_SUFFIX(name) name
#endif

UBO_PREFIX(0) UniformCommonBlock {
	mat4 modelMatrix;
	mat4 modelViewMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 normalMatrix; // Problems with alignment if sent as mat3
	vec3 cameraPosition; float numLights;
};

UBO_PREFIX(1) UniformColorBlock {
	vec3 ambient; float pad1;
	vec3 diffuse; float pad2;
	vec3 specular; float pad3;
	float shininess;
} UBO_SUFFIX(material);

struct UniformLightData {
	vec3 color; float pad1;
	vec3 position; float pad2;
	vec3 direction; float pad3;
	vec3 params; float pad4;
};

UBO_PREFIX(2) UniformLightBlock {
	UniformLightData lights[MAX_LIGHTS];
};

#undef UBO_PREFIX

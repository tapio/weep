
#define MAX_LIGHTS 4

#ifdef __cplusplus
#define UBO_PREFIX(name, bindpoint) struct name { static const uint binding = bindpoint;
#define UBO_SUFFIX(varname) };
#else
#define UBO_PREFIX(name, bindpoint) layout(binding = bindpoint, std140) uniform name {
#define UBO_SUFFIX(varname) } varname;
#endif

UBO_PREFIX(UniformCommonBlock, 0)
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec3 cameraPosition; float numLights;
};

UBO_PREFIX(UniformObjectBlock, 1)
	mat4 modelMatrix;
	mat4 modelViewMatrix;
	mat4 normalMatrix; // Problems with alignment if sent as mat3
};

UBO_PREFIX(UniformColorBlock, 2)
	vec3 ambient; float pad1;
	vec3 diffuse; float pad2;
	vec3 specular; float shininess;
UBO_SUFFIX(material)

struct UniformLightData {
	vec3 color; float pad1;
	vec3 position; float pad2;
	vec3 direction; float pad3;
	vec4 params;
};

UBO_PREFIX(UniformLightBlock, 3)
	UniformLightData lights[MAX_LIGHTS];
};

#undef UBO_PREFIX

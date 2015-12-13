
#define MAX_LIGHTS 4
#define MAX_SHADOW_MAPS 1
#define MAX_SHADOW_CUBES 2
#define MAX_SHADOWS (MAX_SHADOW_MAPS + MAX_SHADOW_CUBES)

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
	vec3 cameraPosition; int numLights;
	vec3 globalAmbient; float exposure;
	vec3 sunPosition; int tonemap;
	vec3 sunColor; float bloomThreshold;
	vec3 fogColor; float fogDensity;
	float near; float far; float pad1; float pad2;
	vec3 vignette; float pad3;
};

UBO_PREFIX(UniformObjectBlock, 1)
	mat4 modelMatrix;
	mat4 modelViewMatrix;
	mat4 shadowMatrix;
	mat4 normalMatrix; // Problems with alignment if sent as mat3
};

UBO_PREFIX(UniformMaterialBlock, 2)
	vec3 ambient; float pad1;
	vec3 diffuse; float pad2;
	vec3 specular; float shininess;
	vec3 emissive; float pad3;
	vec2 uvOffset; vec2 uvRepeat;
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

UBO_PREFIX(UniformCubeShadowBlock, 4)
	mat4 shadowMatrixCube[6];
};

#undef UBO_PREFIX

#ifndef __cplusplus

#define VERTEX_DATA(inout, name) \
	inout VertexData { \
		vec3 position; \
		vec2 texcoord; \
		vec3 normal; \
		vec4 shadowcoord; \
	} name;


#ifdef USE_DIFFUSE_MAP
layout(binding = 10) uniform sampler2D diffuseMap;
#endif
#ifdef USE_NORMAL_MAP
layout(binding = 11) uniform sampler2D normalMap;
#endif
#ifdef USE_SPECULAR_MAP
layout(binding = 12) uniform sampler2D specularMap;
#endif
#if defined(USE_HEIGHT_MAP) || defined(USE_PARALLAX_MAP)
layout(binding = 13) uniform sampler2D heightMap;
#endif
#ifdef USE_EMISSION_MAP
layout(binding = 14) uniform sampler2D emissionMap;
#endif
#ifdef USE_REFLECTION_MAP
layout(binding = 15) uniform sampler2D reflectionMap;
#endif
#ifdef USE_ENV_MAP
layout(binding = 16) uniform samplerCube envMap;
#endif
#ifdef USE_SHADOW_MAP
layout(binding = 17) uniform sampler2D shadowMap;
layout(binding = 18) uniform samplerCube shadowCube[MAX_SHADOW_CUBES];
#endif

#endif // __cplusplus

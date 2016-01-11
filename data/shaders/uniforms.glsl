
#define MAX_LIGHTS 4
#define MAX_SHADOW_MAPS 1
#define MAX_SHADOW_CUBES 3
#define MAX_SHADOWS (MAX_SHADOW_MAPS + MAX_SHADOW_CUBES)
#define MAX_BONES 80

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
	vec3 globalAmbient; float shadowDarkness;
	vec3 sunPosition; int skyType;
	vec3 sunColor; float bloomThreshold;
	vec3 fogColor; float fogDensity;
	float near; float far; float pad2;
};

UBO_PREFIX(UniformObjectBlock, 1)
	mat4 modelMatrix;
	mat4 modelViewMatrix;
	mat4 modelViewProjMatrix;
	mat4 shadowMatrix;
	mat4 normalMatrix; // Problems with alignment if sent as mat3
};

UBO_PREFIX(UniformMaterialBlock, 2)
	vec3 ambient; float pad1;
	vec3 diffuse; float reflectivity;
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

UBO_PREFIX(UniformCubeMatrixBlock, 4)
	mat4 cubeMatrices[6];
};

UBO_PREFIX(UniformSkinningBlock, 5)
	mat3x4 boneMatrices[MAX_BONES];
};

UBO_PREFIX(UniformPostProcessBlock, 6)
	int tonemap; float exposure; float saturation; float padding1;
	vec3 vignette; float sepia;
};


#undef UBO_PREFIX

#define BINDING_MATERIAL_MAP_START 8
#define BINDING_DIFFUSE_MAP 8
#define BINDING_NORMAL_MAP 9
#define BINDING_SPECULAR_MAP 10
#define BINDING_HEIGHT_MAP 11
#define BINDING_EMISSION_MAP 12
#define BINDING_AO_MAP 13
#define BINDING_REFLECTION_MAP 14
#define BINDING_ENV_MAP 15
#define BINDING_SHADOW_MAP 16
#define BINDING_SHADOW_CUBE 17


#ifndef __cplusplus

#ifdef USE_SHADOW_MAP
#define SHADOW_VARYINGS vec4 shadowcoord; vec3 worldPosition;
#else
#define SHADOW_VARYINGS
#endif

#define VERTEX_DATA(inout, name) \
	inout VertexData { \
		vec3 position; \
		vec2 texcoord; \
		vec3 normal; \
		SHADOW_VARYINGS \
	} name;

#ifdef USE_ANIMATION
#define USE_SKINNING
#endif

#ifdef USE_DIFFUSE_MAP
layout(binding = BINDING_DIFFUSE_MAP) uniform sampler2D diffuseMap;
#endif
#ifdef USE_NORMAL_MAP
layout(binding = BINDING_NORMAL_MAP) uniform sampler2D normalMap;
#endif
#ifdef USE_SPECULAR_MAP
layout(binding = BINDING_SPECULAR_MAP) uniform sampler2D specularMap;
#endif
#if defined(USE_HEIGHT_MAP) || defined(USE_PARALLAX_MAP)
layout(binding = BINDING_HEIGHT_MAP) uniform sampler2D heightMap;
#endif
#ifdef USE_EMISSION_MAP
layout(binding = BINDING_EMISSION_MAP) uniform sampler2D emissionMap;
#endif
#ifdef USE_AO_MAP
layout(binding = BINDING_AO_MAP) uniform sampler2D aoMap;
#endif
#ifdef USE_REFLECTION_MAP
layout(binding = BINDING_REFLECTION_MAP) uniform sampler2D reflectionMap;
#endif
#ifdef USE_ENV_MAP
layout(binding = BINDING_ENV_MAP) uniform samplerCube envMap;
#endif
#ifdef USE_SHADOW_MAP
layout(binding = BINDING_SHADOW_MAP) uniform sampler2D shadowMap;
layout(binding = BINDING_SHADOW_CUBE) uniform samplerCube shadowCube[MAX_SHADOW_CUBES];
#endif

#endif // __cplusplus

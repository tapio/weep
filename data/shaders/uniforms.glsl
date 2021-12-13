
#define MAX_LIGHTS 4
#define MAX_SHADOW_MAPS 1
#define MAX_SHADOW_CUBES 3
#define MAX_SHADOWS (MAX_SHADOW_MAPS + MAX_SHADOW_CUBES)
#define MAX_BONES 80
#define PARTICLE_GROUP_SIZE 32

#ifdef __cplusplus
#define UBO_PREFIX(name, bindpoint) struct name { static const uint binding = bindpoint;
#define UBO_SUFFIX(varname) };
// Old windows header cruft leaking in from somewhere...
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif
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
	float near; float far; float dt; float time;
};

UBO_PREFIX(UniformObjectBlock, 1)
	mat4 modelMatrix;
	mat4 modelViewMatrix;
	mat4 modelViewProjMatrix;
	mat4 shadowMatrix;
	mat4 normalMatrix; // Problems with alignment if sent as mat3
};

UBO_PREFIX(UniformParticleBlock, 2)
	float emit; float localSpace; float directionality; float randomRotation;
	vec2 emitRadiusMinMax; vec2 lifeTimeMinMax;
	vec2 speedMinMax; vec2 pad1;
UBO_SUFFIX(particle)

UBO_PREFIX(UniformMaterialBlock, 3)
	vec3 ambient; float alphaTest;
	vec3 diffuse; float reflectivity;
	vec3 specular; float shininess;
	vec3 emissive; float parallax;
	vec2 uvOffset; vec2 uvRepeat;
	vec2 particleSize; vec2 pad2;
UBO_SUFFIX(material)

struct UniformLightData {
	vec3 color; float pad1;
	vec3 position; float pad2;
	vec3 direction; float pad3;
	vec4 params;
};

UBO_PREFIX(UniformLightBlock, 4)
	UniformLightData lights[MAX_LIGHTS];
};

UBO_PREFIX(UniformCubeMatrixBlock, 5)
	mat4 cubeMatrices[6];
};

UBO_PREFIX(UniformSkinningBlock, 6)
	mat3x4 boneMatrices[MAX_BONES];
};

UBO_PREFIX(UniformPostProcessBlock, 7)
	int tonemap; float exposure; float saturation; int postAA;
	vec3 vignette; float sepia;
	float scanlines; float chromaticAberration; float padding1; float padding2;
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

#define BINDING_SSBO_START 30
#define BINDING_SSBO_POSITION 30
#define BINDING_SSBO_VELOCITY 31
#define BINDING_SSBO_LIFE 32
#define BINDING_SSBO_EXTRA 33

#ifndef __cplusplus

#ifdef GL_ES
	#define CONST
#else
	#define CONST const
#endif

#define PI 3.14159265
#define saturate(x) clamp(x, 0.0, 1.0)


#define ATTR_POSITION 0
#define ATTR_TEXCOORD 1
#define ATTR_NORMAL 2
#define ATTR_TANGENT 3
#define ATTR_COLOR 4
#define ATTR_BONE_INDEX 5
#define ATTR_BONE_WEIGHT 6

#ifdef USE_SHADOW_MAP
#define SHADOW_VARYINGS vec4 shadowcoord; vec3 worldPosition;
#else
#define SHADOW_VARYINGS
#endif

#ifdef USE_TANGENT
#define TANGENT_VARYINGS vec3 tangent;
#else
#define TANGENT_VARYINGS
#endif

#ifdef USE_VERTEX_COLOR
#define VERTEX_COLOR_VARYINGS vec4 color;
#else
#define VERTEX_COLOR_VARYINGS
#endif

#define VERTEX_DATA(inout, name) \
	inout VertexData { \
		vec3 position; \
		vec2 texcoord; \
		vec3 normal; \
		TANGENT_VARYINGS \
		VERTEX_COLOR_VARYINGS \
		SHADOW_VARYINGS \
	} name

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

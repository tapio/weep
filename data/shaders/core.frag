
#define MAX_LIGHTS 4

layout(binding = 0, std140) uniform CommonBlock {
	mat4 modelMatrix;
	mat4 modelViewMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 normalMatrix; // Problems with alignment if sent as mat3
	vec3 cameraPosition; float numLights;
};

layout(binding = 1, std140) uniform ColorBlock {
	vec3 ambient; float pad1;
	vec3 diffuse; float pad2;
	vec3 specular; float pad3;
	float shininess;
} material;

struct LightData {
	vec3 color; float pad1;
	vec3 position; float pad2;
	vec3 direction; float pad3;
	vec3 params; float pad4;
};

layout(binding = 2, std140) uniform LightBlock {
	LightData lights[MAX_LIGHTS];
};

layout(binding = 10) uniform sampler2D diffuseMap;
layout(binding = 11) uniform sampler2D normalMap;
layout(binding = 12) uniform sampler2D specularMap;

in VertexData {
	vec2 texcoord;
	vec3 normal;
	vec3 fragPosition;
} input;

layout(location = 0) out vec4 fragment;

void main()
{
	// Accumulators
	vec3 ambientComp = material.ambient;
	vec3 diffuseComp = vec3(0);
	vec3 specularComp = vec3(0);

	vec3 norm = normalize(input.normal);
	vec3 viewDir = normalize(cameraPosition - input.fragPosition);

#ifdef ENABLE_DIFFUSE_MAP
	vec4 diffuseTex = texture(diffuseMap, input.texcoord);
	ambientComp *= diffuseTex.rgb;
#else
	vec4 diffuseTex = vec4(1.0);
#endif

#ifdef ENABLE_SPECULAR_MAP
	vec4 specularTex = texture(specularMap, input.texcoord);
#else
	vec4 specularTex = vec4(1.0);
#endif

	const int count = min(int(numLights), MAX_LIGHTS);
	for (int i = 0; i < count; ++i)
	{
		LightData light = lights[i];

		// Attenuation
		float distance = length(light.position - input.fragPosition);
		float attenuation = 1.0 / (light.params.x + light.params.y * distance +
			light.params.z * (distance * distance));

		vec3 lightDir = normalize(light.position - input.fragPosition);

		// Diffuse
#ifdef ENABLE_DIFFUSE
		float diff = max(dot(norm, lightDir), 0.0);
		diffuseComp += attenuation * diff * material.diffuse * light.color * diffuseTex.rgb;
#endif

		// Specular
#ifdef ENABLE_SPECULAR
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		specularComp += attenuation * spec * material.specular * light.color * specularTex.rgb;
#endif
	}

	fragment = vec4(ambientComp + diffuseComp + specularComp, 1.0);
}

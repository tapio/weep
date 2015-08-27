
#define MAX_LIGHTS 4

layout(binding = 0, std140) uniform CommonBlock {
	mat4 modelMatrix;
	mat4 modelViewMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 normalMatrix; // Problems with alignment if sent as mat3
	vec3 cameraPosition; float pad1;
};

layout(binding = 1, std140) uniform ColorBlock {
	vec3 ambient; float pad1;
	vec3 diffuse; float pad2;
	vec3 specular; float pad3;
	float shininess;
} material;

layout(binding = 2, std140) uniform LightBlock {
	vec3 color; float pad1;
	vec3 position; float pad2;
	vec3 direction; float pad3;
	vec3 params; float pad4;
} light/*[MAX_LIGHTS]*/;

layout(location = 0) uniform sampler2D diffuseMap;
layout(location = 1) uniform sampler2D normalMap;
layout(location = 2) uniform sampler2D specularMap;

in VertexData {
	vec2 texcoord;
	vec3 normal;
	vec3 fragPosition;
} input;

layout(location = 0) out vec4 fragment;

void main()
{
	// Ambient
	vec3 ambientComp = material.ambient;

	// Attenuation
	float distance = length(light.position - input.fragPosition);
	float attenuation = 1.0 / (light.params.x + light.params.y * distance +
		light.params.z * (distance * distance));

	vec3 norm = normalize(input.normal);
	vec3 lightDir = normalize(light.position - input.fragPosition);

	// Diffuse
	vec3 diffuseComp = vec3(0);
#ifdef ENABLE_DIFFUSE
	float diff = max(dot(norm, lightDir), 0.0);
	diffuseComp = attenuation * diff * material.diffuse * light.color;
#endif

#ifdef ENABLE_DIFFUSE_MAP
	vec4 diffuseTex = texture(diffuseMap, input.texcoord);
	ambientComp *= diffuseTex.rgb;
	diffuseComp *= diffuseTex.rgb;
#endif

	// Specular
	vec3 specularComp = vec3(0);
#ifdef ENABLE_SPECULAR
	vec3 viewDir = normalize(cameraPosition - input.fragPosition);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	specularComp = attenuation * spec * material.specular * light.color;
#endif

#ifdef ENABLE_SPECULAR_MAP
	vec4 specularTex = texture(specularMap, input.texcoord);
	specularComp *= specularTex.rgb;
#endif

	fragment = vec4(ambientComp + diffuseComp + specularComp, 1.0);
}

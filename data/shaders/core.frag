#version 330
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_explicit_uniform_location : enable

#define MAX_LIGHTS 4

layout(binding = 0, std140) uniform CommonBlock {
	mat4 modelMatrix;
	mat4 modelViewMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat3 normalMatrix;
	vec3 cameraPosition;
};

layout(binding = 1, std140) uniform ColorBlock {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
} material;

layout(binding = 2, std140) uniform LightBlock {
	vec3 color;
	vec3 position;
	vec3 direction;
	vec3 params;
} light/*[MAX_LIGHTS]*/;

layout(location = 0) uniform sampler2D diffuseMap;

in VertexData {
	vec2 texcoord;
	vec3 normal;
	vec3 fragPosition;
} input;

layout(location = 0) out vec4 fragment;

void main()
{
	vec4 diffuseTex = texture(diffuseMap, input.texcoord);

	// Ambient
	vec3 ambientComp = material.ambient * diffuseTex.rgb;

	// Attenuation
	float distance = length(light.position - input.fragPosition);
	float attenuation = 1.0 / (light.params.x + light.params.y * distance +
		light.params.z * (distance * distance));

	// Diffuse
	vec3 norm = normalize(input.normal);
	vec3 lightDir = normalize(light.position - input.fragPosition);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuseComp = attenuation * diff * material.diffuse * light.color * diffuseTex.rgb;

	// Specular
	vec3 viewDir = normalize(cameraPosition - input.fragPosition);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0 /*TODO: material.shininess*/);
	vec3 specularComp = attenuation * spec * material.specular * light.color;

	fragment = vec4(ambientComp + diffuseComp + specularComp, 1.0);
}

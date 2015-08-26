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
};

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
	// Ambient
	vec3 ambientComp = ambient * light.color;

	// Diffuse
	vec3 norm = normalize(input.normal);
	vec3 lightDir = normalize(light.position - input.fragPosition);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuseComp = diff * light.color;

	// Specular
	float specularStrength = 0.5;
	vec3 viewDir = normalize(cameraPosition - input.fragPosition);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specularComp = specularStrength * spec * light.color;

	vec4 tex = texture(diffuseMap, input.texcoord);
	fragment = vec4(ambientComp + diffuseComp /*+ specularComp*/, 1.0) * tex;
/*
	vec3 spec = vec3(0.0);
	vec3 n = normalize(input.normal);
	vec3 e = normalize(input.eye);

	float intensity = max(dot(n, light.direction), 0.0);

	if (intensity > 0.0) {
		// compute the half vector
		vec3 h = normalize(light.direction + e);
		// compute the specular term into spec
		float intSpec = max(dot(h, n), 0.0);
		spec = specular * pow(intSpec, 30.0 shininess); // shininess
	}

	fragment = vec4(max(intensity * diffuse + spec, ambient), 1.0);*/
}

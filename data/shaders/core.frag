
layout(binding = 10) uniform sampler2D diffuseMap;
layout(binding = 11) uniform sampler2D normalMap;
layout(binding = 12) uniform sampler2D specularMap;

in VertexData {
	vec2 texcoord;
	vec3 normal;
	vec3 fragPosition;
} input;

layout(location = 0) out vec4 fragment;

#define PI 3.14159265

void main()
{
	// Accumulators
	vec3 ambientComp = material.ambient;
	vec3 diffuseComp = vec3(0);
	vec3 specularComp = vec3(0);

	vec3 normal = normalize(input.normal);
	vec3 viewDir = normalize(cameraPosition - input.fragPosition);

#ifdef USE_DIFFUSE_MAP
	vec4 diffuseTex = texture(diffuseMap, input.texcoord);
	ambientComp *= diffuseTex.rgb;
#else
	vec4 diffuseTex = vec4(1.0);
#endif

#ifdef USE_SPECULAR_MAP
	vec4 specularTex = texture(specularMap, input.texcoord);
#else
	vec4 specularTex = vec4(1.0);
#endif

	const int count = min(int(numLights), MAX_LIGHTS);
	for (int i = 0; i < count; ++i)
	{
		UniformLightData light = lights[i];

		// Attenuation
		float distance = length(light.position - input.fragPosition);
		float attenuation = 1.0 / (light.params.x + light.params.y * distance +
			light.params.z * (distance * distance));

		vec3 lightDir = normalize(light.position - input.fragPosition);

		// Diffuse
#ifdef USE_DIFFUSE
		float diff = max(dot(normal, lightDir), 0.0);
		diffuseComp += attenuation * diff * material.diffuse * light.color * diffuseTex.rgb;
#endif

		// Specular
#ifdef USE_SPECULAR
#ifdef USE_PHONG
		const float energy = (2.0 + material.shininess) / (2.0 * PI);
		vec3 reflectDir = reflect(-lightDir, normal);
		float spec = energy * pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
#else // Blinn-Phong
		const float energy = (8.0 + material.shininess) / (8.0 * PI);
		vec3 halfDir = normalize(lightDir + viewDir);
		float spec = energy * pow(max(dot(normal, halfDir), 0.0), material.shininess);
#endif
		specularComp += attenuation * spec * material.specular * light.color * specularTex.rgb;
#endif
	}

	fragment = vec4(ambientComp + diffuseComp + specularComp, 1.0);
}

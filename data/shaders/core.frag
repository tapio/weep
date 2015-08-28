
layout(binding = 10) uniform sampler2D diffuseMap;
layout(binding = 11) uniform sampler2D normalMap;
layout(binding = 12) uniform sampler2D specularMap;

in VertexData {
	vec2 texcoord;
	vec3 normal;
	vec3 fragPosition;
} input;

layout(location = 0) out vec4 fragment;

// TODO: Create utils file
#define PI 3.14159265
#define saturate(x) clamp(x, 0.0, 1.0)

// http://www.thetenthplanet.de/archives/1180
mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
	// get edge vectors of the pixel triangle
	vec3 dp1 = dFdx(p);
	vec3 dp2 = dFdy(p);
	vec2 duv1 = dFdx(uv);
	vec2 duv2 = dFdy(uv);

	// solve the linear system
	vec3 dp2perp = cross(dp2, N);
	vec3 dp1perp = cross(N, dp1);
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	// construct a scale-invariant frame
	float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
	return mat3(T * invmax, B * invmax, N);
}

vec3 perturb_normal(vec3 normal, vec3 eye)
{
	vec3 normalTex = texture(normalMap, input.texcoord).xyz * 2.0 - 1.0;
	normalTex.y = -normalTex.y;
	normalTex.xy *= 2.0; // TODO: Scale
	mat3 TBN = cotangent_frame(normal, -eye, input.texcoord);
	return normalize(TBN * normalTex);
}

void main()
{
	// Accumulators
	vec3 ambientComp = material.ambient;
	vec3 diffuseComp = vec3(0);
	vec3 specularComp = vec3(0);

	vec3 viewDir = normalize(cameraPosition - input.fragPosition);
	vec3 normal = normalize(input.normal);
#ifdef USE_NORMAL_MAP
	normal = perturb_normal(normal, viewDir);
#endif

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
		if (distance >= light.params.x)
			continue;
		float attenuation = pow(saturate(1.0 - distance / light.params.x), light.params.y);

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

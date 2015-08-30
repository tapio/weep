
layout(binding = 10) uniform sampler2D diffuseMap;
layout(binding = 11) uniform sampler2D normalMap;
layout(binding = 12) uniform sampler2D specularMap;
layout(binding = 13) uniform sampler2D heightMap;
layout(binding = 14) uniform sampler2D emissionMap;

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

vec3 perturb_normal(vec3 normal, vec3 eye, vec2 texcoord)
{
	vec3 normalTex = texture(normalMap, texcoord).xyz * 2.0 - 1.0;
	normalTex.y = -normalTex.y;
	normalTex.xy *= 2.0; // TODO: Uniform
	mat3 TBN = cotangent_frame(normal, -eye, texcoord);
	return normalize(TBN * normalTex);
}

vec2 parallax_mapping(vec2 texcoord, vec3 viewDir)
{
	const float heightScale = 0.4; // TODO: Uniform
	// number of depth layers
	const float minLayers = 8;
	const float maxLayers = 32;
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
	// calculate the size of each layer
	float layerDepth = 1.0 / numLayers;
	// depth of current layer
	float currentLayerDepth = 0.0;
	// the amount to shift the texture coordinates per layer (from vector P)
	vec2 P = viewDir.xy / viewDir.z * heightScale;
	vec2 deltaTexCoords = P / numLayers;

	// get initial values
	vec2  currentTexCoords = texcoord;
	float currentDepthMapValue = texture(heightMap, currentTexCoords).r;

	while(currentLayerDepth < currentDepthMapValue)
	{
		// shift texture coordinates along direction of P
		currentTexCoords -= deltaTexCoords;
		// get depthmap value at current texture coordinates
		currentDepthMapValue = texture(heightMap, currentTexCoords).r;
		// get depth of next layer
		currentLayerDepth += layerDepth;
	}

	// -- parallax occlusion mapping interpolation from here on
	// get texture coordinates before collision (reverse operations)
	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

	// get depth after and before collision for linear interpolation
	float afterDepth = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = texture(heightMap, prevTexCoords).r - currentLayerDepth + layerDepth;

	// interpolation of texture coordinates
	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	return finalTexCoords;
}


void main()
{
	// Accumulators
	vec3 ambientComp = material.ambient;
	vec3 diffuseComp = vec3(0);
	vec3 specularComp = vec3(0);
	vec3 emissionComp = vec3(0);

	vec3 viewDir = normalize(cameraPosition - input.fragPosition);
	vec3 normal = normalize(input.normal);
	vec2 texcoord = input.texcoord;

#ifdef USE_PARALLAX_MAP
	mat3 TBN = cotangent_frame(normal, -viewDir, texcoord);
	vec3 tangentViewPos = TBN * cameraPosition;
	vec3 tangentFragPos = TBN * input.fragPosition;
	vec3 tangentViewDir = normalize(tangentViewPos - tangentFragPos);
	texcoord = parallax_mapping(texcoord, tangentViewDir);
	//if (texcoord.x > 1.0 || texcoord.y > 1.0 || texcoord.x < 0.0 || texcoord.y < 0.0)
	//	discard;
#endif

#ifdef USE_NORMAL_MAP
	normal = perturb_normal(normal, viewDir, texcoord);
#endif

#ifdef USE_DIFFUSE_MAP
	vec4 diffuseTex = texture(diffuseMap, texcoord);
	ambientComp *= diffuseTex.rgb;
#else
	vec4 diffuseTex = vec4(1.0);
#endif

#ifdef USE_SPECULAR_MAP
	vec4 specularTex = texture(specularMap, texcoord);
#else
	vec4 specularTex = vec4(1.0);
#endif

#ifdef USE_EMISSION_MAP
	emissionComp += texture(emissionMap, texcoord).rgb;
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

	fragment = vec4(ambientComp + diffuseComp + specularComp + emissionComp, 1.0);

	// Gamma correction
	fragment.rgb = pow(fragment.rgb, vec3(1.0 / 2.2));
}

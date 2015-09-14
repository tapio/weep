
in VertexData {
	vec3 position;
	vec2 texcoord;
	vec3 normal;
} input;

layout(location = 0) out vec4 fragment;
layout(location = 1) out vec4 brightFragment;

// TODO: Create utils file
#define PI 3.14159265
#define saturate(x) clamp(x, 0.0, 1.0)

#if defined(USE_NORMAL_MAP) || defined(USE_PARALLAX_MAP)
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
#endif

#ifdef USE_NORMAL_MAP
vec3 perturb_normal(mat3 TBN, vec2 texcoord)
{
	vec3 normalTex = texture(normalMap, texcoord).xyz * 2.0 - 1.0;
	normalTex.y = -normalTex.y;
	normalTex.xy *= 2.0; // TODO: Uniform
	return normalize(TBN * normalTex);
}
#endif

#ifdef USE_PARALLAX_MAP
vec2 parallax_mapping(vec2 texcoord, vec3 viewDir)
{
	const float heightScale = 0.2; // TODO: Uniform
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
	float currentDepthMapValue = 1.0 - texture(heightMap, currentTexCoords).r;

	while(currentLayerDepth < currentDepthMapValue)
	{
		// shift texture coordinates along direction of P
		currentTexCoords -= deltaTexCoords;
		// get depthmap value at current texture coordinates
		currentDepthMapValue = 1.0 - texture(heightMap, currentTexCoords).r;
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
#endif


void main()
{
	// Accumulators
	vec3 ambientComp = globalAmbient * material.ambient;
	vec3 diffuseComp = vec3(0);
	vec3 specularComp = vec3(0);
	vec3 emissionComp = vec3(0);
	float alpha = 1.f;

	vec3 viewDir = normalize(-input.position);
	vec3 normal = normalize(input.normal);
	vec2 texcoord = input.texcoord;

#if defined(USE_NORMAL_MAP) || defined(USE_PARALLAX_MAP)
	mat3 TBN = cotangent_frame(normal, -viewDir, texcoord);
#endif

#ifdef USE_PARALLAX_MAP
	vec3 tangentViewDir = normalize(TBN * viewDir);
	texcoord = parallax_mapping(texcoord, tangentViewDir);
	//if (texcoord.x > 1.0 || texcoord.y > 1.0 || texcoord.x < 0.0 || texcoord.y < 0.0)
	//	discard;
#endif

#ifdef USE_DIFFUSE_MAP
	vec4 diffuseTex = texture(diffuseMap, texcoord);
	ambientComp *= diffuseTex.rgb;
#ifdef USE_ALPHA_TEST
	if (diffuseTex.a < 0.9)
		discard;
#endif
#ifdef USE_ALPHA_BLEND
	alpha = diffuseTex.a;
#endif
#else // USE_DIFFUSE_MAP
	vec4 diffuseTex = vec4(1.0);
#endif

#ifdef USE_NORMAL_MAP
	normal = perturb_normal(TBN, texcoord);
#endif

#ifdef USE_SPECULAR_MAP
	vec4 specularTex = texture(specularMap, texcoord);
#else
	vec4 specularTex = vec4(1.0);
#endif

#ifdef USE_EMISSION_MAP
	emissionComp += texture(emissionMap, texcoord).rgb;
#endif

	float sunAmount = 0.0;
#if defined(USE_DIFFUSE) || defined(USE_SPECULAR)

	if (sunColor.r > 0 || sunColor.g > 0 || sunColor.b > 0) {
		vec3 sunDir = normalize(-(viewMatrix * vec4(sunDirection, 0.0)).xyz);
		sunAmount = max(dot(viewDir, -sunDir), 0.0);
		// Sun diffuse
#ifdef USE_DIFFUSE
		float diff = max(dot(normal, sunDir), 0.0);
		diffuseComp += diff * material.diffuse * sunColor * diffuseTex.rgb;
#endif
		// Sun specular
#ifdef USE_SPECULAR
#ifdef USE_PHONG
		const float energy = (2.0 + material.shininess) / (2.0 * PI);
		vec3 reflectDir = reflect(-sunDir, normal);
		float spec = energy * pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
#else // Blinn-Phong
		const float energy = (8.0 + material.shininess) / (8.0 * PI);
		vec3 halfDir = normalize(sunDir + viewDir);
		float spec = energy * pow(max(dot(normal, halfDir), 0.0), material.shininess);
#endif
		specularComp += spec * material.specular * sunColor * specularTex.rgb;
#endif
	}

	// Point lights
	const int count = min(numLights, MAX_LIGHTS);
	for (int i = 0; i < count; ++i)
	{
		UniformLightData light = lights[i];
		vec3 lightPos = (viewMatrix * vec4(light.position, 1.0)).xyz;

		// Attenuation
		float distance = length(lightPos - input.position);
		if (distance >= light.params.x)
			continue;
		float attenuation = pow(saturate(1.0 - distance / light.params.x), light.params.y);

		vec3 lightDir = normalize(lightPos - input.position);

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
#endif // defined(USE_DIFFUSE) || defined(USE_SPECULAR)

#ifdef USE_ENV_MAP
	vec3 worldNormal = normalize((vec4(normal, 0.0) * viewMatrix).xyz);
	vec3 worldView = normalize((vec4(viewDir, 0.0) * viewMatrix).xyz);
	vec3 envRefl = reflect(-worldView, worldNormal);
	vec4 envTex = texture(envMap, envRefl);
	float reflStrength = 1.0;
#ifdef USE_REFLECTION_MAP
	reflStrength *= texture(reflectionMap, texcoord).x;
#endif
	fragment = vec4(ambientComp + diffuseComp + specularComp + emissionComp, alpha);
	fragment.rgb *= (1.0 - reflStrength);
	fragment.rgb += reflStrength * envTex.rgb;
	//fragment.rgb = mix(fragment.rgb, envTex.rgb, reflStrength);
#else
	fragment = vec4(ambientComp + diffuseComp + specularComp + emissionComp, alpha);
#endif

#ifdef USE_FOG
	// http://iquilezles.org/www/articles/fog/fog.htm
	float depth = gl_FragCoord.z / gl_FragCoord.w;
	float fogAmount = 1.0 - exp(-depth * fogDensity);
	vec3 sunAffectedFogColor = mix(fogColor, sunColor, pow(sunAmount, 8.0));
	fragment.rgb = mix(fragment.rgb, sunAffectedFogColor, fogAmount);
#endif

	if (bloomThreshold > 0.f) {
		float brightness = dot(fragment.rgb, vec3(0.2126, 0.7152, 0.0722));
		brightFragment = (brightness > bloomThreshold || dot(emissionComp, emissionComp) > 0.0)
			? vec4(fragment.rgb, 1.0) : vec4(0.0, 0.0, 0.0, 1.0);
	} else brightFragment = vec4(0.0, 0.0, 0.0, 1.0);
}

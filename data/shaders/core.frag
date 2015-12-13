
VERTEX_DATA(in, input);

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

#ifdef USE_SHADOW_MAP
#define SHADOW_TAPS 16
const vec2 poissonDisk[SHADOW_TAPS] = vec2[](
	vec2(-0.94201624, -0.39906216),
	vec2(0.94558609, -0.76890725),
	vec2(-0.094184101, -0.92938870),
	vec2(0.34495938, 0.29387760),
	vec2(-0.91588581, 0.45771432),
	vec2(-0.81544232, -0.87912464),
	vec2(-0.38277543, 0.27676845),
	vec2(0.97484398, 0.75648379),
	vec2(0.44323325, -0.97511554),
	vec2(0.53742981, -0.47373420),
	vec2(-0.26496911, -0.41893023),
	vec2(0.79197514, 0.19090188),
	vec2(-0.24188840, 0.99706507),
	vec2(-0.81409955, 0.91437590),
	vec2(0.19984126, 0.78641367),
	vec2(0.14383161, -0.14100790)
);

float random(vec3 seed, int i) {
	vec4 seed4 = vec4(seed, i);
	float dot_product = dot(seed4, vec4(12.9898, 78.233, 45.164, 94.673));
	return fract(sin(dot_product) * 43758.5453);
}

float shadow_mapping()
{
	vec4 pos = input.shadowcoord;
	vec3 projCoords = pos.xyz / pos.w;

	if (projCoords.z > 1.0)
		return 0.0;
	if (projCoords.x > 1.0 || projCoords.x < 0.0 || projCoords.y > 1.0 || projCoords.y < 0.0)
		return 0.0;

	const float bias = 0.005;
	// Get depth of current fragment from light's perspective
	float currentDepth = projCoords.z - bias;

#define USE_PCF
#ifdef USE_PCF
#define PCF_HALF_SIZE 2
	float shadow = 0.0;
	float pcfRadius = 0.75;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	const int samples = 4;
	for (int i = 0; i < samples; ++i) {
		//int index = i;
		int index = int(16.0 * random(gl_FragCoord.xyy, i)) % 16;
		//int index = int(16.0 * random(floor(input.position.xyz * 1000.0), i)) % 16;
		float pcfDepth = texture(shadowMap, projCoords.xy + poissonDisk[index] * texelSize * pcfRadius).r;
		shadow += currentDepth > pcfDepth ? 1.0 : 0.0;
	}
	return shadow / float(samples);
#else
	float closestDepth = texture(shadowMap, projCoords.xy).r;
	return currentDepth > closestDepth ? 1.0 : 0.0;
#endif
}
#endif // USE_SHADOW_MAP

void main()
{
	// Accumulators
	vec3 ambientComp = globalAmbient * material.ambient;
	vec3 diffuseComp = vec3(0);
	vec3 specularComp = vec3(0);
	vec3 emissionComp = material.emissive;
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
	emissionComp *= texture(emissionMap, texcoord).rgb;
#endif

	float sunAmount = 0.0;
#if defined(USE_DIFFUSE) || defined(USE_SPECULAR)

	if (sunColor.r > 0 || sunColor.g > 0 || sunColor.b > 0) {
		vec3 sunDir = normalize((viewMatrix * vec4(sunPosition, 0.0)).xyz);
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

#ifdef USE_SHADOW_MAP
	float shadow = max(1.0 - shadow_mapping(), 0.1);
	diffuseComp *= shadow;
	specularComp *= shadow;
	//ambientComp *= shadow;
#endif

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

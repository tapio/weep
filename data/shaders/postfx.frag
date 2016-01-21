
in VertexData {
	vec2 texcoord;
} inData;

layout(binding = 20) uniform sampler2D sceneMap;
layout(binding = 21) uniform sampler2D bloomMap;
layout(binding = 22) uniform sampler2D depthMap;

layout(location = 0) out vec4 fragment;

#define gamma(x) pow((x), vec3(1.0 / 2.2));

// http://filmicgames.com/archives/75
vec3 Uncharted2Tonemap(vec3 x)
{
	const float A = 0.15;
	const float B = 0.50;
	const float C = 0.10;
	const float D = 0.20;
	const float E = 0.02;
	const float F = 0.30;
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 ACESFilmicTonemap(vec3 x)
{
	const float A = 2.51;
	const float B = 0.03;
	const float C = 2.43;
	const float D = 0.59;
	const float E = 0.14;
	return saturate((x*(A*x+B))/(x*(C*x+D)+E));
}

/*vec4 textureMultisample(sampler2DMS sampler, vec2 uv)
{
	vec2 texSize = textureSize(sampler);
	uv = floor(texSize * uv);
	ivec2 itexcoord = ivec2(int(uv.s), int(uv.t));

	vec4 color = vec4(0.0);
	for (int i = 0; i < multisamples; i++)
		color += texelFetch(sampler, itexcoord, i);
	color /= float(multisamples);
	return color;
}*/

float linearizeDepth(float depth)
{
	return (2.0 * near) / (far + near - depth * (far - near));
}



void main()
{
	vec2 uv = inData.texcoord;
	vec3 hdrColor;
	// Chromatic Aberration
	if (chromaticAberration > 0) {
		float d = distance(uv, vec2(0.5, 0.5));
		const vec3 chromaticOffsets = vec3(-0.1, 0.0, 0.1) * chromaticAberration * d;
		hdrColor = vec3(
			texture(sceneMap, vec2(uv.x + chromaticOffsets.r, uv.y)).r,
			texture(sceneMap, vec2(uv.x + chromaticOffsets.g, uv.y)).g,
			texture(sceneMap, vec2(uv.x + chromaticOffsets.b, uv.y)).b
		);
	}
	else hdrColor = texture(sceneMap, uv).rgb;

	// Bloom
	if (bloomThreshold > 0)
		hdrColor += texture(bloomMap, uv).rgb; // Bloom

	// Vignette
	if (vignette.x > 0) {
		float d = distance(uv, vec2(0.5, 0.5));
		float vig = smoothstep(vignette.x, vignette.x - vignette.y, d);
		hdrColor = mix(hdrColor, hdrColor * vig, vignette.z);
	}

	// Saturation
	float luminance = dot(hdrColor, vec3(0.3086, 0.6094, 0.0820));
	if (saturation > 0.0) {
		hdrColor.rgb += (luminance - hdrColor.rgb) * (1.0 - 1.0 / (1.001 - saturation));
	} else {
		hdrColor.rgb += (luminance - hdrColor.rgb) * (-saturation);
	}

	// Sepia
	if (sepia > 0) {
		vec3 sepiaColor = vec3(
			dot(hdrColor, vec3(0.393, 0.769, 0.189)),
			dot(hdrColor, vec3(0.349, 0.686, 0.168)),
			dot(hdrColor, vec3(0.272, 0.534, 0.131)));
		hdrColor = mix(hdrColor, sepiaColor, sepia);
	}

	// Scanlines
	if (scanlines > 0)
		if (mod(floor(uv.y * textureSize(sceneMap, 0).y), scanlines + 1.0) < 1.0)
			hdrColor *= luminance;

	// Debug
#if 0 // Visualize depth
	hdrColor = vec3(linearizeDepth(texture(depthMap, uv).r));
#endif
#if 0 // Visualize bloom
	hdrColor = texture(bloomMap, uv).rgb;
#endif

	// Tone mapping & gamma
	hdrColor *= exposure;
	vec3 result;
	if (tonemap == 0) { // Reinhard
		result = hdrColor / (hdrColor + vec3(1.0));
		result = gamma(result);
	} else if (tonemap == 1) { // Exposure
		result = vec3(1.0) - exp(-hdrColor);
		result = gamma(result);
	} else if (tonemap == 2) { // Filmic
		vec3 x = max(vec3(0.0), hdrColor - vec3(0.004));
		result = (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
		// No gamma needed
	} else if (tonemap == 3) { // Uncharted 2
		const float exposureBias = 2.0;
		result = Uncharted2Tonemap(exposureBias * hdrColor);
		vec3 whiteScale = 1.0 / Uncharted2Tonemap(vec3(11.2));
		result *= whiteScale;
		result = gamma(result);
	} else if (tonemap == 4) { // ACES
		result = ACESFilmicTonemap(hdrColor);
		result = gamma(result);
	} else {
		result = vec3(1.0, 0.0, 1.0);
	}

	fragment = vec4(result, 1.0);
}

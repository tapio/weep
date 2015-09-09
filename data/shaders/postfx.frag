
in VertexData {
	vec3 position;
	vec2 texcoord;
	vec3 normal;
} inData;

layout(binding = 20) uniform sampler2DMS sceneMap;

layout(location = 0) out vec4 fragment;


const float A = 0.15;
const float B = 0.50;
const float C = 0.10;
const float D = 0.20;
const float E = 0.02;
const float F = 0.30;
const float W = 11.2;

vec3 Uncharted2Tonemap(vec3 x)
{
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec4 textureMultisample(sampler2DMS sampler, vec2 uv)
{
	vec2 texSize = textureSize(sampler);
	uv = floor(texSize * uv);
	ivec2 itexcoord = ivec2(int(uv.s), int(uv.t));

	vec4 color = vec4(0.0);
	for (int i = 0; i < multisamples; i++)
		color += texelFetch(sampler, itexcoord, i);
	color /= float(multisamples);
	return color;
}


void main()
{
	vec3 hdrColor = textureMultisample(sceneMap, inData.texcoord).rgb;

	// Tone mapping & gamma
	vec3 result;
	if (tonemap == 0) { // Reinhard
		result = hdrColor / (hdrColor + vec3(1.0));
		result = pow(result, vec3(1.0 / 2.2)); // Gamma
	} else if (tonemap == 1) { // Exposure
		result = vec3(1.0) - exp(-hdrColor * exposure);
		result = pow(result, vec3(1.0 / 2.2)); // Gamma
	} else if (tonemap == 2) { // Filmic
		vec3 x = max(vec3(0.0), hdrColor - vec3(0.004));
		result = (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
		// No gamma needed
	} else if (tonemap == 3) { // Uncharted 2
		const float exposureBias = 2.0;
		result = Uncharted2Tonemap(exposureBias * hdrColor);
		vec3 whiteScale = 1.0 / Uncharted2Tonemap(vec3(W));
		result *= whiteScale;
		result = pow(result, vec3(1.0 / 2.2)); // Gamma
	} else {
		result = vec3(1.0, 0.0, 1.0);
	}

	fragment = vec4(result, 1.0);
}

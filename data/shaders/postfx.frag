
in VertexData {
	vec3 position;
	vec2 texcoord;
	vec3 normal;
} inData;

layout(location = 0) out vec4 fragment;

// TODO: Uniform
const float exposure = 1.0;

void main()
{
	vec3 hdrColor = texture(diffuseMap, inData.texcoord).rgb;

	// Tone mapping
	vec3 result;
	if (exposure == 0.0) {
		// Reinhard
		result = hdrColor / (hdrColor + vec3(1.0));
	} else {
		// Exposure
		result = vec3(1.0) - exp(-hdrColor * exposure);
	}
	// Gamma correction
	result = pow(result, vec3(1.0 / 2.2));

	fragment = vec4(result, 1.0);
}

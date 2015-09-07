
in VertexData {
	vec3 position;
	vec2 texcoord;
	vec3 normal;
} inData;

layout(location = 0) out vec4 fragment;

void main()
{
	vec3 hdrColor = texture(diffuseMap, inData.texcoord).rgb;

	// Tone mapping & gamma
	vec3 result;
	if (exposure == 0.0) { 	// Reinhard
		result = hdrColor / (hdrColor + vec3(1.0));
		result = pow(result, vec3(1.0 / 2.2)); // Gamma
	} else if (exposure >= 10.0) { // Filmic
		vec3 x = max(vec3(0.0), hdrColor - vec3(0.004));
		result = (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
		// No gamma needed
	} else { // Exposure
		result = vec3(1.0) - exp(-hdrColor * exposure);
		result = pow(result, vec3(1.0 / 2.2)); // Gamma
	}

	fragment = vec4(result, 1.0);
}

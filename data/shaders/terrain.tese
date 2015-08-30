
layout(triangles, equal_spacing, ccw) in;

in TessCtrlData {
	vec2 texcoord;
	vec3 normal;
	vec3 fragPosition;
} inp[];

out VertexData {
	vec2 texcoord;
	vec3 normal;
	vec3 fragPosition;
} outp;

#define tc gl_TessCoord

void main()
{
	// Trivial point location
	vec3 p0 = tc.x * inp[0].fragPosition;
	vec3 p1 = tc.y * inp[1].fragPosition;
	vec3 p2 = tc.z * inp[2].fragPosition;
	vec3 position = p0 + p1 + p2;

	// Trivial texcoord
	vec2 uv0 = tc.x * inp[0].texcoord;
	vec2 uv1 = tc.y * inp[1].texcoord;
	vec2 uv2 = tc.z * inp[2].texcoord;
	outp.texcoord = uv0 + uv1 + uv2;

	// Trivial normal
	vec3 n0 = tc.x * inp[0].normal;
	vec3 n1 = tc.y * inp[1].normal;
	vec3 n2 = tc.z * inp[2].normal;
	outp.normal = normalize(n0 + n1 + n2);

	// Displace
	float height = texture(heightMap, outp.texcoord).r;
	position.y += height;

	outp.fragPosition = position;

	// Project
	gl_Position = projectionMatrix * vec4(position, 1.0);
}


#version 430

layout(triangles, equal_spacing, ccw) in;

layout(binding = 0, std140) uniform CommonBlock {
	mat4 modelMatrix;
	mat4 modelViewMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 normalMatrix; // Problems with alignment if sent as mat3
	vec3 cameraPosition; float pad1;
};

layout(location = 3) uniform sampler2D heightMap;

in TessCtrlData {
	vec3 position;
	vec2 texcoord;
	vec3 normal;
} inp[];

out TessEvalData {
	vec2 texcoord;
	vec3 normal;
} outp;

#define tc gl_TessCoord

void main()
{
	// Trivial point location
	vec3 p0 = tc.x * inp[0].position;
	vec3 p1 = tc.y * inp[1].position;
	vec3 p2 = tc.z * inp[2].position;
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

	// Project
	gl_Position = projectionMatrix * vec4(position, 1.0);
}


#version 430
layout(vertices = 3) out;

layout(binding = 0, std140) uniform CommonBlock {
	mat4 modelMatrix;
	mat4 modelViewMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat3 normalMatrix;
	vec3 cameraPosition;
};

in VertexData {
	vec3 position;
	vec2 texcoord;
	vec3 normal;
} inp[];

out TessCtrlData {
	vec3 position;
	vec2 texcoord;
	vec3 normal;
} outp[];

#define ID gl_InvocationID

void main()
{
	outp[ID].position = inp[ID].position;
	outp[ID].texcoord = inp[ID].texcoord;
	outp[ID].normal = inp[ID].normal;

	if (ID == 0) {
		vec3 diff = cameraPosition - inp[0].position;
		float dist2 = dot(diff, diff);

		float tessLevel = 1;
		if (dist2 < 1 * 1) tessLevel = 8;
		else if (dist2 < 2 * 2) tessLevel = 4;
		else if (dist2 < 5 * 5) tessLevel = 2;

		gl_TessLevelInner[0] = tessLevel;
		gl_TessLevelOuter[0] = tessLevel;
		gl_TessLevelOuter[1] = tessLevel;
		gl_TessLevelOuter[2] = tessLevel;
	}
}


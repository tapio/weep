#version 430
layout(vertices = 3) out;

in VertexData {
	vec3 position;
	vec2 texcoord;
	vec3 normal;
} inp[];

out TessCtrlData {
	vec3 position;
	vec2 texcoord;
	vec3 normal;
	float curvature;
} outp[];

#define ID gl_InvocationID

const float tessLevel = 2;

void main()
{
	outp[ID].position = inp[ID].position;
	outp[ID].texcoord = inp[ID].texcoord;
	outp[ID].normal = inp[ID].normal;

	// Calculate curvature factors for each edge
	// Uses the angles between the corner normals of the patch
	outp[ID].curvature = 1.0 - dot(normalize(inp[ID].normal), normalize(inp[(ID+1)%3].normal));

	if (ID == 0) {
		// Set tessellation levels
		gl_TessLevelInner[0] = tessLevel;
		gl_TessLevelOuter[0] = tessLevel;
		gl_TessLevelOuter[1] = tessLevel;
		gl_TessLevelOuter[2] = tessLevel;
	}
}



layout(vertices = 3) out;

in VertexData {
	vec2 texcoord;
	vec3 normal;
	vec3 fragPosition;
} inp[];

out VertexData {
	vec2 texcoord;
	vec3 normal;
	vec3 fragPosition;
} outp[];

out float curvature[];

#define ID gl_InvocationID

const float tessLevel = 8;

void main()
{
	outp[ID].fragPosition = inp[ID].fragPosition;
	outp[ID].texcoord = inp[ID].texcoord;
	outp[ID].normal = inp[ID].normal;

	// Calculate curvature factors for each edge
	// Uses the angles between the corner normals of the patch
	curvature[ID] = 1.0 - dot(normalize(inp[ID].normal), normalize(inp[(ID+1)%3].normal));

	if (ID == 0) {
		// Set tessellation levels
		gl_TessLevelInner[0] = tessLevel;
		gl_TessLevelOuter[0] = tessLevel;
		gl_TessLevelOuter[1] = tessLevel;
		gl_TessLevelOuter[2] = tessLevel;
	}
}


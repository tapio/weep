
layout(vertices = 3) out;

in VertexData {
	vec2 texcoord;
	vec3 normal;
	vec3 fragPosition;
} inp[];

out TessCtrlData {
	vec2 texcoord;
	vec3 normal;
	vec3 fragPosition;
} outp[];

#define ID gl_InvocationID

void main()
{
	outp[ID].fragPosition = inp[ID].fragPosition;
	outp[ID].texcoord = inp[ID].texcoord;
	outp[ID].normal = inp[ID].normal;

	if (ID == 0) {
		float dist2 = dot(inp[0].fragPosition, inp[0].fragPosition);

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


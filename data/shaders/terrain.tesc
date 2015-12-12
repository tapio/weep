
layout(vertices = 3) out;

VERTEX_DATA(in, inp[]);
VERTEX_DATA(out, outp[]);


#define ID gl_InvocationID

void main()
{
	outp[ID].position = inp[ID].position;
	outp[ID].texcoord = inp[ID].texcoord;
	outp[ID].normal = inp[ID].normal;
#ifdef USE_SHADOW_MAP
	outp[ID].shadowcoord = inp[ID].shadowcoord;
#endif

	if (ID == 0) {
		float dist2 = dot(inp[0].position, inp[0].position);

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



layout(vertices = 3) out;

VERTEX_DATA(in, inp[]);
VERTEX_DATA(out, outp[]);

out float curvature[];

#define ID gl_InvocationID

const float tessLevel = 8;

void main()
{
	outp[ID].position = inp[ID].position;
	outp[ID].texcoord = inp[ID].texcoord;
	outp[ID].normal = inp[ID].normal;
#ifdef USE_SHADOW_MAP
	outp[ID].shadowcoord = inp[ID].shadowcoord;
	outp[ID].worldPosition = inp[ID].worldPosition;
#endif
#ifdef USE_VERTEX_COLOR
	outp[ID].color = inp[ID].color;
#endif

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



layout(vertices = 3) out;

VERTEX_DATA(in, inData[]);
VERTEX_DATA(out, outData[]);


#define ID gl_InvocationID

void main()
{
	outData[ID].position = inData[ID].position;
	outData[ID].texcoord = inData[ID].texcoord;
	outData[ID].normal = inData[ID].normal;
#ifdef USE_SHADOW_MAP
	for (int i = 0; i < MAX_SHADOW_MAPS; ++i)
		outData[ID].shadowcoords[i] = inData[ID].shadowcoords[i];
	outData[ID].worldPosition = inData[ID].worldPosition;
#endif
#ifdef USE_VERTEX_COLOR
	outData[ID].color = inData[ID].color;
#endif

	if (ID == 0) {
		float dist2 = dot(inData[0].position, inData[0].position);

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


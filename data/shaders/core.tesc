
layout(vertices = 3) out;

VERTEX_DATA(in, inData[]);
VERTEX_DATA(out, outData[]);

#define ID gl_InvocationID

float calculateTessLevel(float dist)
{
	const float minLevel = 1.f;
	const float maxLevel = 4.f;
	const float minDist = 1.f;
	const float maxDist = 12.f;
	float d = clamp(dist, minDist, maxDist);
	float alpha = 1.0 - (d - minDist) / (maxDist - minDist);
	return mix(minLevel, maxLevel, alpha * alpha * alpha);
}

void main()
{
	outData[ID].position = inData[ID].position;
	outData[ID].texcoord = inData[ID].texcoord;
	outData[ID].normal = inData[ID].normal;
#ifdef USE_SHADOW_MAP
	outData[ID].shadowcoord = inData[ID].shadowcoord;
	outData[ID].worldPosition = inData[ID].worldPosition;
#endif
#ifdef USE_VERTEX_COLOR
	outData[ID].color = inData[ID].color;
#endif

	// Set tessellation levels
	const int idA = (ID + 1) % 3;
	const int idB = (ID + 2) % 3;
	const vec3 camPos = cameraPosition;
	float distA = distance(camPos, inData[idA].worldPosition);
	float distB = distance(camPos, inData[idB].worldPosition);
	gl_TessLevelOuter[ID] = calculateTessLevel((distA + distB) * 0.5);
	if (ID == 0)
		gl_TessLevelInner[0] = gl_TessLevelOuter[0];
}


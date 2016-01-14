
layout(vertices = 3) out;

VERTEX_DATA(in, inData[]);
VERTEX_DATA(out, outData[]);

out float curvature[];

#define ID gl_InvocationID

float calculateTessLevel(float dist)
{
	const float minLevel = 1.f;
	const float maxLevel = 8.f;
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

	// Calculate curvature factors for each edge
	// Uses the angles between the corner normals of the patch
	curvature[ID] = 1.0 - dot(normalize(inData[ID].normal), normalize(inData[(ID+1)%3].normal));

	// Set tessellation levels
	if (ID == 0) {
		const vec3 camPos = cameraPosition;
		float dist0 = distance(camPos, inData[0].worldPosition);
		float dist1 = distance(camPos, inData[1].worldPosition);
		float dist2 = distance(camPos, inData[2].worldPosition);
		gl_TessLevelOuter[0] = calculateTessLevel((dist1 + dist2) * 0.5);
		gl_TessLevelOuter[1] = calculateTessLevel((dist2 + dist0) * 0.5);
		gl_TessLevelOuter[2] = calculateTessLevel((dist0 + dist1) * 0.5);
		gl_TessLevelInner[0] = gl_TessLevelOuter[2];

	}
}


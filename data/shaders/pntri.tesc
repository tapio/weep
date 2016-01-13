
layout(vertices = 3) out;

VERTEX_DATA(in, inp[]);
VERTEX_DATA(out, outp[]);

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

	// Set tessellation levels
	if (ID == 0) {
		const vec3 camPos = cameraPosition;
		float dist0 = distance(camPos, inp[0].worldPosition);
		float dist1 = distance(camPos, inp[1].worldPosition);
		float dist2 = distance(camPos, inp[2].worldPosition);
		gl_TessLevelOuter[0] = calculateTessLevel((dist1 + dist2) * 0.5);
		gl_TessLevelOuter[1] = calculateTessLevel((dist2 + dist0) * 0.5);
		gl_TessLevelOuter[2] = calculateTessLevel((dist0 + dist1) * 0.5);
		gl_TessLevelInner[0] = gl_TessLevelOuter[2];

	}
}


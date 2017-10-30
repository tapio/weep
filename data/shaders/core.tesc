
layout(vertices = 3) out;

VERTEX_DATA(in, inData[]);
VERTEX_DATA(out, outData[]);

#ifdef GL_ES
	#define CONST
#else
	#define CONST const
#endif

#define ID gl_InvocationID

vec2 project(vec3 p)
{
	vec4 res = projectionMatrix * vec4(p, 1.0);
	return res.xy / res.w * 0.5 + 0.5;
}

float calculateTessLevel(float dist)
{
	const float minLevel = 1.0;
	const float maxLevel = 4.0;
	const float minDist = 0.04;
	const float maxDist = 0.1;
	float d = clamp(dist, minDist, maxDist);
	float alpha = 1.0 - (d - minDist) / (maxDist - minDist);
	return mix(maxLevel, minLevel, alpha * alpha);
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
	CONST int idA = (ID + 0) % 3;
	CONST int idB = (ID + 1) % 3;
	float dist = distance(project(inData[idA].position), project(inData[idB].position));
	gl_TessLevelOuter[ID] = calculateTessLevel(dist);
	barrier();
	if (ID == 0)
		gl_TessLevelInner[0] = (gl_TessLevelOuter[0] + gl_TessLevelOuter[1] + gl_TessLevelOuter[2]) / 3.0;
}


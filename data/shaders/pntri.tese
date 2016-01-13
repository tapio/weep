
layout(triangles, equal_spacing, ccw) in;

VERTEX_DATA(in, inData[]);
in float curvature[];
VERTEX_DATA(out, outData);

vec2 interp2(vec2 a, vec2 b, vec2 c) {
	return gl_TessCoord.x * a + gl_TessCoord.y * b + gl_TessCoord.z * c;
}

vec3 interp3(vec3 a, vec3 b, vec3 c) {
	return gl_TessCoord.x * a + gl_TessCoord.y * b + gl_TessCoord.z * c;
}

vec4 interp4(vec4 a, vec4 b, vec4 c) {
	return gl_TessCoord.x * a + gl_TessCoord.y * b + gl_TessCoord.z * c;
}

#define tc gl_TessCoord

// <1.0 - Less roundness when approaching zero
// ~1.0 ~ Sphere
// >1.0 - Original polygons pop out
const float exaggeration = 0.3;

void main()
{
	vec3 position = interp3(inData[0].position, inData[1].position, inData[2].position);
	outData.texcoord = interp2(inData[0].texcoord, inData[1].texcoord, inData[2].texcoord);
	outData.normal = normalize(interp3(inData[0].normal, inData[1].normal, inData[2].normal));
#ifdef USE_SHADOW_MAP
	outData.shadowcoord = interp4(inData[0].shadowcoord, inData[1].shadowcoord, inData[2].shadowcoord);
	outData.worldPosition = interp3(inData[0].worldPosition, inData[1].worldPosition, inData[2].worldPosition);
#endif
#ifdef USE_VERTEX_COLOR
	outData.color = interp4(inData[0].color, inData[1].color, inData[2].color);
#endif

	// Calculate distance to each edge
	float d0 = tc.y * tc.z;
	float d1 = tc.x * tc.z;
	float d2 = tc.x * tc.y;

	// Calculate total displacement
	//float f = inData[0].curvature * d2 + inData[1].curvature * d0 + inData[2].curvature * d1;
	float f = curvature[0] * d2 + curvature[1] * d0 + curvature[2] * d1;

	// Displace
	position += exaggeration * f * outData.normal;

	outData.position = position;

	// Project
	gl_Position = projectionMatrix * vec4(position, 1.0);
}


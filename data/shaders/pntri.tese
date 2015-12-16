
layout(triangles, equal_spacing, ccw) in;

VERTEX_DATA(in, inp[]);
in float curvature[];
VERTEX_DATA(out, outp);

#define tc gl_TessCoord

// <1.0 - Less roundness when approaching zero
// ~1.0 ~ Sphere
// >1.0 - Original polygons pop out
const float exaggeration = 0.3;

void main()
{
	// Trivial point location
	vec3 p0 = tc.x * inp[0].position;
	vec3 p1 = tc.y * inp[1].position;
	vec3 p2 = tc.z * inp[2].position;
	vec3 position = p0 + p1 + p2;

	// Trivial texcoord
	vec2 uv0 = tc.x * inp[0].texcoord;
	vec2 uv1 = tc.y * inp[1].texcoord;
	vec2 uv2 = tc.z * inp[2].texcoord;
	outp.texcoord = uv0 + uv1 + uv2;

	// Trivial normal
	vec3 n0 = tc.x * inp[0].normal;
	vec3 n1 = tc.y * inp[1].normal;
	vec3 n2 = tc.z * inp[2].normal;
	outp.normal = normalize(n0 + n1 + n2);

#ifdef USE_SHADOW_MAP
	// Trivial shadowcoord
	vec4 sc0 = tc.x * inp[0].shadowcoord;
	vec4 sc1 = tc.y * inp[1].shadowcoord;
	vec4 sc2 = tc.z * inp[2].shadowcoord;
	outp.shadowcoord = sc0 + sc1 + sc2;

	vec3 wp0 = tc.x * inp[0].worldPosition;
	vec3 wp1 = tc.y * inp[1].worldPosition;
	vec3 wp2 = tc.z * inp[2].worldPosition;
	outp.worldPosition = wp0 + wp1 + wp2;
#endif

	// Calculate distance to each edge
	float d0 = tc.y * tc.z;
	float d1 = tc.x * tc.z;
	float d2 = tc.x * tc.y;

	// Calculate total displacement
	//float f = inp[0].curvature * d2 + inp[1].curvature * d0 + inp[2].curvature * d1;
	float f = curvature[0] * d2 + curvature[1] * d0 + curvature[2] * d1;

	// Displace
	position += exaggeration * f * outp.normal;

	outp.position = position;

	// Project
	gl_Position = projectionMatrix * vec4(position, 1.0);
}


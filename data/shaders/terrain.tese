
layout(triangles, equal_spacing, ccw) in;

VERTEX_DATA(in, inp[]);
VERTEX_DATA(out, outp);

#define tc gl_TessCoord

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

#ifdef USE_VERTEX_COLOR
	vec3 c0 = tc.x * inp[0].color;
	vec3 c1 = tc.y * inp[1].color;
	vec3 c2 = tc.z * inp[2].color;
	outp.color = normalize(c0 + c1 + c2);
#endif

	// Displace
	float height = texture(heightMap, outp.texcoord).r;
	position.y += height;

	outp.position = position;

	// Project
	gl_Position = projectionMatrix * vec4(position, 1.0);
}


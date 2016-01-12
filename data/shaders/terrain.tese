
layout(triangles, equal_spacing, ccw) in;

VERTEX_DATA(in, inp[]);
VERTEX_DATA(out, outp);

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

void main()
{
	vec3 position = interp3(inp[0].position, inp[1].position, inp[2].position);
	outp.texcoord = interp2(inp[0].texcoord, inp[1].texcoord, inp[2].texcoord);
	outp.normal = normalize(interp3(inp[0].normal, inp[1].normal, inp[2].normal));
#ifdef USE_SHADOW_MAP
	outp.shadowcoord = interp4(inp[0].shadowcoord, inp[1].shadowcoord, inp[2].shadowcoord);
	outp.worldPosition = interp3(inp[0].worldPosition, inp[1].worldPosition, inp[2].worldPosition);
#endif
#ifdef USE_VERTEX_COLOR
	outp.color = interp4(inp[0].color, inp[1].color, inp[2].color);
#endif

	// Displace
	float height = texture(heightMap, outp.texcoord).r;
	position.y += height;

	outp.position = position;

	// Project
	gl_Position = projectionMatrix * vec4(position, 1.0);
}


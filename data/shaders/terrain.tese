
layout(triangles, equal_spacing, ccw) in;

VERTEX_DATA(in, inData[]);
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

void main()
{
	vec3 position = interp3(inData[0].position, inData[1].position, inData[2].position);
	outData.texcoord = interp2(inData[0].texcoord, inData[1].texcoord, inData[2].texcoord);
	outData.normal = normalize(interp3(inData[0].normal, inData[1].normal, inData[2].normal));
#ifdef USE_SHADOW_MAP
	for (int i = 0; i < MAX_SHADOW_MAPS; ++i)
		outData.shadowcoords[i] = interp4(inData[0].shadowcoords[i], inData[1].shadowcoords[i], inData[2].shadowcoords[i]);
	outData.worldPosition = interp3(inData[0].worldPosition, inData[1].worldPosition, inData[2].worldPosition);
#endif
#ifdef USE_VERTEX_COLOR
	outData.color = interp4(inData[0].color, inData[1].color, inData[2].color);
#endif

	// Displace
	float height = texture(heightMap, outData.texcoord).r;
	position.y += height;

	outData.position = position;

	// Project
	gl_Position = projectionMatrix * vec4(position, 1.0);
}


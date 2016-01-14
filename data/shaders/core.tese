
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

// http://ogldev.atspace.co.uk/www/tutorial31/tutorial31.html
vec3 projectToPlane(vec3 point, vec3 planePoint, vec3 planeNormal)
{
	vec3 v = point - planePoint;
	float len = dot(v, planeNormal);
	vec3 d = len * planeNormal;
	return (point - d);
}

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

	vec3 p0 = projectToPlane(position, inData[0].position, normalize(inData[0].normal));
	vec3 p1 = projectToPlane(position, inData[1].position, normalize(inData[1].normal));
	vec3 p2 = projectToPlane(position, inData[2].position, normalize(inData[2].normal));
	vec3 phongPos = interp3(p0, p1, p2);

	outData.position = mix(position, phongPos, 0.5);

	// Project
	gl_Position = projectionMatrix * vec4(outData.position, 1.0);
}


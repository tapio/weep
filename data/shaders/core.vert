
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;

out VertexData {
	vec3 position;
	vec2 texcoord;
	vec3 normal;
	vec4 shadowcoord;
} outData;

void main()
{
	outData.texcoord = texcoord * material.uvRepeat + material.uvOffset;
	outData.normal = mat3(normalMatrix) * normal;
	outData.position = (modelViewMatrix * vec4(position, 1.0)).xyz;
	outData.shadowcoord = shadowMatrix * vec4(position, 1.0);
#ifndef USE_TESSELLATION
	gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
#endif
}


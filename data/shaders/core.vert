
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;

VERTEX_DATA(out, outData);

void main()
{
	outData.texcoord = texcoord * material.uvRepeat + material.uvOffset;
	outData.normal = mat3(normalMatrix) * normal;
	outData.position = (modelViewMatrix * vec4(position, 1.0)).xyz;
#ifdef USE_SHADOW_MAP
	outData.shadowcoord = shadowMatrix * vec4(position, 1.0);
#endif
#ifndef USE_TESSELLATION
	gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
#endif
}


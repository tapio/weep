
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec4 color;
#ifdef USE_SKINNING
layout(location = 4) in vec4 boneWeights;
layout(location = 5) in ivec4 boneIndices;
#endif

VERTEX_DATA(out, outData);

void main()
{
#ifdef USE_SKINNING
	mat3x4 m = boneMatrices[boneIndices.x] * boneWeights.x;
	m += boneMatrices[boneIndices.y] * boneWeights.y;
	m += boneMatrices[boneIndices.z] * boneWeights.z;
	m += boneMatrices[boneIndices.w] * boneWeights.w;
	gl_Position = modelViewProjMatrix * vec4(position * m, 1.0);
#endif
	outData.texcoord = texcoord * material.uvRepeat + material.uvOffset;
	outData.normal = mat3(normalMatrix) * normal;
	outData.position = (modelViewMatrix * vec4(position, 1.0)).xyz;
#ifdef USE_SHADOW_MAP
	outData.worldPosition = (modelMatrix * vec4(position, 1.0)).xyz;
	outData.shadowcoord = shadowMatrix * vec4(position, 1.0);
#endif
#if !(USE_TESSELLATION) && !defined(USE_SKINNING)
	gl_Position = modelViewProjMatrix * vec4(position, 1.0);
#endif
}


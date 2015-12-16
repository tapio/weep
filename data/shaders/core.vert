
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
	vec4 mpos = vec4(position, 1.0);
	mpos = vec4(mpos * m, 1.0);
	gl_Position = modelViewProjMatrix * mpos;
	outData.normal = mat3(normalMatrix) * (vec4(normal, 0.0) * m).xyz;
	outData.position = (modelViewMatrix * mpos).xyz;
#else
// Not skinned
#ifndef USE_TESSELLATION
	gl_Position = modelViewProjMatrix * vec4(position, 1.0);
#endif
	outData.normal = mat3(normalMatrix) * normal;
	outData.position = (modelViewMatrix * vec4(position, 1.0)).xyz;
#endif

	outData.texcoord = texcoord * material.uvRepeat + material.uvOffset;

#ifdef USE_SHADOW_MAP
	outData.worldPosition = (modelMatrix * vec4(position, 1.0)).xyz;
	outData.shadowcoord = shadowMatrix * vec4(position, 1.0);
#endif
}


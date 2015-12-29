
layout(location = 0) in vec3 position;
#ifdef USE_ALPHA_TEST
layout(location = 1) in vec2 texcoord;
#endif
#ifdef USE_SKINNING
layout(location = 4) in vec4 boneIndices;
layout(location = 5) in vec4 boneWeights;
#endif

#if defined(USE_ALPHA_TEST) || defined(USE_DEPTH_CUBE)
out VertexData {
#ifdef USE_DEPTH_CUBE
	vec3 position;
#endif
#ifdef USE_ALPHA_TEST
	vec2 texcoord;
#endif
} outData;
#endif

void main()
{
	vec4 pos = vec4(position, 1.0);
#ifdef USE_SKINNING
	mat3x4 m = boneMatrices[int(boneIndices.x)] * boneWeights.x;
	m += boneMatrices[int(boneIndices.y)] * boneWeights.y;
	m += boneMatrices[int(boneIndices.z)] * boneWeights.z;
	m += boneMatrices[int(boneIndices.w)] * boneWeights.w;
	pos = vec4(pos * m, 1.0);
#endif // USE_SKINNING
#ifdef USE_DEPTH_CUBE
	gl_Position = modelMatrix * pos;
#else
	gl_Position = modelViewProjMatrix * pos;
#endif
#ifdef USE_ALPHA_TEST
	outData.texcoord = texcoord * material.uvRepeat + material.uvOffset;
#endif
}


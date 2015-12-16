
layout(location = 0) in vec3 position;
#ifdef USE_ALPHA_TEST
layout(location = 1) in vec2 texcoord;
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
#ifdef USE_ALPHA_TEST
	outData.texcoord = texcoord * material.uvRepeat + material.uvOffset;
#endif
#ifdef USE_DEPTH_CUBE
	gl_Position = modelMatrix * vec4(position, 1.0);
#else
	gl_Position = modelViewProjMatrix * vec4(position, 1.0);
#endif
}


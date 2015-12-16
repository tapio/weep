
#if defined(USE_ALPHA_TEST) || defined(USE_DEPTH_CUBE)
in VertexData {
#ifdef USE_DEPTH_CUBE
	vec3 position;
#endif
#ifdef USE_ALPHA_TEST
	vec2 texcoord;
#endif
} inData;
#endif

void main()
{
#ifdef USE_ALPHA_TEST
	vec4 diffuseTex = texture(diffuseMap, inData.texcoord);
	if (diffuseTex.a < 0.8)
		discard;
#endif
#ifdef USE_DEPTH_CUBE
	// Get distance between fragment and light source and normalize to far plane
	float lightDistance = length(inData.position - cameraPosition) / far;
	gl_FragDepth = lightDistance;
#endif
}

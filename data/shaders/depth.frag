
#ifdef USE_DEPTH_CUBE
in vec4 fragPos;
#endif

void main()
{
#ifdef USE_DEPTH_CUBE
	// Get distance between fragment and light source and normalize to far plane
	float lightDistance = length(fragPos.xyz - cameraPosition) / far;
	gl_FragDepth = lightDistance;
#endif
}

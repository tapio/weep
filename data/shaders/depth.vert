
layout(location = 0) in vec3 position;

void main()
{
#ifdef USE_DEPTH_CUBE
	gl_Position = modelMatrix * vec4(position, 1.0);
#else
	gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
#endif
}


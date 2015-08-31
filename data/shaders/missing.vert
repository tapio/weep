
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;

void main()
{
	gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
}


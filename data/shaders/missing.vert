
layout(location = ATTR_POSITION) in vec3 position;

void main()
{
	gl_Position = modelViewProjMatrix * vec4(position, 1.0);
}



layout(location = ATTR_POSITION) in vec3 position;

layout(std430, binding = 30) buffer positionBuffer
{
	vec3 pos[];
};

void main()
{
	vec3 particlePos = pos[gl_VertexID / 3];
	/*vec3 offset = vec3(-0.1, 0.0, 0.0);
	if ((gl_VertexID % 3) == 1)
		offset = vec3(0.0, -0.1, 0.0);
	else if ((gl_VertexID % 2) == 1)
		offset = vec3(0.0, 0.0, -0.1);*/

	gl_Position = modelViewProjMatrix * vec4(particlePos + position, 1.0);
}

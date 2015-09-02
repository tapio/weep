
layout (location = 0) in vec3 position;
out vec3 texcoord;

void main()
{
	texcoord = position;
	vec4 pos = projectionMatrix * viewMatrix * vec4(position, 1.0);
	gl_Position = pos.xyww;
}

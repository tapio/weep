
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;

out VertexData {
	vec2 texcoord;
} outData;

void main()
{
	outData.texcoord = texcoord;
	gl_Position = vec4(position, 1.0);
}


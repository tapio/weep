
layout(location = ATTR_POSITION) in vec3 position;
layout(location = ATTR_TEXCOORD) in vec2 texcoord;

out VertexData {
	vec2 texcoord;
} outData;

void main()
{
	outData.texcoord = texcoord;
	gl_Position = vec4(position, 1.0);
}


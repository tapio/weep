
layout (location = 0) in vec3 position;

out VertexData {
	vec3 texcoord;
} outData;

void main()
{
	outData.texcoord = position;
#ifdef USE_CUBE_RENDER
	gl_Position = vec4(position, 1.0);
#else
	vec4 pos = projectionMatrix * viewMatrix * vec4(position, 1.0);
	gl_Position = pos.xyww;
#endif
}

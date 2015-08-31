
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;

out VertexData {
	vec3 position;
	vec2 texcoord;
	vec3 normal;
} output;

void main()
{
	output.normal = mat3(normalMatrix) * normal;
	gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
}


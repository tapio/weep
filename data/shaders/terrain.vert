#version 430

layout(binding = 0, std140) uniform CommonBlock {
	mat4 modelMatrix;
	mat4 modelViewMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat3 normalMatrix;
	vec3 cameraPosition;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;

out VertexData {
	vec3 position;
	vec2 texcoord;
	vec3 normal;
} outp;

void main()
{
	outp.texcoord = texcoord;
	outp.normal = (modelViewMatrix * vec4(normal, 0.0)).xyz;
	outp.position = (modelViewMatrix * vec4(position.x, 0.0, position.z, 1.0)).xyz;
}

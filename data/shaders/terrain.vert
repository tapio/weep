#version 430

layout(binding = 0, std140) uniform CommonBlock {
	mat4 modelMatrix;
	mat4 modelViewMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 normalMatrix; // Problems with alignment if sent as mat3
	vec3 cameraPosition; float pad1;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;

out VertexData {
	vec3 position;
	vec2 texcoord;
	vec3 normal;
	vec3 eye;
} outp;

void main()
{
	outp.texcoord = texcoord;
	outp.normal = mat3(normalMatrix) * normal;
	outp.eye = (modelMatrix * vec4(position.x, 0.0, position.z, 1.0)).xyz - cameraPosition;
	outp.position = (modelViewMatrix * vec4(position.x, 0.0, position.z, 1.0)).xyz;
}

#version 330
#extension GL_ARB_shading_language_420pack : enable

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
	vec2 texcoord;
	vec3 normal;
	vec3 fragPosition;
} output;

void main()
{
	output.texcoord = texcoord;
	output.normal = mat3(normalMatrix) * normal;
	output.fragPosition = (modelMatrix * vec4(position, 1.0)).xyz;
	gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
}


#version 330
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_explicit_uniform_location : enable

layout(binding = 1, std140) uniform ColorBlock {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

layout(location = 0) uniform sampler2D diffuseMap;

in VertexData {
	vec2 texcoord;
	vec3 normal;
} input;

layout(location = 0) out vec4 fragment;

void main()
{
	vec4 tex = texture(diffuseMap, input.texcoord);
	fragment = vec4(ambient, 0.0) + tex * vec4(diffuse, 1.0);
}

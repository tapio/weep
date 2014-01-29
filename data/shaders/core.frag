#version 330
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 1, std140) uniform ColorBlock {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

layout(location = 0) out vec4 fragment;

void main()
{
	vec3 color = ambient + diffuse;
	fragment = vec4(color, 1.0);
}

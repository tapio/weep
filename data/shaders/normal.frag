#version 330
#extension GL_ARB_shading_language_420pack : enable

in VertexData {
	vec3 normal;
} input;

layout(location = 0) out vec4 fragment;

void main()
{
	fragment = vec4(0.5 * normalize(input.normal) + 0.5, 1.0);
}

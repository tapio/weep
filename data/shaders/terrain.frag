#version 430

layout(binding = 1, std140) uniform ColorBlock {
	vec3 ambient; float pad1;
	vec3 diffuse; float pad2;
	vec3 specular; float pad3;
	float shininess;
} material;

layout(location = 0) uniform sampler2D diffuseMap;

in TessEvalData {
	vec2 texcoord;
	vec3 normal;
} inp;

layout(location = 0) out vec4 fragment;

void main()
{
	vec4 tex = texture(diffuseMap, inp.texcoord);
	fragment = vec4(material.ambient, 0.0) + tex * vec4(material.diffuse, 1.0);
}

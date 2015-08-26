#version 430

layout(binding = 1, std140) uniform ColorBlock {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

layout(location = 0) uniform sampler2D diffuseMap;

in TessEvalData {
	vec2 texcoord;
	vec3 normal;
} inp;

layout(location = 0) out vec4 fragment;

void main()
{
	vec4 tex = texture(diffuseMap, inp.texcoord);
	fragment = vec4(ambient, 0.0) + tex * vec4(diffuse, 1.0);
}


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

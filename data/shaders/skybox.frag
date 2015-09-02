
in vec3 texcoord;
out vec4 fragment;

void main()
{
	fragment = texture(envMap, texcoord);
}


VERTEX_DATA(in, input);

layout(location = 0) out vec4 fragment;

void main()
{
	fragment = vec4(0.5 * normalize(input.normal) + 0.5, 1.0);
}

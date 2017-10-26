
VERTEX_DATA(in, inData);

layout(location = 0) out vec4 fragment;

void main()
{
	fragment = vec4(0.5 * normalize(inData.normal) + 0.5, 1.0);
}

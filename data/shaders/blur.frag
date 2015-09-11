in VertexData {
	vec3 position;
	vec2 texcoord;
	vec3 normal;
} inData;

layout(binding = 20) uniform sampler2D image;

layout(location = 0) out vec4 fragment;

uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
	vec2 tex_offset = 1.0 / textureSize(image, 0);
	vec3 result = texture(image, inData.texcoord).rgb * weight[0];
#ifdef BLUR_HORIZONTAL
	for(int i = 1; i < 5; ++i)
	{
		result += texture(image, inData.texcoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
		result += texture(image, inData.texcoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
	}
#elif defined(BLUR_VERTICAL)
	for(int i = 1; i < 5; ++i)
	{
		result += texture(image, inData.texcoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
		result += texture(image, inData.texcoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
	}
#else
#error "No blur direction defined"
#endif

	fragment = vec4(result, 1.0);
}

in vec3 texcoord;

layout(location = 0) out vec4 fragment;
layout(location = 1) out vec4 brightFragment;

void main()
{
	fragment = texture(envMap, texcoord);

	if (bloomThreshold > 0.f) {
		//float brightness = dot(fragment.rgb, vec3(0.2126, 0.7152, 0.0722));
		//brightFragment = brightness > bloomThreshold ? vec4(fragment.rgb, 1.0) : vec4(0.0, 0.0, 0.0, 1.0);
		brightFragment = fragment;
	} else brightFragment = vec4(0.0, 0.0, 0.0, 1.0);
}

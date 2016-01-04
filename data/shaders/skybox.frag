
in VertexData {
	vec3 texcoord;
};

layout(location = 0) out vec4 fragment;
#ifndef USE_CUBE_RENDER
layout(location = 1) out vec4 brightFragment;
#endif

void main()
{
	fragment = texture(envMap, texcoord);

#ifndef USE_CUBE_RENDER
	if (bloomThreshold > 0.f) {
		//float brightness = dot(fragment.rgb, vec3(0.2126, 0.7152, 0.0722));
		//brightFragment = brightness > bloomThreshold ? vec4(fragment.rgb, 1.0) : vec4(0.0, 0.0, 0.0, 1.0);
		brightFragment = fragment;
	} else brightFragment = vec4(0.0, 0.0, 0.0, 1.0);
#endif
}

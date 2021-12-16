
layout(location = ATTR_POSITION) in vec3 position;
layout(location = ATTR_TEXCOORD) in vec2 texcoord;

#ifdef USE_VERTEX_COLOR
layout(location = ATTR_COLOR) in vec4 color;
#endif

VERTEX_DATA(out, outData);

layout(std430, binding = BINDING_SSBO_POSITION) buffer positionBuffer
{
	vec3 pos[];
};

layout(std430, binding = BINDING_SSBO_LIFE) buffer LifeBuffer
{
	vec2 life[];
};

layout(std430, binding = BINDING_SSBO_EXTRA) buffer ExtraBuffer
{
	float rot[];
};

vec2 rotate2d(vec2 v, float a) {
	float s = sin(a);
	float c = cos(a);
	mat2 m = mat2(c, -s, s, c);
	return m * v;
}

void main()
{
	uint particleId = gl_InstanceID;
	vec4 particlePos = vec4(pos[particleId], 1.0);
	float phase = 1.0 - life[particleId].x / life[particleId].y;
	float phaseCurve = sqrt(sin(PI * phase)); // Inverted U
	vec4 localVertexPos = vec4(position * vec3(material.particleSize * phaseCurve, 0.0), 1.0);

	float rot = rot[particleId];
	if (rot != 0.0) {
		localVertexPos.xy = rotate2d(localVertexPos.xy, rot);
	}

	// Billboard shenanigans
	mat4 modelView = modelViewMatrix;
	modelView[0][0] = 1.0; //modelMatrix[0][0];
	modelView[0][1] = 0.0;
	modelView[0][2] = 0.0;
	modelView[1][0] = 0.0;
	modelView[1][1] = 1.0; //modelMatrix[1][1];
	modelView[1][2] = 0.0;
	modelView[2][0] = 0.0;
	modelView[2][1] = 0.0;
	modelView[2][2] = 1.0; //modelMatrix[2][2];

#ifdef USE_LOCAL_SPACE

	// Transform particle pos (from local space) to view space
	vec4 posViewSpace = modelViewMatrix * particlePos;
	// Add vertex offsets that are transformed with a matrix with rotation removed
	posViewSpace += modelView * localVertexPos;

	#ifdef USE_SHADOW_MAP
	outData.worldPosition = (modelMatrix * particlePos).xyz; // Approx, ignores billboard verts
	for (int i = 0; i < MAX_SHADOW_MAPS; ++i)
		outData.shadowcoords[i] = shadowMatrices[i] * particlePos; // Approx, ignores billboard verts
	#endif

#else // USE_LOCAL_SPACE

	// Transform particle pos (already in world space) to view space
	vec4 posViewSpace = viewMatrix * particlePos;
	// Add vertex offsets that are transformed with a matrix with rotation removed
	posViewSpace += modelView * localVertexPos;

	#ifdef USE_SHADOW_MAP
	outData.worldPosition = particlePos.xyz; // Approx, ignores billboard verts
	for (int i = 0; i < MAX_SHADOW_MAPS; ++i)
		outData.shadowcoords[i] = shadowMatrices[i] * particlePos; // TODO: This is wrong, pos is in wrong space
	#endif

#endif // USE_LOCAL_SPACE

	outData.normal = vec3(0, 0, 1);
	outData.position = posViewSpace.xyz;
	outData.texcoord = texcoord * material.uvRepeat + material.uvOffset;

	gl_Position = projectionMatrix * posViewSpace;

#ifdef USE_VERTEX_COLOR
	#ifdef USE_VERTEX_COLOR_GRADIENT
	outData.color = mix(vec4(material.ambient, phaseCurve), vec4(material.diffuse, phaseCurve), phase);
	#else
	outData.color = vec4(1.0, 1.0, 1.0, phaseCurve);
	#endif
#endif
}

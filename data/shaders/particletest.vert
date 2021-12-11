
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

void main()
{
	uint particleId = gl_VertexID / 4;
	vec4 particlePos = vec4(pos[particleId], 1.0);
	float particleSize = 0.01f;

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

	// Transform particle pos to view space
	vec4 posViewSpace = modelViewMatrix * particlePos;
	// Add vertex offsets that are transformed with a matrix with rotation removed
	posViewSpace += modelView * vec4(position * particleSize, 1.0);

	outData.normal = vec3(0, 0, 1);
	outData.position = posViewSpace.xyz;
	outData.texcoord = texcoord * material.uvRepeat + material.uvOffset;

	gl_Position = projectionMatrix * posViewSpace;

#ifdef USE_VERTEX_COLOR
	float phase = 1.0 - life[particleId].x / life[particleId].y;
	outData.color = mix(vec4(0.75, 0.0, 0.0, 1.0), vec4(1.0, 0.5, 0.0, 1.0), phase);
#endif
#ifdef USE_SHADOW_MAP
	outData.worldPosition = (modelMatrix * particlePos).xyz; // Approx, ignores billboard verts
	outData.shadowcoord = shadowMatrix * particlePos; // Approx, ignores billboard verts
#endif
}

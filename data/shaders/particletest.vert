
layout(location = ATTR_POSITION) in vec3 position;

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
	float life[];
};

void main()
{
	uint particleId = gl_VertexID / 4;
	vec3 particlePos = pos[particleId];
	float particleSize = 0.05f;

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
	vec4 p = modelViewMatrix * vec4(particlePos, 1.0);
	// Add vertex offsets that are transformed with a matrix with rotation removed
	p += modelView * vec4(position * particleSize, 1.0);

	outData.normal = mat3(normalMatrix) * vec3(0, 0, 1); // TODO
	outData.position = p.xyz;
	outData.texcoord = vec2(0.0, 0.0) * material.uvRepeat + material.uvOffset; // TODO

	gl_Position = projectionMatrix * p;

#ifdef USE_VERTEX_COLOR
	outData.color = mix(vec4(1.0, 0.0, 0.0, 1.0), vec4(0.0, 0.0, 1.0, 1.0), sin(life[particleId]));
#endif
}


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
	uint particleId = gl_VertexID / 3;
	vec3 particlePos = pos[particleId];
	/*vec3 offset = vec3(-0.1, 0.0, 0.0);
	if ((gl_VertexID % 3) == 1)
		offset = vec3(0.0, -0.1, 0.0);
	else if ((gl_VertexID % 2) == 1)
		offset = vec3(0.0, 0.0, -0.1);*/

	gl_Position = modelViewProjMatrix * vec4(particlePos + position, 1.0);
	outData.normal = mat3(normalMatrix) * vec3(0, 0, 1); // TODO
	outData.position = (modelViewMatrix * gl_Position).xyz;
	outData.texcoord = vec2(0.0, 0.0) * material.uvRepeat + material.uvOffset; // TODO

#ifdef USE_VERTEX_COLOR
	outData.color = mix(vec4(1.0, 0.0, 0.0, 1.0), vec4(0.0, 0.0, 1.0, 1.0), sin(life[particleId]));
#endif
}

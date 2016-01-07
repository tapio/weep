layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

#if defined(USE_ALPHA_TEST) || defined(USE_DEPTH_CUBE)
in VertexData {
#ifdef USE_DEPTH_CUBE
	vec3 position;
#endif
#ifdef USE_ALPHA_TEST
	vec2 texcoord;
#endif
} inData[];
#endif

#if defined(USE_ALPHA_TEST) || defined(USE_DEPTH_CUBE)
out VertexData {
#ifdef USE_DEPTH_CUBE
	vec3 position;
#endif
#ifdef USE_ALPHA_TEST
	vec2 texcoord;
#endif
} outData;
#endif

void main()
{
	for (int face = 0; face < 6; ++face) {
		gl_Layer = face;
		for (int i = 0; i < 3; ++i) {
#ifdef USE_DEPTH_CUBE
			outData.position = gl_in[i].gl_Position.xyz;
#endif
#ifdef USE_ALPHA_TEST
			outData.texcoord = inData[i].texcoord;
#endif
			gl_Position = cubeMatrices[face] * gl_in[i].gl_Position;
			EmitVertex();
		}
		EndPrimitive();
	}
}

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

VERTEX_DATA(in, inData[]);
VERTEX_DATA(out, outData);

void main()
{
	for (int face = 0; face < 6; ++face) {
		gl_Layer = face;
		for (int i = 0; i < 3; ++i) {

			outData.position = inData[i].position;
			outData.texcoord = inData[i].texcoord;
			outData.normal = inData[i].normal;
#ifdef USE_VERTEX_COLOR
			outData.color = inData[i].color;
#endif

			gl_Position = cubeMatrices[face] * gl_in[i].gl_Position;
			EmitVertex();
		}
		EndPrimitive();
	}
}

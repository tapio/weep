layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;


void main()
{
	for (int face = 0; face < 6; ++face) {
		gl_Layer = face;
		for (int i = 0; i < 3; ++i) {
			gl_Position = shadowMatrixCube[face] * gl_in[i].gl_Position;
			EmitVertex();
		}
		EndPrimitive();
	}
}

#include "buffers.hpp"
#include "glutil.hpp"

void BufferObjectBase::create(uint type, uint binding, uint size, const void* data)
{
	glGenBuffers(1, &id);
	glBindBuffer(type, id);
	glBindBufferBase(type, binding, id);
	glBufferData(type, size, (const GLvoid*)data, GL_DYNAMIC_DRAW);
}

void BufferObjectBase::upload(uint type, uint size, const void* data)
{
	glBindBuffer(type, id);
	glBufferData(type, size, (const GLvoid*)data, GL_DYNAMIC_DRAW);
}

void BufferObjectBase::destroy()
{
	if (id) {
		glDeleteBuffers(1, &id);
		id = 0;
	}
}

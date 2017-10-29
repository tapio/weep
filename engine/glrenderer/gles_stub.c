#ifdef _WIN32
#include "glad/glad.h"
#else
#include "GL/glcorearb.h"
#endif
#include <stdio.h>

void APIENTRY glPolygonMode (GLenum face, GLenum mode)
{
}

void APIENTRY glTexImage2DMultisample (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
	printf("GLES STUB: glTexImage2DMultisample\n");
}

void APIENTRY glFramebufferTexture (GLenum target, GLenum attachment, GLuint texture, GLint level)
{
	printf("GLES STUB: glFramebufferTexture\n");
	glFramebufferTexture2D(target, attachment, GL_TEXTURE_2D, texture, level);
}

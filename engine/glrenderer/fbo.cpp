#include "fbo.hpp"
#include "glutil.hpp"
#include "engine.hpp"

FBO::FBO()
{

}

FBO::~FBO()
{
	if (fbo)
		destroy();
}

void FBO::create()
{
	ASSERT(width && height);
	uint texType = samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glGenTextures(numTextures, tex);
	for (uint i = 0; i < numTextures; ++i) {
		glBindTexture(texType, tex[i]);
		bool depth = i == depthAttachment;
		uint internalFormat = depth ? GL_DEPTH_COMPONENT : GL_RGB16F;
		if (samples > 1)
			glTexImage2DMultisample(texType, samples, internalFormat, width, height, GL_TRUE);
		else {
			glTexImage2D(texType, 0, internalFormat, width, height, 0, depth ? GL_DEPTH_COMPONENT : GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		uint attach = depth ? GL_DEPTH_ATTACHMENT : (GL_COLOR_ATTACHMENT0 + i);
		glFramebufferTexture(GL_FRAMEBUFFER, attach, tex[i], 0);
	}
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		logError("Framebuffer not complete!");
	if (numTextures >= 3) {
		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);
	} else if (numTextures == 1 && depthAttachment == 0) {
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void FBO::destroy()
{
	glDeleteTextures(numTextures, tex);
	glDeleteFramebuffers(1, &fbo);
	fbo = 0;
	for (uint i = 0; i < MAX_TEXTURES; ++i)
		tex[i] = 0;
}

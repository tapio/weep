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
	uint texType = samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glGenTextures(numTextures, tex);
	for (uint i = 0; i < numTextures; ++i) {
		glBindTexture(texType, tex[i]);
		bool depth = i >= 2;
		uint internalFormat = depth ? GL_DEPTH_COMPONENT24 : GL_RGB16F;
		if (samples > 1)
			glTexImage2DMultisample(texType, samples, internalFormat, Engine::width(), Engine::height(), GL_TRUE);
		else glTexImage2D(texType, 0, internalFormat, Engine::width(), Engine::height(), 0, GL_RGB, GL_FLOAT, NULL);
		uint attach = depth ? GL_DEPTH_ATTACHMENT : (GL_COLOR_ATTACHMENT0 + i);
		glFramebufferTexture2D(GL_FRAMEBUFFER, attach, texType, tex[i], 0);
	}
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		logError("Framebuffer not complete!");
	if (numTextures >= 3) {
		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);
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
}

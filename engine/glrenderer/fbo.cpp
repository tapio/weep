#include "fbo.hpp"
#include "glutil.hpp"
#include "engine.hpp"

FBO::~FBO()
{
	if (fbo)
		destroy();
}

void FBO::create()
{
	logDebug("Create FBO %s", name.c_str());
	ASSERT(width && height);
	ASSERT(!cube || samples <= 1);
	uint texType = GL_TEXTURE_2D;
	if (cube) texType = GL_TEXTURE_CUBE_MAP;
	else if (samples > 1) texType = GL_TEXTURE_2D_MULTISAMPLE;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glGenTextures(numTextures, tex);
	for (uint i = 0; i < numTextures; ++i) {
		glBindTexture(texType, tex[i]);
		bool depth = i == depthAttachment;
		uint internalFormat = depth ? GL_DEPTH_COMPONENT24 : GL_RGB16F;
		uint format = depth ? GL_DEPTH_COMPONENT : GL_RGB;
		uint type = depth ? GL_UNSIGNED_INT : GL_FLOAT;
		if (samples > 1)
			glTexImage2DMultisample(texType, samples, internalFormat, width, height, GL_TRUE);
		else {
			if (cube) {
				for (uint j = 0; j < 6; ++j)
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, internalFormat, width, height, 0,
						format, type, NULL);
			} else {
				glTexImage2D(texType, 0, internalFormat, width, height, 0, format, type, NULL);
			}
			glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			if (cube)
				glTexParameteri(texType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}
		uint attach = depth ? GL_DEPTH_ATTACHMENT : (GL_COLOR_ATTACHMENT0 + i);
		if (cube) {
			glFramebufferTexture(GL_FRAMEBUFFER, attach, tex[i], 0);
			//for (uint j = 0; j < 6; ++j)
			//	glFramebufferTexture2D(GL_FRAMEBUFFER, attach, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, tex[i], 0);
		} else {
			glFramebufferTexture2D(GL_FRAMEBUFFER, attach, texType, tex[i], 0);
		}
	}
	uint fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fbStatus != GL_FRAMEBUFFER_COMPLETE)
		logError("Framebuffer not complete! (%d)", fbStatus);
	if (numTextures >= 3) {
		GLenum attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);
	} else if (numTextures == 1 && depthAttachment == 0) {
		GLenum attachment = GL_NONE;
		glDrawBuffers(1, &attachment);
		glReadBuffer(attachment);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::bind() const
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

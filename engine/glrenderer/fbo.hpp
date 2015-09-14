#pragma once
#include "common.hpp"

struct FBO {
	NONCOPYABLE(FBO);

	FBO();
	~FBO();

	void create();
	void bind();
	void destroy();
	bool valid() const { return fbo > 0; }

	static const uint MAX_TEXTURES = 3;

	uint fbo = 0;
	uint tex[MAX_TEXTURES] = {0, 0, 0};
	uint numTextures = 0;
	uint width = 0;
	uint height = 0;
	uint samples = 0;
	uint depthAttachment = -1;
};

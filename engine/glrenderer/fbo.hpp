#pragma once
#include "common.hpp"

struct FBO {
	NONCOPYABLE(FBO);

	FBO();
	~FBO();

	void create();
	void bind();
	void destroy();

	static const uint MAX_TEXTURES = 3;

	uint fbo;
	uint tex[MAX_TEXTURES] = {0, 0, 0};
	uint numTextures = 0;
	uint samples = 0;
};

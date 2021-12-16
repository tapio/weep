#pragma once
#include "common.hpp"

struct FBO
{
	FBO(const string& debugName = "") : name(debugName) {}
	~FBO();

	NONCOPYABLE(FBO);

	void create();
	void bind() const;
	void destroy();
	bool valid() const { return fbo > 0; }

	static const uint MAX_TEXTURES = 3;

	string name;
	uint fbo = 0;
	uint tex[MAX_TEXTURES] = {0, 0, 0};
	uint numTextures = 1;
	uint width = 0;
	uint height = 0;
	uint samples = 0;
	uint depthAttachment = -1;
	bool cube = false;
};

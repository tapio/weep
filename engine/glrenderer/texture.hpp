#pragma once
#include "common.hpp"
#include "glutil.hpp"

class Texture
{
public:
	NONCOPYABLE(Texture);

	uint id = 0;
	uint wrapS = GL_REPEAT;
	uint wrapT = GL_REPEAT;
	uint minFilter = GL_LINEAR;
	uint magFilter = GL_LINEAR;

	void create();
	void upload(struct Image& image);
	void update();
	void destroy();
};

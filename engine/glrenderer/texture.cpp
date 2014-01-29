#include "texture.hpp"
#include "image.hpp"

namespace {
	static uint formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
}

void Texture::create()
{
	glGenTextures(1, &id);
}


void Texture::upload(Image& image)
{
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, formats[image.channels], image.width, image.height,
		0, formats[image.channels], GL_UNSIGNED_BYTE, &image.data.front());
	update();
}

void Texture::update()
{
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
}

void Texture::destroy()
{
	if (id) {
		glDeleteTextures(1, &id);
		id = 0;
	}
}
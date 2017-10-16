#include "texture.hpp"
#include "image.hpp"

namespace {
	static uint formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
	static uint formats_sRGB[] = { 0, 0, 0, GL_SRGB, GL_SRGB8_ALPHA8 };
}

Texture::~Texture()
{
	destroy();
}

void Texture::create()
{
	glGenTextures(1, &id);
}

void Texture::destroy()
{
	if (id) {
		glDeleteTextures(1, &id);
		id = 0;
	}
}

void Texture::upload(Image& image)
{
	type = GL_TEXTURE_2D;
	glBindTexture(type, id);
	uint internalFormat = image.sRGB ? formats_sRGB[image.channels] : formats[image.channels];
	glTexImage2D(type, 0, internalFormat, image.width, image.height,
		0, formats[image.channels], GL_UNSIGNED_BYTE, &image.data.front());
	update();
	if (minFilter == GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST
		|| minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_NEAREST_MIPMAP_NEAREST)
		glGenerateMipmap(type);
}

void Texture::uploadCube(Image* images[6])
{
	type = GL_TEXTURE_CUBE_MAP;
	glBindTexture(type, id);
	for (int i = 0; i < 6; ++i) {
		Image& image = *images[i];
		uint internalFormat = image.sRGB ? formats_sRGB[image.channels] : formats[image.channels];
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, image.width, image.height,
			0, formats[image.channels], GL_UNSIGNED_BYTE, &image.data.front());
	}
	magFilter = minFilter = GL_LINEAR;
	wrapS = wrapT = wrapR = GL_CLAMP_TO_EDGE;
	update();
}

void Texture::uploadArray(const std::vector<Image*>& images)
{
	type = GL_TEXTURE_2D_ARRAY;
	glBindTexture(type, id);
	const Image& firstImage = *images[0];
	const int mipLevelCount = 1;
	const int width = firstImage.width;
	const int height = firstImage.height;
	const uint internalFormat = firstImage.sRGB ? formats_sRGB[firstImage.channels] : formats[firstImage.channels];
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevelCount, internalFormat, width, height, images.size());
	for (uint i = 0; i < images.size(); ++i) {
		Image& image = *images[i];
		ASSERT(image.width == width);
		ASSERT(image.height == height);
		ASSERT(image.channels == image.channels);
		glTexSubImage3D(type, 0, 0, 0, i, width, height, 1, formats[image.channels], GL_UNSIGNED_BYTE, &image.data.front());
		//glTexImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, image.width, image.height, i,
		//0, formats[image.channels], GL_UNSIGNED_BYTE, &image.data.front());
	}
	if (minFilter == GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST
		|| minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_NEAREST_MIPMAP_NEAREST)
		glGenerateMipmap(type);
	update();
}

void Texture::update()
{
	glBindTexture(type, id);
	glTexParameteri(type, GL_TEXTURE_WRAP_S, wrapS);
	glTexParameteri(type, GL_TEXTURE_WRAP_T, wrapT);
	if (type == GL_TEXTURE_CUBE_MAP)
		glTexParameteri(type, GL_TEXTURE_WRAP_R, wrapR);
	glTexParameteri(type, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(type, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameterf(type, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
}

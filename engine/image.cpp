#include "image.hpp"
#define STBI_NO_HDR
#include <stb_image/stb_image.c>

bool Image::load(const std::string& path, int forceChannels)
{
	unsigned char *pixels = stbi_load(path.c_str(), &width, &height, &channels, forceChannels);
	if (!pixels) {
		logError("Failed to load image %s", path.c_str());
		return false;
	} else if (forceChannels && channels != forceChannels) {
		logWarning("Image %s has %d channels but %d was requested", path.c_str(), channels, forceChannels);
		channels = forceChannels;
	}
	unsigned n = width * height * channels;
	data = std::vector<unsigned char>(pixels, pixels + n);
	stbi_image_free(pixels);
	return true;
}

#include "image.hpp"
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_TGA
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

bool Image::load(const std::string& path, int forceChannels)
{
	START_MEASURE(loadTimeMs)
	unsigned char *pixels = stbi_load(path.c_str(), &width, &height, &channels, forceChannels);
	if (!pixels) {
		logError("Failed to load image %s", path.c_str());
		return false;
	} else if (forceChannels && channels != forceChannels) {
		//logDebug("Image %s has %d channels but %d was requested", path.c_str(), channels, forceChannels);
		channels = forceChannels;
	}
	unsigned n = width * height * channels;
	data = std::vector<unsigned char>(pixels, pixels + n);
	stbi_image_free(pixels);
	END_MEASURE(loadTimeMs)
	logDebug("Loaded image %s %dx%d in %.1fms", path.c_str(), width, height, loadTimeMs);
	return true;
}

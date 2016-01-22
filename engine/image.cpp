#include "image.hpp"
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_TGA
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#include <stb_image/stb_image_write.h>

bool Image::load(const std::string& path_, int forceChannels)
{
	START_MEASURE(loadTimeMs)
	path = path_;
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

bool Image::save(const string& path) const
{
	return stbi_write_png(path.c_str(), width, height, channels, (void*)&data[0], 0);
}

#pragma once
#include "common.hpp"

class Image {
public:
	Image() {}
	Image(const std::string& path, int forceChannels = 0) { load(path, forceChannels); }

	bool load(const std::string& path, int forceChannels = 0);

	int width = 0;
	int height = 0;
	int channels = 0;
	std::vector<unsigned char> data;
};

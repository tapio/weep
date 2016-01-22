#pragma once
#include "common.hpp"

struct Image {
	Image() {}
	Image(int w, int h, int comps = 3): width(w), height(h), channels(comps) {
		data.resize(w * h * channels);
	}
	Image(const std::string& path, int forceChannels = 0) { load(path, forceChannels); }

	bool load(const std::string& path, int forceChannels = 0);
	bool save(const std::string& path) const;

	int width = 0;
	int height = 0;
	int channels = 0;
	bool sRGB = false;
	std::string path;
	std::vector<unsigned char> data;
};

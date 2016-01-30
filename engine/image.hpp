#pragma once
#include "common.hpp"

struct Image {
	Image() {}
	Image(int w, int h, int comps = 3): width(w), height(h), channels(comps) {
		data.resize(w * h * channels);
	}
	Image(const string& path, int forceChannels = 0) { load(path, forceChannels); }

	bool load(const string& path, int forceChannels = 0);
	bool save(const string& path) const;
	void screenshot();

	int width = 0;
	int height = 0;
	int channels = 0;
	bool sRGB = false;
	string path;
	std::vector<unsigned char> data;
};

struct GifMovie {
	GifMovie() {}
	GifMovie(const string& path, int w, int h, int fps = 10, bool dither = false);
	void startRecording();
	void recordFrame(float dt);
	void finish();

	Image frame;
	std::shared_ptr<struct GifWriter> writer = nullptr;
	bool dither = false;
	float frameDelay = 0;
	float currentTime = 0;
	bool recording = false;
};

#include "image.hpp"
#include "glrenderer/glutil.hpp"
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
#include <gif-h/gif.h>

bool Image::load(const std::string& path_, int forceChannels)
{
	START_MEASURE(imageLoadTimeMs)
	path = path_;
	unsigned char *pixels = stbi_load(path.c_str(), &width, &height, &channels, forceChannels);
	if (!pixels) {
		END_CPU_SAMPLE()
		logError("Failed to load image %s", path.c_str());
		return false;
	} else if (forceChannels && channels != forceChannels) {
		//logDebug("Image %s has %d channels but %d was requested", path.c_str(), channels, forceChannels);
		channels = forceChannels;
	}
	unsigned n = width * height * channels;
	data = std::vector<unsigned char>(pixels, pixels + n);
	stbi_image_free(pixels);
	END_MEASURE(imageLoadTimeMs)
	logDebug("Loaded image %s %dx%d in %.1fms", path.c_str(), width, height, imageLoadTimeMs);
	return true;
}

bool Image::save(const string& path) const
{
	return stbi_write_png(path.c_str(), width, height, channels, (void*)&data[0], 0);
}

void Image::screenshot()
{
	BEGIN_CPU_SAMPLE(screenshotTime)
	BEGIN_GPU_SAMPLE(Screenshot)
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	GLenum format = channels == 4 ? GL_RGBA : GL_RGB;
	glReadPixels(0, 0, width, height, format, GL_UNSIGNED_BYTE, &data[0]);
	END_GPU_SAMPLE()
	// Fix orientation
	const int lineSize = width * channels;
	unsigned char* line_tmp = new unsigned char[lineSize];
	unsigned char* line_a = &data[0];
	unsigned char* line_b = &data[0] + (lineSize * (height - 1));
	while (line_a < line_b) {
		memcpy(line_tmp, line_a, lineSize);
		memcpy(line_a, line_b, lineSize);
		memcpy(line_b, line_tmp, lineSize);
		line_a += lineSize;
		line_b -= lineSize;
	}
	delete[] line_tmp;
	END_CPU_SAMPLE()
}



GifMovie::GifMovie(const string& path, int w, int h, int fps, bool dither_)
: frame(w, h, 4), dither(dither_), frameDelay(1.f / fps)
{
	writer.reset(new GifWriter);
	frame.path = path;
}

void GifMovie::startRecording()
{
	GifBegin(writer.get(), frame.path.c_str(), frame.width, frame.height, frameDelay * 100, 8, dither);
	recording = true;
	currentTime = 0;
	frames = 0;
	logInfo("Started capturing gif movie");
}

void GifMovie::recordFrame(float dt)
{
	ASSERT(recording);
	currentTime += dt;
	if (currentTime < frameDelay)
		return;
	while (currentTime >= frameDelay)
		currentTime -= frameDelay;
	frame.screenshot();
	GifWriteFrame(writer.get(), &frame.data[0], frame.width, frame.height, frameDelay * 100, 8, dither);
	frames++;
}

void GifMovie::finish()
{
	ASSERT(recording);
	GifEnd(writer.get());
	recording = false;
	logInfo("Movie capture with %u frames saved to %s", frames, frame.path.c_str());
}

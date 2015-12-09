#pragma once
#include "common.hpp"
#include <thread>

struct Image;
struct Geometry;

class Resources
{
public:
	enum CachePolicy {
		NO_CACHE,
		USE_CACHE
	};

	Resources();
	~Resources();
	void reset();

	void addPath(const string& path);
	void removePath(const string& path);
	string findPath(const string& path) const;
	std::vector<string> listFiles(const string& path, const string& filter = "") const;

	string getText(const string& path, CachePolicy cache);
	std::vector<char>& getBinary(const string& path);
	Image* getImage(const string& path);
	Image* getImageAsync(const string& path);
	Geometry* getGeometry(const string& path);
	Geometry* getHeightmap(const string& path);

	void startAsyncLoading();

private:
	std::vector<string> m_paths;
	std::map<string, string> m_texts;
	std::map<string, std::unique_ptr<std::vector<char>>> m_binaries;
	std::map<string, std::unique_ptr<Image>> m_images;
	std::map<string, std::unique_ptr<Geometry>> m_geoms;

	volatile bool m_loadingActive = false;
	std::vector<Image*> m_loadQueue;
	std::thread m_loadingThread;
};

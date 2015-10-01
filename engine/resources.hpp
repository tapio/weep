#pragma once
#include "common.hpp"

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
	Image* getImage(const string& path);
	Geometry* getGeometry(const string& path);
	Geometry* getHeightmap(const string& path);

private:
	std::vector<string> m_paths;
	std::map<string, string> m_texts;
	std::map<string, std::unique_ptr<Image>> m_images;
	std::map<string, std::unique_ptr<Geometry>> m_geoms;
};

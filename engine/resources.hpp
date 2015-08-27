#pragma once
#include "common.hpp"

struct Image;
struct Geometry;

class Resources
{
public:
	Resources();
	~Resources();
	void reset();

	void addPath(const string& path);
	string findPath(const string& path) const;

	string getText(const string& path) const; // Not cached
	Image* getImage(const string& path);
	Geometry* getGeometry(const string& path);
	Geometry* getHeightmap(const string& path);

private:
	std::vector<string> m_paths;
	std::map<string, std::unique_ptr<Image>> m_images;
	std::map<string, std::unique_ptr<Geometry>> m_geoms;
};

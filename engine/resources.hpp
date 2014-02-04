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

	Image* getImage(const string& path);
	Geometry* getGeometry(const string& path);
	Geometry* getHeightmap(const string& path);

private:
	std::map<string, std::unique_ptr<Image>> m_images;
	std::map<string, std::unique_ptr<Geometry>> m_geoms;
};

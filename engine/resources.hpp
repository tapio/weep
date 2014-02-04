#pragma once
#include "common.hpp"

struct Image;
struct Geometry;

class Resources
{
public:
	Resources();
	~Resources();

	Image* getImage(const string& path);
	Geometry* getGeometry(const string& path);

private:
	std::map<string, std::unique_ptr<Image>> m_images;
	std::map<string, std::unique_ptr<Geometry>> m_geoms;
};

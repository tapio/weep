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
	std::map<string, Image*> m_images;
	std::map<string, Geometry*> m_geoms;
};

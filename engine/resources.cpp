#include "resources.hpp"
#include "image.hpp"
#include "geometry.hpp"

Resources::Resources()
{

}

Resources::~Resources()
{

}

Image* Resources::getImage(const string& path)
{
	auto it = m_images.find(path);
	if (it != m_images.end()) return it->second;
	auto img = new Image("../data/" + path, 4);
	m_images[path] = img;
	return img;
}

Geometry* Resources::getGeometry(const string& path)
{
	auto it = m_geoms.find(path);
	if (it != m_geoms.end()) return it->second;
	auto geom = new Geometry("../data/" + path);
	m_geoms[path] = geom;
	return geom;
}

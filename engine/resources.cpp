#include "resources.hpp"
#include "image.hpp"
#include "geometry.hpp"

Resources::Resources()
{

}

Resources::~Resources()
{

}

void Resources::reset()
{
	m_images.clear();
	m_geoms.clear();
	logDebug("Resource cache dropped");
}

Image* Resources::getImage(const string& path)
{
	auto& ptr = m_images[path];
	if (!ptr) ptr.reset(new Image("../data/" + path, 4));
	return ptr.get();
}

Geometry* Resources::getGeometry(const string& path)
{
	auto& ptr = m_geoms[path];
	if (!ptr) ptr.reset(new Geometry("../data/" + path));
	return ptr.get();
}

Geometry*Resources::getHeightmap(const string& path)
{
	auto& ptr = m_geoms[path];
	if (!ptr) ptr.reset(new Geometry(*getImage(path)));
	return ptr.get();
}

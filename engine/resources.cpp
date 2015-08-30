#include "resources.hpp"
#include "image.hpp"
#include "geometry.hpp"
#include <fstream>
#include <algorithm>

static bool fileExists(const string& path)
{
	std::ifstream f(path);
	return f.good();
}

static string getCanonicalDir(const string& path)
{
	// TODO: Not portable
	return path.back() == '/' ? path : (path + "/");
}


Resources::Resources()
{
}

Resources::~Resources()
{
}

void Resources::reset()
{
	// Paths are not dropped
	m_texts.clear();
	m_images.clear();
	m_geoms.clear();
	logDebug("Resource cache dropped");
}

void Resources::addPath(const string& path)
{
	m_paths.insert(m_paths.begin(), getCanonicalDir(path));
}

void Resources::removePath(const string& path)
{
	m_paths.erase(std::find(m_paths.begin(), m_paths.end(), getCanonicalDir(path)));
}

string Resources::findPath(const string& path) const
{
	for (auto& it : m_paths) {
		string fullPath = it + path;
		if (fileExists(fullPath))
			return fullPath;
	}
	logError("Could not find file \"%s\" from any of the resource paths", path.c_str());
	ASSERT(0);
	return "";
}

string Resources::getText(const string& path, CachePolicy cache)
{
	if (cache == USE_CACHE) {
		auto it = m_texts.find(path);
		if (it != m_texts.end())
			return it->second;
	}
	std::ifstream f(findPath(path));
	std::string text((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	if (cache == USE_CACHE)
		m_texts[path] = text;
	return text;
}

Image* Resources::getImage(const string& path)
{
	auto& ptr = m_images[path];
	if (!ptr) ptr.reset(new Image(findPath(path), 4));
	return ptr.get();
}

Geometry* Resources::getGeometry(const string& path)
{
	auto& ptr = m_geoms[path];
	if (!ptr) ptr.reset(new Geometry(findPath(path)));
	return ptr.get();
}

Geometry*Resources::getHeightmap(const string& path)
{
	auto& ptr = m_geoms[path];
	if (!ptr) ptr.reset(new Geometry(*getImage(findPath(path))));
	return ptr.get();
}

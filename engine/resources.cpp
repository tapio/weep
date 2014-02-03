#include "resources.hpp"
#include "image.hpp"

Image* Resources::getImage(string path)
{
	path = "../data/" + path;
	auto it = m_images.find(path);
	if (it != m_images.end()) return it->second;
	auto img = new Image(path, 4);
	m_images[path] = img;
	return img;
}

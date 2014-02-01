#pragma once
#include "common.hpp"

struct Image;

class Resources
{
public:

	Image* getImage(string path);

private:
	std::map<string, Image*> m_images;
};

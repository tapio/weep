#pragma once
#include "common.hpp"

struct Geometry;

class Renderer
{
public:
	void addGeometry(Geometry* geometry);
	void render();

private:
	friend Renderer& GetRenderer();
	Renderer();
	~Renderer();

	std::vector<Geometry*> geometries;
};


inline Renderer& GetRenderer() {
	static Renderer s_renderer;
	return s_renderer;
}

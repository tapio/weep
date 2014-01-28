#pragma once

class Renderer
{
	friend Renderer& GetRenderer();

	Renderer();
	~Renderer();

public:
	void render();
};


Renderer& GetRenderer() {
	static Renderer s_renderer;
	return s_renderer;
}
#include "engine.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "model.hpp"
#include "camera.hpp"
#include "renderer.hpp"
#include "resources.hpp"

#include <SDL2/SDL.h>


int main(int, char*[])
{
	Engine::init();
	Resources resources;
	Renderer renderer;

	float ar = Engine::width() / (float)Engine::height();
	Camera camera;
	camera.makePerspective(45, ar, 0.1, 1000);
	camera.view = lookAt(vec3(0, 0, 1), vec3(0, 0, 0), vec3(0, 1, 0));

	std::vector<Model> models;

	std::string err;
	Json jsonScene = Json::parse(readFile("testscene.json"), err);
	if (!err.empty())
		panic("Failed to read scene: %s", err.c_str());
	ASSERT(jsonScene.is_array());
	for (uint i = 0; i < jsonScene.array_items().size(); ++i) {
		const Json& def = jsonScene[i];
		ASSERT(def.is_object());
		models.emplace_back(Model());
		Model& model = models.back();
		if (!def["material"].is_null()) {
			const Json& materialDef = def["material"];
			ASSERT(materialDef.is_object());
			model.material.reset(new Material());
			//model.material->ambient = vec3(0.2f, 0.2f, 0.2f);
			//model.material->diffuse = vec3(0.0f, 0.0f, 0.3f);
			if (!materialDef["diffuseMap"].is_null())
				model.material->diffuseMap = resources.getImage(materialDef["diffuseMap"].string_value());
			model.material->shaderName = materialDef["shaderName"].string_value();
		}
		if (!def["geometry"].is_null()) {
			const string& geomPath = def["geometry"].string_value();
			model.geometry = resources.getGeometry(geomPath);
		}
		if (!def["position"].is_null()) {
			const Json& posDef = def["position"];
			ASSERT(posDef.is_array());
			vec3 pos(posDef[0].number_value(), posDef[1].number_value(), posDef[2].number_value());
			model.transform = translate(model.transform, pos);
		}
		renderer.addModel(&model);
	}
	logDebug("Loaded scene with %d models", models.size());

	bool running = true;
	SDL_Event e;
	while (running) {
		renderer.render(camera);
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT || e.key.keysym.sym == SDLK_ESCAPE)
			{
				running = false;
				break;
			}

			if (e.type == SDL_KEYDOWN) {
				vec3 input;
				switch (e.key.keysym.scancode) {
					case SDL_SCANCODE_UP:
					case SDL_SCANCODE_W:
						input.z = -1;
						break;
					case SDL_SCANCODE_DOWN:
					case SDL_SCANCODE_S:
						input.z = 1;
						break;
					case SDL_SCANCODE_LEFT:
					case SDL_SCANCODE_A:
						input.x = -1;
						break;
					case SDL_SCANCODE_RIGHT:
					case SDL_SCANCODE_D:
						input.x = 1;
						break;
					default:
						break;
				}
				vec3 dir = camera.rotation * input;
				camera.position += dir * 0.1f;
			}
			else if (e.type == SDL_MOUSEMOTION) {
				const vec3 axis(0, 1, 0);
				camera.rotation = rotate(camera.rotation, -0.01f * e.motion.xrel, axis);
			}

		}
		Engine::swap();
	}

	renderer.reset();
	Engine::deinit();

	return EXIT_SUCCESS;
}

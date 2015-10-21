#include "common.hpp"
#include "components.hpp"
#include "material.hpp"
#include "engine.hpp"
#include "../game.hpp"

EXPORT void ModuleFunc(uint msg, void* param)
{
	Game& game = *static_cast<Game*>(param);
	switch (msg) {
		case $id(INIT):
		{
			ASSERT($id(Qwerty#¤%) == id::hash("Qwerty#¤%"));
			ASSERT($id(Qwerty#¤%) != 0);
			ASSERT(id::gen("") == id::hash(""));
			break;
		}
		case $id(UPDATE):
		{
			int lightIndex = 0;
			game.entities.for_each<Light>([&](Entity e, Light& light) {
				/**/ if (lightIndex == 0) light.position.x = 5.f * glm::sin(Engine::timems() / 800.f);
				else if (lightIndex == 1) light.position.x = 4.f * glm::sin(Engine::timems() / 500.f);
				else if (lightIndex == 2) light.position.y = 1.f + 1.5f * glm::sin(Engine::timems() / 1000.f);
				if (e.has<Transform>())
					e.get<Transform>().position = light.position;
				if (e.has<Model>())
					e.get<Model>().materials[0]->emissive = light.color * 1.f;
				lightIndex++;
			});
			break;
		}
	}
}

#include "common.hpp"
#include "model.hpp"
#include "light.hpp"
#include "material.hpp"
#include "engine.hpp"

EXPORT void ModuleFunc(uint msg, void* param)
{
	switch (msg) {
		case $id(UPDATE):
		{
			Entities& entities = *static_cast<Entities*>(param);
			int lightIndex = 0;
			entities.for_each<Light>([&](Entity e, Light& light) {
				/**/ if (lightIndex == 0) light.position.x = 5.f * glm::sin(Engine::timems() / 800.f);
				else if (lightIndex == 1) light.position.x = 4.f * glm::sin(Engine::timems() / 500.f);
				else if (lightIndex == 2) light.position.y = 1.f + 1.5f * glm::sin(Engine::timems() / 1000.f);
				if (e.has<Model>()) {
					Model& model = e.get<Model>();
					model.position = light.position;
					model.materials[0]->emissive = light.color * 1.f;
				}
				lightIndex++;
			});
			break;
		}
	}
}

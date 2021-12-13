// Miscellaneous test stuff and a place to write temp code.

#include "common.hpp"
#include "components.hpp"
#include "material.hpp"
#include "engine.hpp"
#include "../game.hpp"

EXPORT void MODULE_FUNC_NAME(uint msg, void* param)
{
	Game& game = *static_cast<Game*>(param);
	switch (msg) {
		case $id(INIT):
		{
			game.engine.moduleInit();
			ASSERT($id(Qwerty#$%) == id::hash("Qwerty#$%"));
			ASSERT($id(Qwerty#$%) != 0);
			ASSERT(id::fnv1a("") == id::hash(""));
			break;
		}
		case $id(UPDATE):
		{
#if EMBED_MODULES || !defined(_WIN32) // Crashes with non-embedded Windows build...
			int lightIndex = 0;
			game.entities.for_each<Light>([&](Entity e, Light& light) {
				float time = Engine::timems() * game.engine.timeMult;
				/**/ if (lightIndex == 0) light.position.x = 5.f * glm::sin(time / 800.f);
				else if (lightIndex == 1) light.position.x = 4.f * glm::sin(time / 500.f);
				else if (lightIndex == 2) light.position.y = 1.f + 1.5f * glm::sin(time / 1000.f);
				if (e.has<Transform>())
					e.get<Transform>().position = light.position;
				if (e.has<Model>())
					e.get<Model>().materials[0].emissive = light.color * 1.f;
				lightIndex++;
			});
#endif
			break;
		}
		case $id(TRIGGER_ENTER):
		{
			logDebug("ENTER TRIGGER VOLUME");
			break;
		}
		case $id(TRIGGER_EXIT):
		{
			logDebug("EXIT TRIGGER VOLUME");
			break;
		}
	}
}

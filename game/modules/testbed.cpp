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

			/*Entity ent = game.entities.get_entity_by_tag("particletest1");
			if (ent.is_alive() && !ent.has<PropertyAnimation>()) {
				PropertyAnimation anim;
				anim.state = AnimationState::PLAYING;
				anim.mode = AnimationMode::LOOP;
				std::vector<PropertyAnimation::Keyframe<quat>> keyframes {
					{ 0.f, quat_identity },
					{ 4.0f, glm::rotate(quat_identity, 2 * 3.14159265f, up_axis) },
				};
				anim.quatTracks.emplace_back($id(rotation), keyframes);
				ent.add(anim);
			}*/

			break;
		}
		case $id(UPDATE):
		{
#if EMBED_MODULES || !defined(_WIN32) // Crashes with non-embedded Windows build...
			int lightIndex = 0;
			game.entities.for_each<Light, Transform>([&](Entity e, Light& light, Transform& trans) {
				float time = Engine::timems() * game.engine.timeMult;
				/**/ if (lightIndex == 0) trans.modify().position.x = 5.f * glm::sin(time / 800.f);
				else if (lightIndex == 1) trans.modify().position.x = 4.f * glm::sin(time / 500.f);
				else if (lightIndex == 2) trans.modify().position.y = 1.f + 1.5f * glm::sin(time / 1000.f);
				if (e.has<Model>())
					e.get<Model>().materials[0].emissive = light.color * 1.f;
				lightIndex++;
			});

			int particleIndex = 0;
			game.entities.for_each<Particles, Transform>([&](Entity e, Particles& particles, Transform& transform) {
				float time = Engine::timems() * game.engine.timeMult;
				/**/ if (particleIndex == 0) {
					//transform.position.y = 1.f + glm::sin(time / 1000.f); transform.dirty = true;
					//transform.setRotation(glm::rotate(transform.rotation, game.engine.dt * 3.14f * 0.5f, up_axis));
				} else if (particleIndex == 1) {
					particles.directionality = 0.5f + 0.25f * (glm::sin(time / 2000.f) + 1.f);
					transform.setRotation(glm::rotate(quat_identity, glm::sin(time / 4000.f), right_axis));
				}
				particleIndex++;
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

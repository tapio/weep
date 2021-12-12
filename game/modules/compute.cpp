// Compute shader testing

#include "common.hpp"
#include "components.hpp"
#include "material.hpp"
#include "engine.hpp"
#include "geometry.hpp"
#include "renderer.hpp"
#include "glrenderer/renderdevice.hpp"
#include "../game.hpp"

#include <glm/gtc/random.hpp>

static SSBO<vec3> posBuffer(BINDING_SSBO_POSITION);
static SSBO<vec3> velBuffer(BINDING_SSBO_VELOCITY);
static SSBO<vec2> lifeBuffer(BINDING_SSBO_LIFE);

EXPORT void MODULE_FUNC_NAME(uint msg, void* param)
{
	Game& game = *static_cast<Game*>(param);
	switch (msg) {
		case $id(INIT):
		{
			game.engine.moduleInit();

			Entity particleSystem = game.entities.get_entity_by_tag("particletest");
			if (particleSystem.is_alive() && particleSystem.has<Particles>()) {
				// TODO: Move buffers to rendersystem
				uint numParticles = particleSystem.get<Particles>().count;
				posBuffer.create(numParticles);
				velBuffer.create(numParticles);
				lifeBuffer.create(numParticles);
			}

			break;
		}
		case $id(UPDATE):
		{
			break;
		}
	}
}

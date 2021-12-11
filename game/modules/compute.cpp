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

static uint NumParticles = 0;

EXPORT void MODULE_FUNC_NAME(uint msg, void* param)
{
	Game& game = *static_cast<Game*>(param);
	switch (msg) {
		case $id(INIT):
		{
			game.engine.moduleInit();

			Entity particleSystem = game.entities.get_entity_by_tag("particletest");
			if (particleSystem.has<Model>()) {
				// TODO: Woah, this could be simpler...
				NumParticles = particleSystem.get<Model>().lods[0].geometry->batches[0].numVertices / 4;
			}

			posBuffer.create(NumParticles);
			velBuffer.create(NumParticles);
			lifeBuffer.create(NumParticles);

			break;
		}
		case $id(UPDATE):
		{
			RenderSystem& renderer = game.entities.get_system<RenderSystem>();
			const ShaderProgram& compShader = renderer.device().getProgram($id(particles_simulate));
			BEGIN_GPU_SAMPLE(ParticleCompute)
			compShader.use();
			compShader.compute(NumParticles / PARTICLE_GROUP_SIZE);
			END_GPU_SAMPLE()

			break;
		}
	}
}

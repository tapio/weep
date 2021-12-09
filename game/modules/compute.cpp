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

constexpr uint NumParticles = 1024 * 64;
static SSBO<vec3> posBuffer(BINDING_SSBO_POSITION, NumParticles);
static SSBO<vec3> velBuffer(BINDING_SSBO_VELOCITY, NumParticles);
static SSBO<float> lifeBuffer(BINDING_SSBO_LIFE, NumParticles);
static Geometry particleGeo;

EXPORT void MODULE_FUNC_NAME(uint msg, void* param)
{
	Game& game = *static_cast<Game*>(param);
	switch (msg) {
		case $id(INIT):
		{
			game.engine.moduleInit();

			for (vec3& pos : posBuffer.buffer)
				pos = glm::linearRand(vec3(-1,-1,-1), vec3(1,1,1)) * 0.5f;
			posBuffer.create();

			for (vec3& vel : velBuffer.buffer)
				vel = glm::normalize(glm::linearRand(vec3(-1,-1,-1), vec3(1,1,1))) * 0.1f;
			velBuffer.create();

			for (float& life : lifeBuffer.buffer)
				life = glm::linearRand(0.0f, 5.0f);
			lifeBuffer.create();

			particleGeo.batches.push_back({});
			Batch& batch = particleGeo.batches.back();
			batch.name = "ParticleBatch";
			batch.positions.resize(NumParticles * 3);
			for (vec3& pos : batch.positions)
				pos = glm::linearRand(vec3(-1,-1,-1), vec3(1,1,1)) * 0.1f;
			batch.setupAttributes();
			particleGeo.calculateBoundingBox();

			Entity particleSystem = game.entities.get_entity_by_tag("particletest");
			if (particleSystem.has<Model>()) {
				particleSystem.get<Model>().lods[0].geometry = &particleGeo;
				particleSystem.get<Model>().bounds = particleGeo.bounds;
			}

			break;
		}
		case $id(UPDATE):
		{
			RenderSystem& renderer = game.entities.get_system<RenderSystem>();
			const ShaderProgram& compShader = renderer.device().getProgram($id(particles_simulate));
			compShader.use();
			compShader.compute(NumParticles);

			break;
		}
	}
}

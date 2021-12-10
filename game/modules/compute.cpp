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

constexpr uint NumParticles = 1024 * 1024;
static SSBO<vec3> posBuffer(BINDING_SSBO_POSITION, NumParticles);
static SSBO<vec3> velBuffer(BINDING_SSBO_VELOCITY, NumParticles);
static SSBO<vec2> lifeBuffer(BINDING_SSBO_LIFE, NumParticles);
static Geometry particleGeo;

EXPORT void MODULE_FUNC_NAME(uint msg, void* param)
{
	Game& game = *static_cast<Game*>(param);
	switch (msg) {
		case $id(INIT):
		{
			game.engine.moduleInit();

			posBuffer.create();
			velBuffer.create();
			lifeBuffer.create();

			particleGeo.batches.push_back({});
			Batch& batch = particleGeo.batches.back();
			batch.name = "ParticleBatch";
			batch.positions.resize(NumParticles * 4);
			// Particle size applied in shader
			for (uint i = 0; i < batch.positions.size(); i += 4) {
				batch.positions[i+0] = {-0.5f, -0.5f, 0.f};
				batch.positions[i+1] = {-0.5f,  0.5f, 0.f};
				batch.positions[i+2] = { 0.5f,  0.5f, 0.f};
				batch.positions[i+3] = { 0.5f, -0.5f, 0.f};
			}
			batch.indices.resize(NumParticles * 6);
			for (uint i = 0; i < batch.indices.size(); i += 6) {
				uint base = i / 6 * 4;
				batch.indices[i+0] = base + 0;
				batch.indices[i+1] = base + 2;
				batch.indices[i+2] = base + 1;
				batch.indices[i+3] = base + 0;
				batch.indices[i+4] = base + 3;
				batch.indices[i+5] = base + 2;
			}

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
			BEGIN_GPU_SAMPLE(ParticleCompute)
			compShader.use();
			compShader.compute(NumParticles / PARTICLE_GROUP_SIZE);
			END_GPU_SAMPLE()

			break;
		}
	}
}

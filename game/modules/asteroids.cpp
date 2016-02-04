// Simple top-down asteroids clone to test creating a sprite based game with the engine.
// Also demonstrates using game-specific ad-hoc components (custom physics in this case),
// as well as creating entities without the help of SceneLoader and a json definition.

#include "common.hpp"
#include "gui.hpp"
#include "components.hpp"
#include "physics.hpp"
#include "audio.hpp"
#include "renderer.hpp"
#include "geometry.hpp"
#include "image.hpp"
#include "tween.hpp"
#include "../game.hpp"
#include "../controller.hpp"
#include "SDL_events.h"
#include <glm/gtc/random.hpp>
#include <glm/gtx/component_wise.hpp>


struct Physics {
	vec2 pos = {0.f, 0.f};
	vec2 vel = {0.f, 0.f};
	float angle = 0.f;
	float angVel = 0.f;
};

struct Asteroid {
	float hp = 5.f;
};
struct Laser {
	float range = 10.f;
};


static const float turnSpeed = 4.f;
static const float maxSpeed = 6.f;
static const float accel = 5.f;
static const float fireDelay = 0.075f;
static const float laserSpeed = 10.f;
static const vec2 areaExtents = vec2(10, 7);
static const char* s_asteroids[] = {
	"sprites/space-shooter/Meteors/meteorBrown_big1.png",
	"sprites/space-shooter/Meteors/meteorBrown_big2.png",
	"sprites/space-shooter/Meteors/meteorBrown_big3.png",
	"sprites/space-shooter/Meteors/meteorBrown_big4.png"
};

static Game* s_game = nullptr;
static int s_score = 0;
static float s_hp = 100.f;
static bool s_gameOver = true;
static float s_resetTime = 0;
static float s_fireTime = 0;
static Tween s_damageAnim = Tween(0.25f, false);
static Tween s_startAnim = Tween(0.35f, false);

void spawnAsteroid() {
	Entity asteroid = s_game->entities.create();
	asteroid.add<Asteroid>();
	Transform& trans = asteroid.add<Transform>();
	Physics& phys = asteroid.add<Physics>();
	phys.pos.x = glm::linearRand(-areaExtents.x, areaExtents.x);
	phys.pos.y = glm::linearRand(-areaExtents.y, areaExtents.y);
	phys.angle = glm::linearRand(-M_PI, M_PI);
	phys.angVel = glm::linearRand(-2.f, 2.f);
	phys.vel.x = glm::linearRand(-1.f, 1.f);
	phys.vel.y = glm::linearRand(-1.f, 1.f);
	phys.vel = glm::normalize(phys.vel) * glm::linearRand(0.5f, 2.f);
	Model& model = asteroid.add<Model>();
	model.lods[0].geometry = model.geometry = s_game->resources.getGeometry("debug/plane.obj");
	model.bounds.min = model.lods[0].geometry->bounds.min * trans.scale;
	model.bounds.max = model.lods[0].geometry->bounds.max * trans.scale;
	model.bounds.radius = model.lods[0].geometry->bounds.radius * glm::compMax(trans.scale);
	Material material;
	material.flags |= Material::ALPHA_TEST;
	material.ambient = vec3(1);
	const char* asteroidImg = s_asteroids[glm::linearRand<int>(0, countof(s_asteroids)-1)];
	material.map[Material::DIFFUSE_MAP] = s_game->resources.getImage(asteroidImg);
	material.map[Material::DIFFUSE_MAP]->sRGB = true;
	model.materials.emplace_back(material);
}

void spawnLaser(vec2 pos, float angle, float speed) {
	Entity laser = s_game->entities.create();
	laser.add<Laser>();
	Transform& trans = laser.add<Transform>();
	trans.scale = vec3(0.25f);
	Physics& phys = laser.add<Physics>();
	phys.pos = pos;
	phys.angle = angle;
	phys.vel.x = glm::cos(angle) * speed;
	phys.vel.y = -glm::sin(angle) * speed;
	Model& model = laser.add<Model>();
	model.lods[0].geometry = model.geometry = s_game->resources.getGeometry("debug/plane.obj");
	model.bounds.min = model.lods[0].geometry->bounds.min * trans.scale;
	model.bounds.max = model.lods[0].geometry->bounds.max * trans.scale;
	model.bounds.radius = model.lods[0].geometry->bounds.radius * glm::compMax(trans.scale);
	Material material;
	material.flags |= Material::ALPHA_TEST;
	material.ambient = vec3(0.01f);
	material.emissive = vec3(0.75f);
	material.map[Material::DIFFUSE_MAP] = s_game->resources.getImage("sprites/space-shooter/Lasers/laserBlue01.png");
	material.map[Material::DIFFUSE_MAP]->sRGB = true;
	material.map[Material::EMISSION_MAP] = s_game->resources.getImage("sprites/space-shooter/Lasers/laserBlue01.png");
	material.map[Material::EMISSION_MAP]->sRGB = true;
	model.materials.emplace_back(material);
}

void begin() {
	Entity cameraEnt = s_game->entities.get_entity_by_tag("camera");
	Controller& controller = cameraEnt.get<Controller>();
	controller.enabled = false;

	Entity pl = s_game->entities.get_entity_by_tag("player");
	if (pl.is_alive()) {
		Transform& trans = pl.get<Transform>();
		trans.setPosition(vec3(0));
		Physics& phys = pl.get<Physics>();
		phys.angle = 0;
		phys.vel = vec2(0, 0);
	}

	for (int i = 0; i < 10; ++i)
		spawnAsteroid();
}

void reset() {
	std::srand(std::time(0));
	s_score = 0;
	s_hp = 100.f;
	s_gameOver = false;
	s_resetTime = 5.f;
	s_fireTime = 0.f;
	s_startAnim.reset();
	s_game->entities.get_system<RenderSystem>().env().saturation = 0.f;
	Entity pl = s_game->entities.get_entity_by_tag("player");
	if (pl.is_alive() && !pl.has<Physics>())
		pl.add<Physics>();
	begin();
}

void thrust(float dir, float dt) {
	Entity pl = s_game->entities.get_entity_by_tag("player");
	Physics& phys = pl.get<Physics>();
	float thrust = dir * accel * dt;
	phys.vel.x += glm::cos(phys.angle) * thrust;
	phys.vel.y -= glm::sin(phys.angle) * thrust;
	if (length(phys.vel) > maxSpeed)
		phys.vel = normalize(phys.vel) * maxSpeed;
}

void steer(float dir, float dt) {
	Entity pl = s_game->entities.get_entity_by_tag("player");
	Physics& phys = pl.get<Physics>();
	phys.angle += turnSpeed * dir * dt;
}

void fire(float dt) {
	s_fireTime -= dt;
	if (s_fireTime <= 0) {
		s_fireTime = fireDelay;
		Entity pl = s_game->entities.get_entity_by_tag("player");
		Physics& phys = pl.get<Physics>();
		spawnLaser(phys.pos, phys.angle + glm::linearRand(-0.01f, 0.01f),
			glm::length(phys.vel) + laserSpeed);
		AudioSystem& audio = s_game->entities.get_system<AudioSystem>();
		audio.play($id(laser));
	}
}

bool hitTest(vec2 posA, vec2 posB, float rA, float rB) {
	float d2 = glm::distance2(posA, posB);
	float r2 = (rA + rB) * 0.75f; r2 *= r2;
	return d2 <= r2;
}

void simulate(float dt) {
	s_game->entities.for_each<Physics, Transform>([&](Entity e, Physics& phys, Transform& trans) {
		phys.pos += phys.vel * dt;
		phys.angle += phys.angVel * dt;
		if (e.has<Laser>()) {
			Laser& laser = e.get<Laser>();
			laser.range -= glm::length(phys.vel * dt);
			if (laser.range <= 0)
				e.kill();
		}
		// Wrapping
		if (phys.pos.x < -areaExtents.x)
			phys.pos.x += 2 * areaExtents.x;
		else if (phys.pos.x > areaExtents.x)
			phys.pos.x -= 2 * areaExtents.x;
		if (phys.pos.y < -areaExtents.y)
			phys.pos.y += 2 * areaExtents.y;
		else if (phys.pos.y > areaExtents.y)
			phys.pos.y -= 2 * areaExtents.y;
		// Apply to visual
		trans.position.x = phys.pos.x;
		trans.position.z = phys.pos.y;
		trans.rotation = glm::angleAxis(phys.angle - 1.57079632679f, vec3(0, 1, 0));
		trans.dirty = true;
	});

	if (s_gameOver)
		return;

	AudioSystem& audio = s_game->entities.get_system<AudioSystem>();
	Entity pl = s_game->entities.get_entity_by_tag("player");
	Physics& pl_phys = pl.get<Physics>();
	Model& pl_model = pl.get<Model>();
	s_game->entities.for_each<Asteroid, Physics, Model>([&](Entity asteroidEntity, Asteroid& asteroid, Physics& asteroid_phys, Model& asteroid_model) {
		if (hitTest(pl_phys.pos, asteroid_phys.pos, pl_model.bounds.radius, asteroid_model.bounds.radius)) {
			s_hp -= glm::linearRand(15.f, 25.f);
			asteroidEntity.kill();
			s_damageAnim.reset();
			audio.play($id(hit));
			if (s_hp <= 0)
				s_gameOver = true;
		}
		s_game->entities.for_each<Laser, Physics, Model>([&](Entity laserEntity, Laser&, Physics& laser_phys, Model& laser_model) {
			if (hitTest(asteroid_phys.pos, laser_phys.pos, asteroid_model.bounds.radius, laser_model.bounds.radius)) {
				asteroid.hp -= 1.f;
				s_score++;
				laserEntity.kill();
				if (asteroid.hp <= 0.f) {
					asteroidEntity.kill();
					s_score += 10;
					spawnAsteroid();
					spawnAsteroid();
				}
				audio.play($id(hit));
			}
		});
	});

}

EXPORT void ModuleFunc(uint msg, void* param)
{
	switch (msg) {
		case $id(INIT):
		{
			s_game = static_cast<Game*>(param);
			s_game->engine.moduleInit();
			ImGuiSystem& imgui = s_game->entities.get_system<ImGuiSystem>();
			imgui.applyInternalState();

			reset();
			break;
		}
		case $id(INPUT):
		{
			SDL_Event& e = *static_cast<SDL_Event*>(param);
			if (e.type == SDL_KEYUP)
			{
				SDL_Keysym keysym = e.key.keysym;
				if (keysym.sym == SDLK_r) {
					reset();
				}
			}
			break;
		}
		case $id(UPDATE):
		{
			s_game = static_cast<Game*>(param);

			// Input
			if (!s_gameOver) {
				const uint8* keys = SDL_GetKeyboardState(NULL);
				if (keys[SDL_SCANCODE_UP])
					thrust(1, s_game->engine.dt);
				if (keys[SDL_SCANCODE_DOWN])
					thrust(-1, s_game->engine.dt);
				if (keys[SDL_SCANCODE_LEFT])
					steer(1, s_game->engine.dt);
				if (keys[SDL_SCANCODE_RIGHT])
					steer(-1, s_game->engine.dt);
				if (keys[SDL_SCANCODE_SPACE])
					fire(s_game->engine.dt);
			}

			simulate(s_game->engine.dt);

			if (s_damageAnim.active()) {
				s_damageAnim.update(s_game->engine.dt);
				Environment& env = s_game->entities.get_system<RenderSystem>().env();
				env.scanlines = s_damageAnim.active()
					? (1.0f - easing::backInOut(s_damageAnim.t)) : 0.f;
			}

			if (s_startAnim.active()) {
				s_startAnim.update(s_game->engine.dt);
				Environment& env = s_game->entities.get_system<RenderSystem>().env();
				env.vignette = s_startAnim.active()
					? vec3(s_startAnim.t, 0.5f, 1.f) : vec3(0);
			}

			ScopedFont sf(s_game->entities, $id(asteroids_hud));
			ImGui::SetNextWindowPos(ImVec2(20, 20));
			ImGui::Begin("##Points", NULL, ImGuiSystem::MinimalWindow);
			ImGui::Text("Shields: %d", glm::max(0, (int)s_hp));
			ImGui::Text("Score: %d", s_score);
			ImGui::End();

			if (s_gameOver) {
				s_game->entities.get_system<RenderSystem>().env().saturation = -1.f;
				ScopedFont sf(s_game->entities, $id(asteroids_big));
				ImGui::SetNextWindowPosCenter();
				ImGui::Begin("##GameOver", NULL, ImGuiSystem::MinimalWindow);
				ImGui::Text("Game Over!");
				ImGui::Text("Score: %d", s_score);
				ImGui::End();

				s_resetTime -= s_game->engine.dt;
				if (s_resetTime <= 0.f)
					reset();
			}
			break;
		}
	}
}

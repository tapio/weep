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
	float angle = 0.f;
	vec2 vel = {0.f, 0.f};
};

static const float turnSpeed = 4.f;
static const float maxSpeed = 6.f;
static const float accel = 5.f;
static const vec2 areaExtents = vec2(10, 7);
static const char* s_asteroids[] = {
	"sprites/space-shooter/Meteors/meteorBrown_big1.png",
	"sprites/space-shooter/Meteors/meteorBrown_big2.png",
	"sprites/space-shooter/Meteors/meteorBrown_big3.png",
	"sprites/space-shooter/Meteors/meteorBrown_big4.png"
};

static Game* s_game = nullptr;
static float s_scoreWindowWidth = 150.f;
static int s_score = 0;
static bool s_gameOver = true;
static float s_resetTime = 0;
static Tween s_dieAnim = Tween(0.25f, false);
static Tween s_startAnim = Tween(0.35f, false);

void spawnAsteroid() {
	Entity asteroid = s_game->entities.create();
	Transform& trans = asteroid.add<Transform>();
	trans.position.x = glm::linearRand(-areaExtents.x, areaExtents.x);
	trans.position.z = glm::linearRand(-areaExtents.y, areaExtents.y);
	Physics& phys = asteroid.add<Physics>();
	phys.angle = glm::linearRand(-M_PI, M_PI);
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
	s_gameOver = false;
	s_resetTime = 5.f;
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

void simulate(float dt) {
	s_game->entities.for_each<Physics, Transform>([&](Entity, Physics& phys, Transform& trans){
		trans.position.x += phys.vel.x * dt;
		trans.position.z += phys.vel.y * dt;
		trans.rotation = glm::angleAxis(phys.angle - 1.57079632679f, vec3(0, 1, 0));
		trans.dirty = true;
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
			//AudioSystem& audio = s_game->entities.get_system<AudioSystem>();

			// Input
			const uint8* keys = SDL_GetKeyboardState(NULL);
			if (keys[SDL_SCANCODE_UP])
				thrust(1, s_game->engine.dt);
			if (keys[SDL_SCANCODE_DOWN])
				thrust(-1, s_game->engine.dt);
			if (keys[SDL_SCANCODE_LEFT])
				steer(1, s_game->engine.dt);
			if (keys[SDL_SCANCODE_RIGHT])
				steer(-1, s_game->engine.dt);

			simulate(s_game->engine.dt);

			if (s_dieAnim.active()) {
				s_dieAnim.update(s_game->engine.dt);
				Environment& env = s_game->entities.get_system<RenderSystem>().env();
				env.chromaticAberration = s_dieAnim.active()
					? 0.1f * (1.0f - easing::backInOut(s_dieAnim.t)) : 0.f;
			}

			if (s_startAnim.active()) {
				s_startAnim.update(s_game->engine.dt);
				Environment& env = s_game->entities.get_system<RenderSystem>().env();
				env.vignette = s_startAnim.active()
					? vec3(s_startAnim.t, 0.5f, 1.f) : vec3(0);
			}

			ScopedFont sf(s_game->entities, $id(asteroids_big));
			float x = (Engine::width() - s_scoreWindowWidth) * 0.5f;
			ImGui::SetNextWindowPos(ImVec2(x, 20));
			ImGui::Begin("##Points", NULL, ImGuiSystem::MinimalWindow);
			s_scoreWindowWidth = ImGui::GetWindowWidth();
			ImGui::Text("Score: %d", s_score);
			ImGui::End();

			if (s_gameOver) {
				s_game->entities.get_system<RenderSystem>().env().saturation = -1.f;
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

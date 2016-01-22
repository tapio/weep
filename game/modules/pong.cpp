#include "common.hpp"
#include "gui.hpp"
#include "components.hpp"
#include "physics.hpp"
#include "audio.hpp"
#include "renderer.hpp"
#include "tween.hpp"
#include "../game.hpp"
#include "../controller.hpp"
#include "SDL_events.h"
#include <glm/gtc/random.hpp>

static Game* s_game = nullptr;
static float s_pointsWindowWidth = 150.f;
static int s_points[2] = {};
static const char* s_names[] = { "Player 1", "Player 2" };
static const int pointsToWin = 3;
static int s_winner = -1;
static float s_resetTime = 0;
static Tween s_dieAnim = Tween(0.25f, false);
static Tween s_startAnim = Tween(0.35f, false);

void begin(int dir) {
	Entity cameraEnt = s_game->entities.get_entity_by_tag("camera");
	Controller& controller = cameraEnt.get<Controller>();
	controller.enabled = false;

	Entity ball = s_game->entities.get_entity_by_tag("ball");
	if (ball.is_alive()) {
		Transform& trans = ball.get<Transform>();
		trans.setPosition(vec3(0));
		btRigidBody& body = ball.get<btRigidBody>();
		body.setLinearVelocity(btVector3(10 * dir, 0, glm::linearRand(-6, 6)));
	}
	Entity paddle1 = s_game->entities.get_entity_by_tag("paddle1");
	if (paddle1.is_alive()) {
		Transform& trans = paddle1.get<Transform>();
		trans.setPosition(vec3(-10, 0, 0));
		btRigidBody& body = paddle1.get<btRigidBody>();
		body.setLinearVelocity(btVector3(0, 0, 0));
	}
	Entity paddle2 = s_game->entities.get_entity_by_tag("paddle2");
	if (paddle2.is_alive()) {
		Transform& trans = paddle2.get<Transform>();
		trans.setPosition(vec3(10, 0, 0));
		btRigidBody& body = paddle2.get<btRigidBody>();
		body.setLinearVelocity(btVector3(0, 0, 0));
	}
}

void reset() {
	s_points[0] = 0;
	s_points[1] = 0;
	s_winner = -1;
	s_resetTime = 5.f;
	s_startAnim.reset();
	s_game->entities.get_system<RenderSystem>().env().saturation = 0.f;
	begin(glm::linearRand(0, 1) ? 1 : -1);
}

void steer(const string& paddleName, int dir) {
	const float paddleVel = 12.f;
	Entity paddle = s_game->entities.get_entity_by_tag(paddleName);
	if (paddle.is_alive()) {
		btRigidBody& body = paddle.get<btRigidBody>();
		body.setLinearVelocity(btVector3(0, 0, dir * paddleVel));
	}
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
			if (e.type == SDL_KEYDOWN)
			{
				SDL_Keysym keysym = e.key.keysym;
				if (keysym.sym == SDLK_w)
					steer("paddle1", -1);
				if (keysym.sym == SDLK_s)
					steer("paddle1", 1);
				if (keysym.sym == SDLK_UP)
					steer("paddle2", -1);
				if (keysym.sym == SDLK_DOWN)
					steer("paddle2", 1);
			}
			else if (e.type == SDL_KEYUP)
			{
				SDL_Keysym keysym = e.key.keysym;
				if (keysym.sym == SDLK_r) {
					reset();
				}
				if (keysym.sym == SDLK_w)
					steer("paddle1", 0);
				if (keysym.sym == SDLK_s)
					steer("paddle1", 0);
				if (keysym.sym == SDLK_UP)
					steer("paddle2", 0);
				if (keysym.sym == SDLK_DOWN)
					steer("paddle2", 0);
			}
			break;
		}
		case $id(UPDATE):
		{
			s_game = static_cast<Game*>(param);
			AudioSystem& audio = s_game->entities.get_system<AudioSystem>();

			Entity ball = s_game->entities.get_entity_by_tag("ball");
			if (ball.is_alive() && s_winner < 0) {
				Transform& trans = ball.get<Transform>();
				if (trans.position.x < -11) {
					s_points[1]++;
					if (s_points[1] >= pointsToWin) {
						audio.play($id(win));
						s_winner = 1;
					} else {
						audio.play($id(point));
						s_dieAnim.reset();
						begin(1);
					}
				} else if (trans.position.x > 11) {
					s_points[0]++;
					if (s_points[0] >= pointsToWin) {
						audio.play($id(win));
						s_winner = 0;
					} else {
						audio.play($id(point));
						s_dieAnim.reset();
						begin(-1);
					}
				} else if (glm::dot(trans.position, trans.position) > 100 * 100) {
					begin(glm::linearRand(0, 1) ? 1 : -1);
				}
			}

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

			ScopedFont sf(s_game->entities, $id(pong_big));
			float x = (Engine::width() - s_pointsWindowWidth) * 0.5f;
			ImGui::SetNextWindowPos(ImVec2(x, 20));
			ImGui::Begin("##Points", NULL, ImGuiSystem::MinimalWindow);
			s_pointsWindowWidth = ImGui::GetWindowWidth();
			ImGui::Text("%d  -  %d", s_points[0], s_points[1]);
			ImGui::End();

			if (s_winner >= 0) {
				s_game->entities.get_system<RenderSystem>().env().saturation = -1.f;
				ImGui::SetNextWindowPosCenter();
				ImGui::Begin("##Winner", NULL, ImGuiSystem::MinimalWindow);
				ImGui::Text("%s Won!", s_names[s_winner]);
				ImGui::End();

				s_resetTime -= s_game->engine.dt;
				if (s_resetTime <= 0.f)
					reset();
			}
			break;
		}
	}
}

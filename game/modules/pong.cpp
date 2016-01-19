#include "common.hpp"
#include "gui.hpp"
#include "components.hpp"
#include "physics.hpp"
#include "glrenderer/texture.hpp"
#include "../game.hpp"
#include "SDL_events.h"

static Game* s_game = nullptr;

void begin() {
	Entity ball = s_game->entities.get_entity_by_tag("ball");
	if (ball.is_alive()) {
		Transform& trans = ball.get<Transform>();
		trans.setPosition(vec3(0));
		btRigidBody& body = ball.get<btRigidBody>();
		body.setLinearVelocity(btVector3(-8, 0, -3));
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

			begin();
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
					begin();
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
			break;
		}
	}
}

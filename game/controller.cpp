#include "controller.hpp"
#include "physics.hpp"
#include <SDL2/SDL.h>


Controller::Controller(vec3 pos, quat rot)
: position(pos)
, rotation(rot)
{}

void Controller::update(float dt)
{
	const unsigned char* keys = SDL_GetKeyboardState(NULL);
	vec3 input(0);
	float jump = 0;

	if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W])
		input.z = -1;
	else if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S])
		input.z = 1;

	if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A])
		input.x = -1;
	else if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D])
		input.x = 1;

	if (keys[SDL_SCANCODE_PAGEUP] || keys[SDL_SCANCODE_Q])
		input.y = 1;
	else if (keys[SDL_SCANCODE_PAGEDOWN] || keys[SDL_SCANCODE_Z])
		input.y = -1;

	if (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT])
		input *= fast;

	if (jumpDelay > 0)
		jumpDelay -= dt;

	if (keys[SDL_SCANCODE_SPACE] && onGround && jumpDelay <= 0) {
		jump = 1.f;
		jumpDelay = 0.250f;
	}

	rotation = quat();
	rotation = glm::rotate(rotation, glm::radians(angles.y), yaxis);
	rotation = glm::rotate(rotation, glm::radians(angles.x), xaxis);

	if (dot(input, input) > 0.001 || jump) {
		vec3 dir = rotation * input;
		if (!fly) dir.y = 0;
		if (body) {
			vec3 force = dir * speed * moveForce * dt;
			force.y += jump * jumpForce;
			body->applyCentralImpulse(convert(force));
		} else {
			position += dir * speed * dt;
		}
	}
	// Brake force to limit speed
	if (body) {
		btVector3 vel = body->getLinearVelocity();
		vel *= -brakeForce * dt;
		if (!fly) vel.setY(0);
		body->applyCentralImpulse(vel);
	}
}

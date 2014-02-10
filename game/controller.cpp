#include "controller.hpp"
#include <SDL2/SDL.h>


Controller::Controller(vec3& pos, quat& rot)
: position(pos)
, rotation(rot)
{}

void Controller::update(float dt)
{
	const unsigned char* keys = SDL_GetKeyboardState(NULL);
	vec3 input;

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

	vec3 dir = rotation * input;
	position += dir * dt * speed;
}

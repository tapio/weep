#pragma once
#include "common.hpp"

class AnimationSystem : public System
{
public:
	AnimationSystem();
	~AnimationSystem();
	void reset();
	void update(Entities& entities, float dt);

	void play(Entity& entity);
	void stop(Entity& entity);
};

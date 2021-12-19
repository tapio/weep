#pragma once
#include "common.hpp"
#define ECS_ASSERT ASSERT
#include <ecs/ecs.hpp>

class AnimationSystem : public ecs::System
{
public:
	AnimationSystem();
	~AnimationSystem();
	void reset();
	void update(ecs::Entities& entities, float dt);

	void play(ecs::Entity entity);
	void pause(ecs::Entity entity);
	void stop(ecs::Entity entity);
};

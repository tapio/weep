#pragma once
#include "common.hpp"
#define ECS_ASSERT ASSERT
#include <ecs/ecs.hpp>

class TriggerSystem : public ecs::System
{
public:
	TriggerSystem();
	~TriggerSystem();

	void update(ecs::Entities& entities, float dt);
};

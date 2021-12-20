#pragma once
#include "common.hpp"
#include <ecs/ecs.hpp>

class TriggerSystem : public ecs::System
{
public:
	TriggerSystem();
	~TriggerSystem();

	void update(ecs::Entities& entities, float dt);
};

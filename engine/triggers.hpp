#pragma once
#include "common.hpp"

class TriggerSystem : public System
{
public:
	TriggerSystem();
	~TriggerSystem();

	void update(Entities& entities, float dt);
};

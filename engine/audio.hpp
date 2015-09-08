#pragma once
#include "common.hpp"
#include "soloud.h"

class AudioSystem
{
public:
	AudioSystem();
	~AudioSystem();
	void reset();
	void update();

	std::unique_ptr<SoLoud::Soloud> soloud;
};

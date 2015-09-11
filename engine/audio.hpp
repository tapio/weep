#pragma once
#include "common.hpp"
#include "soloud.h"

struct Camera;

class AudioSystem
{
public:
	AudioSystem();
	~AudioSystem();
	void reset();
	void update(Camera& camera);

	std::unique_ptr<SoLoud::Soloud> soloud;
};
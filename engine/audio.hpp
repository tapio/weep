#pragma once
#include "common.hpp"
#include "soloud.h"
#include "soloud_wav.h"

struct Camera;

class AudioSystem : public System
{
public:
	AudioSystem();
	~AudioSystem();
	void reset();
	void update(Camera& camera);

	std::unique_ptr<SoLoud::Soloud> soloud;
};

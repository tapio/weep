#pragma once
#include "common.hpp"
#include "soloud.h"
#include "soloud_wav.h"
#include <unordered_map>

struct Transform;

class AudioSystem : public System
{
public:
	AudioSystem();
	~AudioSystem();
	void reset();
	void update(Entities& entities, const Transform& listener);

	void add(const string& name, const std::vector<char>& data);
	uint play(uint eventId);
	uint play(uint eventId, vec3 position);

	std::unique_ptr<SoLoud::Soloud> soloud;

private:
	std::unordered_map<uint, std::vector<std::unique_ptr<SoLoud::Wav>>> m_samples;
};

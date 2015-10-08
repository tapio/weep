#pragma once
#include "common.hpp"
#include "soloud.h"
#include "soloud_wav.h"
#include <unordered_map>

struct Camera;

class AudioSystem : public System
{
public:
	AudioSystem();
	~AudioSystem();
	void reset();
	void update(Entities& entities, Camera& camera);

	void add(const string& name, const std::vector<char>& data);
	void play(const string& name);
	void play(const string &name, vec3 position);

	std::unique_ptr<SoLoud::Soloud> soloud;

private:
	std::unordered_map<string, std::vector<std::unique_ptr<SoLoud::Wav>>> m_samples;
};

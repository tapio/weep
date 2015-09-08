#include "audio.hpp"
#include "soloud.h"

using namespace SoLoud;

AudioSystem::AudioSystem()
{
	soloud.reset(new Soloud());
	soloud->init(Soloud::CLIP_ROUNDOFF, Soloud::SDL2);
}

AudioSystem::~AudioSystem()
{
	soloud->deinit();
	reset();
}

void AudioSystem::reset()
{
}

void AudioSystem::update()
{
	soloud->update3dAudio();
}

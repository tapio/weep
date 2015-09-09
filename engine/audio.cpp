#include "audio.hpp"
#include "camera.hpp"
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

void AudioSystem::update(Camera& camera)
{
	vec3 forward = glm::rotate(camera.rotation, vec3(0, 0, -1));
	vec3 up = glm::rotate(camera.rotation, vec3(0, 1, 0));
	soloud->set3dListenerPosition(camera.position.x, camera.position.y, camera.position.z);
	soloud->set3dListenerAt(forward.x, forward.y, forward.z);
	soloud->set3dListenerUp(up.x, up.y, up.z);
	soloud->update3dAudio();
}

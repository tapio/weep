#include "audio.hpp"
#include "camera.hpp"
#include "soloud.h"
#include "components.hpp"
#include <glm/gtc/random.hpp>

using namespace ecs;

static_assert(sizeof(uint) == sizeof(SoLoud::handle), "Audio handle size mismatch!");


AudioSystem::AudioSystem()
{
	soloud.reset(new SoLoud::Soloud());
	soloud->init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::SDL2);
}

AudioSystem::~AudioSystem()
{
	soloud->deinit();
	reset();
}

void AudioSystem::reset()
{
}

void AudioSystem::update(Entities& entities, const Transform& listener)
{
	vec3 forward = glm::rotate(listener.rotation, forward_axis);
	vec3 up = glm::rotate(listener.rotation, up_axis);
	soloud->set3dListenerPosition(listener.position.x, listener.position.y, listener.position.z);
	soloud->set3dListenerAt(forward.x, forward.y, forward.z);
	soloud->set3dListenerUp(up.x, up.y, up.z);

	// AudioSource
	entities.for_each<AudioSource, Transform>([&](Entity e, AudioSource& sound, Transform& trans) {
		// TODO: Manage distance culling and auto-play
		// TODO: Utilize transform.dirty
		if (soloud->isValidVoiceHandle(sound.handle)) {
			soloud->set3dSourcePosition(sound.handle, trans.position.x, trans.position.y, trans.position.z);
		}
	});

	// Move sounds
	entities.for_each<MoveSound, Transform>([&](Entity e, MoveSound& sound, Transform& trans) {
		ASSERT(!sound.needsGroundContact || e.has<GroundTracker>());
		if (!sound.needsGroundContact || e.get<GroundTracker>().onGround) {
			sound.delta += length(sound.prevPos - trans.position);
			sound.prevPos = trans.position;
			if (sound.delta > sound.stepLength) {
				play(sound.event, trans.position);
				sound.delta = 0;
			}
		}
	});

	// Contact sounds
	entities.for_each<ContactSound, Transform>([&](Entity e, ContactSound& sound, Transform& trans) {
		ASSERT(e.has<ContactTracker>());
		if (e.get<ContactTracker>().hadContact) {
			play(sound.event, trans.position);
		}
	});

	soloud->update3dAudio();
}

void AudioSystem::add(const string& name, const std::vector<char>& data)
{
	auto& samples = m_samples[id::hash(name)];
	samples.emplace_back(new SoLoud::Wav);
	samples.back()->loadMem((unsigned char*)&data[0], data.size(), false, false);
}

uint AudioSystem::play(uint eventId)
{
	auto& samples = m_samples[eventId];
	if (samples.empty()) {
		logWarning("Can't play event %d as it's not loaded", eventId);
		return 0;
	}
	int sampleIndex = glm::linearRand(0, (int)(samples.size() - 1));
	return soloud->play(*samples[sampleIndex]);
}

uint AudioSystem::play(uint eventId, vec3 position)
{
	auto& samples = m_samples[eventId];
	if (samples.empty()) {
		logWarning("Can't play event %d as it's not loaded", eventId);
		return 0;
	}
	int sampleIndex = glm::linearRand(0, (int)(samples.size() - 1));
	return soloud->play3d(*samples[sampleIndex], position.x, position.y, position.z);
}

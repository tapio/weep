#include "animation.hpp"
#include "components.hpp"
#include "geometry.hpp"


AnimationSystem::AnimationSystem()
{
}

AnimationSystem::~AnimationSystem()
{
	reset();
}

void AnimationSystem::reset()
{
}

template<typename T>
static inline void lerpPropertyTrack(PropertyAnimation::Track<T>& track, float time) {
	const auto& keyframes = track.keyframes;
	if (keyframes.empty())
		return;
	if (time <= keyframes[0].time) {
		track.currentValue =  keyframes[0].value;
		return;
	}
	const uint lastIndex = keyframes.size() - 1;
	if (time >= keyframes[lastIndex].time) {
		track.currentValue = keyframes[lastIndex].value;
		return;
	}
	uint i = 0;
	while (i < lastIndex && keyframes[i + 1].time < time) ++i;
	const auto& frame1 = keyframes[i];
	const auto& frame2 = keyframes[i + 1];
	float alpha = (time - frame1.time) / (frame2.time - frame1.time);
	track.currentValue = glm::mix(frame1.value, frame2.value, alpha);
}

static inline float calculateAnimLength(const PropertyAnimation anim) {
	float length = 0.f;
	for (auto& track : anim.floatTracks)
		length = glm::max(track.length(), length);
	for (auto& track : anim.vec3Tracks)
		length = glm::max(track.length(), length);
	for (auto& track : anim.quatTracks)
		length = glm::max(track.length(), length);
	return length;
}

void AnimationSystem::update(Entities& entities, float dt)
{
	entities.for_each<BoneAnimation, Model>([dt](Entity, BoneAnimation& anim, Model& model) {
		if (anim.state != AnimationState::PLAYING)
			return;
		Geometry& geom = *model.lods[0].geometry;
		Geometry::Animation a = geom.animations[anim.animation];
		anim.time += dt * anim.speed * a.frameRate;
		float alpha = glm::fract(anim.time);
		uint frameA = (int)std::floor(anim.time) % a.length;
		uint frameB = (frameA + 1) % a.length;
		mat3x4* matA = &geom.animFrames[frameA * geom.bones.size()];
		mat3x4* matB = &geom.animFrames[frameB * geom.bones.size()];
		ASSERT(anim.bones.size() == geom.bones.size());
		for (uint i = 0; i < anim.bones.size(); ++i) {
			mat3x4 mat = matA[i] * (1.f - alpha) + matB[i] * alpha;
			int parent = geom.boneParents[i];
			ASSERT(parent < (int)i);
			if (parent >= 0) anim.bones[i] = multiplyBones(anim.bones[parent], mat);
			else anim.bones[i] = mat;
		}
	});
	entities.for_each<PropertyAnimation>([this, dt](Entity e, PropertyAnimation& anim) {
		if (anim.state != AnimationState::PLAYING)
			return;
		anim.time += dt * anim.speed;
		for (auto& track : anim.floatTracks) {
			lerpPropertyTrack(track, anim.time);
		}
		for (auto& track : anim.vec3Tracks) {
			lerpPropertyTrack(track, anim.time);
			if (track.id == $id(position)) {
				if (e.has<Transform>())
					e.get<Transform>().setPosition(track.currentValue);
			}
			else if (track.id == $id(scale)) {
				if (e.has<Transform>())
					e.get<Transform>().setScale(track.currentValue);
			}
		}
		for (auto& track : anim.quatTracks) {
			lerpPropertyTrack(track, anim.time);
			if (track.id == $id(rotation)) {
				if (e.has<Transform>())
					e.get<Transform>().setRotation(track.currentValue);
			}
		}
		if (anim.length <= 0.f)
			anim.length = calculateAnimLength(anim);
		if (anim.time >= anim.length) {
			switch (anim.mode) {
				case AnimationMode::ONCE: stop(e); break;
				case AnimationMode::LOOP: anim.time = 0.f; break;
				case AnimationMode::PINGPONG: anim.time = anim.length; anim.speed *= -1.f; break;
			}
		}
	});
}

void AnimationSystem::play(Entity e)
{
	if (e.has<BoneAnimation>() && e.has<Model>()) {
		BoneAnimation& anim = e.get<BoneAnimation>();
		if (anim.state == AnimationState::PAUSED) {
			anim.state = AnimationState::PLAYING;
		} else {
			Geometry& geom = *e.get<Model>().lods[0].geometry;
			Geometry::Animation a = geom.animations[anim.animation];
			anim.state = AnimationState::PLAYING;
			anim.bones.assign(&geom.animFrames[a.start * geom.bones.size()], &geom.animFrames[a.start * geom.bones.size() + geom.bones.size()]);
		}
	}
	if (e.has<PropertyAnimation>()) {
		PropertyAnimation& anim = e.get<PropertyAnimation>();
		anim.state = AnimationState::PLAYING;
		anim.length = calculateAnimLength(anim);
	}
}

void AnimationSystem::pause(Entity e)
{
	if (e.has<BoneAnimation>()) {
		BoneAnimation& anim = e.get<BoneAnimation>();
		anim.state = AnimationState::PAUSED;
	}
	if (e.has<PropertyAnimation>()) {
		PropertyAnimation& anim = e.get<PropertyAnimation>();
		anim.state = AnimationState::PAUSED;
	}
}

void AnimationSystem::stop(Entity e)
{
	if (e.has<BoneAnimation>() && e.has<Model>()) {
		BoneAnimation& anim = e.get<BoneAnimation>();
		anim.state = AnimationState::STOPPED;
		anim.time = 0.f;
		anim.bones.clear();
		anim.bones.assign(e.get<Model>().lods[0].geometry->bones.size(), mat3x4(1));
	}
	if (e.has<PropertyAnimation>()) {
		PropertyAnimation& anim = e.get<PropertyAnimation>();
		anim.state = AnimationState::STOPPED;
		anim.time = 0.f;
		// TODO: Reset to first keyframe
	}
}

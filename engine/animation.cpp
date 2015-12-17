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

void AnimationSystem::update(Entities& entities, float dt)
{
	entities.for_each<Animation, Model>([&](Entity, Animation& anim, Model& model) {
		Geometry& geom = *model.geometry;
		Geometry::Animation a = geom.animations[anim.animation];
		anim.time += dt * anim.speed * a.frameRate;
		float alpha = 0.f; // TODO
		uint frameA = (int)std::floor(anim.time) % a.length;
		uint frameB = (frameA + 1) % a.length;
		mat3x4* matA = &geom.animFrames[frameA * geom.bones.size()];
		mat3x4* matB = &geom.animFrames[frameB * geom.bones.size()];
		ASSERT(anim.bones.size() == geom.bones.size());
		for (uint i = 0; i < anim.bones.size(); ++i) {
			mat3x4 mat = matA[i] * (1.f - alpha) + matB[i] * alpha;
			int parent = geom.boneParents[i];
			ASSERT(parent < (int)i);
			if (parent >= 0) anim.bones[i] = mat3x4(mat4(anim.bones[parent]) * mat4(mat));
			else anim.bones[i] = mat;
		}
	});
}

void AnimationSystem::play(Entity& e)
{
	if (!e.has<Animation>() || !e.has<Model>())
		return;
	Animation& anim = e.get<Animation>();
	Geometry& geom = *e.get<Model>().geometry;
	Geometry::Animation a = geom.animations[anim.animation];
	anim.state = Animation::PLAYING;
	anim.bones.assign(&geom.animFrames[a.start * geom.bones.size()], &geom.animFrames[a.start * geom.bones.size() + geom.bones.size()]);
}

void AnimationSystem::stop(Entity& e)
{
	if (!e.has<Animation>() || !e.has<Model>())
		return;
	Animation& anim = e.get<Animation>();
	anim.state = Animation::STOPPED;
	anim.time = 0.f;
	anim.bones = e.get<Model>().geometry->bones;
}

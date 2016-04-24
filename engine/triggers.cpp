#include "triggers.hpp"
#include "components.hpp"
#include "module.hpp"

TriggerSystem::TriggerSystem()
{
}

TriggerSystem::~TriggerSystem()
{
}

void TriggerSystem::update(Entities& entities, float /*dt*/)
{
	ModuleSystem& modules = entities.get_system<ModuleSystem>();
	entities.for_each<TriggerVolume, Transform>([&](Entity, TriggerVolume& trigger, Transform& volTransform) {
		if (trigger.times <= 0)
			return;
		if (trigger.bounds.radius > 0.f) {
			entities.for_each<TriggerGroup, Transform>([&](Entity, TriggerGroup& group, Transform& transform) {
				if (!(trigger.groups & group.group) || trigger.times <= 0)
					return;
				float rSq = trigger.bounds.radius * trigger.bounds.radius;
				if (glm::distance2(volTransform.position, transform.position) <= rSq) {
					if (!group.triggered) {
						trigger.times--;
						group.triggered = true;
						if (trigger.receiverModule && trigger.enterMessage) {
							modules.call(trigger.receiverModule, trigger.enterMessage, nullptr); // TODO: Some param
						}
					}
				} else {
					if (group.triggered && trigger.receiverModule && trigger.exitMessage) {
						group.triggered = false;
						modules.call(trigger.receiverModule, trigger.exitMessage, nullptr); // TODO: Some param
					}
				}
			});
		} else {
			ASSERT(!"Only sphere trigger volume implemented currently.");
		}
	});
}

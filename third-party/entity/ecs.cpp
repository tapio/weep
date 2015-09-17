#include "ecs.hpp"
#include <algorithm>

namespace entity
{

	BaseComponent::Id BaseComponent::id_counter = 0;

	// Entity

	void Entity::kill()
	{
		entities->kill(*this);
	}

	bool Entity::is_alive() const
	{
		return entities && entities->is_entity_alive(*this);
	}

	void Entity::tag(std::string tag_name)
	{
		entities->tag_entity(*this, tag_name);
	}

	void Entity::group(std::string group_name)
	{
		entities->group_entity(*this, group_name);
	}

	std::string Entity::to_string() const
	{
		std::string s = "entity_" + std::to_string(get_index()) + "_v" + std::to_string(get_version());
		return s;
	}

	// System

	void System::add_entity(Entity e)
	{
		entities.push_back(e);
	}

	void System::remove_entity(Entity e)
	{
		entities.erase(std::remove_if(entities.begin(), entities.end(),
			[&e](Entity other) { return e == other; }
		), entities.end());
	}

	// Entities

	Entities::Entities()
	{
	}

	void Entities::update()
	{
		for (auto e : created_entities) {
			update_systems(e);
		}
		created_entities.clear();

		for (auto e : killed_entities) {
			destroy_entity(e);
		}
		killed_entities.clear();
	}

	void Entities::update_systems(Entity e)
	{
		const auto &entity_component_mask = get_component_mask(e);

		for (auto &it : systems) {
			auto system = it.second;
			const auto &system_component_mask = system->get_component_mask();
			auto interest = (entity_component_mask & system_component_mask) == system_component_mask;

			if (interest) {
				system->add_entity(e);
			}
		}
	}

	Entity Entities::create()
	{
		auto e = create_entity();
		created_entities.push_back(e);
		return e;
	}

	void Entities::kill(Entity e)
	{
		killed_entities.push_back(e);
	}

	Entity Entities::create_entity()
	{
		Entity::Id index;

		if (free_ids.size() > MINIMUM_FREE_IDS) {
			index = free_ids.front();
			free_ids.pop_front();
		}
		else {
			versions.push_back(0);
			index = (unsigned int)versions.size() - 1;
			assert(index < (1 << Entity::INDEX_BITS));

			if (index >= component_masks.size()) {
				// TODO: grow by doubling?
				component_masks.resize(index + 1);
			}
		}

		assert(index < versions.size());
		Entity e(index, versions[index]);
		e.entities = this;

		return e;
	}

	void Entities::destroy_entity(Entity e)
	{
		const auto index = e.get_index();
		assert(index < versions.size());        // sanity check
		assert(index < component_masks.size());
		++versions[index];                      // increase the version for that id
		free_ids.push_back(index);              // make the id available for reuse
		component_masks[index].reset();         // reset the component mask for that id
	}

	bool Entities::is_entity_alive(Entity e) const
	{
		const auto index = e.get_index();
		assert(index < versions.size());
		return versions[index] == e.get_version();
	}

	const ComponentMask& Entities::get_component_mask(Entity e) const
	{
		const auto index = e.get_index();
		assert(index < component_masks.size());
		return component_masks[index];
	}

	void Entities::tag_entity(Entity e, std::string tag_name)
	{
		tagged_entities.emplace(tag_name, e);
	}

	bool Entities::has_tagged_entity(std::string tag_name) const
	{
		return tagged_entities.find(tag_name) != tagged_entities.end();
	}

	Entity Entities::get_entity_by_tag(std::string tag_name)
	{
		assert(has_tagged_entity(tag_name));
		return tagged_entities[tag_name];
	}

	void Entities::group_entity(Entity e, std::string group_name)
	{
		entity_groups.emplace(group_name, std::set<Entity>());
		entity_groups[group_name].emplace(e);
	}

	bool Entities::has_entity_group(std::string group_name) const
	{
		return entity_groups.find(group_name) != entity_groups.end();
	}

	std::vector<Entity> Entities::get_entity_group(std::string group_name)
	{
		assert(has_entity_group(group_name));
		auto &s = entity_groups[group_name];
		return std::vector<Entity>(s.begin(), s.end());
	}

}

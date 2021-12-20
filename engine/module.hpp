#pragma once
#include "common.hpp"
#include <ecs/ecs.hpp>

namespace json11 {
	class Json;
}


struct Module {
	typedef void (*ModuleFunc)(uint msg, void* param);

	Module(const string& name);
	Module(const string& name, ModuleFunc embeddedFunc);
	~Module();

	ModuleFunc func = nullptr;
	bool enabled = true;
	bool embedded = false;
	void* handle = nullptr;
	uint mtime = 0;
	string name;
};


class ModuleSystem : public ecs::System
{
public:
	static void cleanUpHotloadFiles();
	void registerEmbeddedModule(const std::string name, Module::ModuleFunc func); // Only adds it to "search path", does not automatically "load" it so is not callable out-of-the-box (use "load" function as normal)
	void load(const json11::Json& modules, bool clear = true);
	void reload(uint module, void* callbackParam);
	bool autoReload(void* callbackParam);

	void call(uint msg, void* param = nullptr);
	void call(uint module, uint msg, void* param = nullptr);

	bool canReloadAnything = false;

	std::unordered_map<uint, Module> modules = {};
	std::unordered_map<uint, Module::ModuleFunc> embeddedModules = {};
};

#include "module.hpp"
#include <chrono>
#include <thread>
#include <SDL2/SDL_loadso.h>

static inline string getPath(const string& moduleName)
{
#if defined(WIN32) || defined(_WIN32)
	return moduleName + ".dll";
#else
	return "./lib" + moduleName + ".so";
#endif
}

Module::Module(const string& moduleName)
{
	name = moduleName;
	string path = getPath(moduleName);
	handle = SDL_LoadObject(path.c_str());
	if (!handle) {
		logError("%s", SDL_GetError()); // SDL already produces a descriptive error
		ASSERT(!"Module load failed");
		return;
	}
	func = (ModuleFunc)SDL_LoadFunction(handle, "ModuleFunc");
	if (!func) {
		logError("%s (%s)", SDL_GetError(), path.c_str());
		ASSERT(!"ModuleFunc load failed");
		return;
	}
	mtime = timestamp(path);
	logDebug("Loaded module %s", path.c_str());
}

Module::~Module()
{
	if (handle) {
		SDL_UnloadObject(handle);
		handle = 0;
		logDebug("Unloaded module %s", name.c_str());
	}
}

void ModuleSystem::load(const Json& modulesDef, bool clear)
{
	if (clear)
		modules.clear();
	if (modulesDef.is_array()) {
		for (auto& it : modulesDef.array_items())
			modules.emplace(it.string_value(), it.string_value());
	}
}

void ModuleSystem::reload(string name) // Needs to be by value to support passing module.name
{
	modules.erase(name);
	modules.emplace(name, name);
}

bool ModuleSystem::autoReload()
{
	for (auto& it : modules) {
		string name = it.second.name;
		string path = getPath(name);
		if (timestamp(path) > it.second.mtime) {
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
			reload(name);
			return true; // Iterators are now invalid
		}
	}
	return false;
}

void ModuleSystem::call(uint msg, void* param)
{
	for (auto& it : modules)
		if (it.second.func && it.second.enabled)
			it.second.func(msg, param);
}

void ModuleSystem::call(const string& module, uint msg, void* param)
{
	const auto it = modules.find(module);
	if (it != modules.end() && it->second.func && it->second.enabled)
		it->second.func(msg, param);
}

#include "module.hpp"
#include "utils.hpp"
#include <chrono>
#include <thread>
#include <SDL_loadso.h>

using namespace utils;

static inline string getPath(const string& moduleName)
{
#if defined(__MINGW32__)
	return "./lib" + moduleName + ".dll";
#elif defined(WIN32) || defined(_WIN32)
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

Module::Module(const string& moduleName, ModuleFunc embeddedFunc)
{
	name = moduleName;
	func = embeddedFunc;
	embedded = true;
	logDebug("Loaded embedded module %s", moduleName.c_str());
}

Module::~Module()
{
	if (handle) {
		SDL_UnloadObject(handle);
		handle = 0;
		logDebug("Unloaded module %s", name.c_str());
	} else if (!embedded) logWarning("Invalid module handle when destructing %s", name.c_str());
}

void ModuleSystem::registerEmbeddedModule(const std::string name, Module::ModuleFunc func)
{
	embeddedModules.emplace(id::hash(name), func);
}

void ModuleSystem::load(const Json& modulesDef, bool clear)
{
	if (clear) {
		modules.clear();
		canReloadAnything = false;
	}
	if (modulesDef.is_array()) {
		for (auto& it : modulesDef.array_items()) {
			uint hash = id::hash(it.string_value());
			auto embedIt = embeddedModules.find(hash);
			if (embedIt != embeddedModules.end()) {
				if (modules.find(hash) == modules.end()) {
					modules.emplace(hash, Module(it.string_value(), embedIt->second));
				} else logWarning("Module %s already loaded", it.string_value().c_str());
				continue;
			}
			if (modules.emplace(hash, it.string_value()).second) {
				canReloadAnything = true;
			} else logWarning("Module %s already loaded", it.string_value().c_str());
		}
	}
}

void ModuleSystem::reload(uint module)
{
	const auto it = modules.find(module);
	if (it != modules.end() && !it->second.embedded) {
		string name = it->second.name;
		modules.erase(it);
		modules.emplace(module, name);
	}
}

bool ModuleSystem::autoReload()
{
	for (auto& it : modules) {
		if (it.second.embedded)
			continue;
		string path = getPath(it.second.name);
		if (timestamp(path) > it.second.mtime) {
			sleep(300);
			reload(it.first);
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

void ModuleSystem::call(uint module, uint msg, void* param)
{
	const auto it = modules.find(module);
	if (it != modules.end() && it->second.func && it->second.enabled)
		it->second.func(msg, param);
}

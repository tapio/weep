#include "module.hpp"
#include <SDL2/SDL_loadso.h>

Module::Module(const std::string& moduleName)
{
	name = moduleName;
#if defined(WIN32) || defined(_WIN32)
	string path = name + ".dll";
#else
	string path = "lib" + name + ".so";
#endif
	handle = SDL_LoadObject(path.c_str());
	ASSERT(handle);
	init = (ModuleInitFunc)SDL_LoadFunction(handle, "ModuleInit");
	deinit = (ModuleDeinitFunc)SDL_LoadFunction(handle, "ModuleDeinit");
	update = (ModuleUpdateFunc)SDL_LoadFunction(handle, "ModuleUpdate");
	ASSERT(init || deinit || update);
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

void Modules::load(const Json& modules)
{
	clear();
	if (modules.is_array()) {
		for (auto& it : modules.array_items())
			emplace(it.string_value(), it.string_value());
	}
}

void Modules::reload(const string& name)
{
	erase(name);
	emplace(name, name);
}

void Modules::init(Entities& entities)
{
	for (auto& it : *this)
		if (it.second.init)
			it.second.init(entities);
}

void Modules::deinit(Entities& entities)
{
	for (auto& it : *this)
		if (it.second.deinit)
			it.second.deinit(entities);
}

void Modules::update(Entities& entities)
{
	for (auto& it : *this)
		if (it.second.update && it.second.enabled)
			it.second.update(entities);
}

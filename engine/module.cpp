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
	func = (ModuleFunc)SDL_LoadFunction(handle, "ModuleFunc");
	ASSERT(func);
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

void Modules::call(uint msg, void* param)
{
	for (auto& it : *this)
		if (it.second.func && it.second.enabled)
			it.second.func(msg, param);
}

void Modules::init(Entities& entities)
{
	for (auto& it : *this)
		if (it.second.func)
			it.second.func($id(INIT), &entities);
}

void Modules::deinit(Entities& entities)
{
	for (auto& it : *this)
		if (it.second.func)
			it.second.func($id(DEINIT), &entities);
}

void Modules::update(Entities& entities)
{
	call($id(UPDATE), &entities);
}

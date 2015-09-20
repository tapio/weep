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

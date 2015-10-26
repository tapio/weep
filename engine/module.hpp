#pragma once
#include "common.hpp"

struct Module {
	typedef void (*ModuleFunc)(uint msg, void* param);

	Module(const string& name);
	~Module();

	ModuleFunc func = nullptr;
	bool enabled = true;
	void* handle = nullptr;
	uint mtime = 0;
	string name;
};


class ModuleSystem : public System
{
public:
	void load(const Json& modules, bool clear = true);
	void reload(uint module);
	bool autoReload();

	void call(uint msg, void* param = nullptr);
	void call(uint module, uint msg, void* param = nullptr);

	std::map<uint, Module> modules = {};
};

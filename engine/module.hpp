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


class ModuleSystem
{
public:
	void load(const Json& modules);
	void reload(const string& name);
	bool autoReload();

	void call(uint msg, void* param = nullptr);
	void call(const string& module, uint msg, void* param = nullptr);

	std::map<string, Module> modules = {};
};

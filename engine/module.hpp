#include "common.hpp"

struct Module {
	typedef void (*ModuleInitFunc)(Entities& entities);
	typedef void (*ModuleDeinitFunc)(Entities& entities);
	typedef void (*ModuleUpdateFunc)(Entities& entities);

	Module(const string& name);
	~Module();

	ModuleInitFunc init = nullptr;
	ModuleDeinitFunc deinit = nullptr;
	ModuleUpdateFunc update = nullptr;
	void* handle = nullptr;
	string name;
};

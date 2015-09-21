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
	bool enabled = true;
	void* handle = nullptr;
	string name;
};


class Modules : public std::map<string, Module>
{
public:
	void load(const Json& modules);
	void reload(const string& name);

	void init(Entities& entities);
	void deinit(Entities& entities);
	void update(Entities& entities);
};

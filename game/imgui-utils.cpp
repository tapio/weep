
#include "imgui-utils.hpp"
#include "utils.hpp"

#include <map>

using namespace magic_enum::bitwise_operators;
static std::map<std::size_t, std::vector<std::string>> s_EnumOptions;


const std::vector<std::string>* FindEnumOptions(std::size_t handle)
{
	auto it = s_EnumOptions.find(handle);
	if (it != s_EnumOptions.end())
		return &it->second;
	return nullptr;
}

void RegisterEnumOptions(std::size_t handle, const std::vector<std::string>& options)
{
	s_EnumOptions[handle] = options;
}

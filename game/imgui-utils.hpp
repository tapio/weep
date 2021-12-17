#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include <imgui/imgui_internal.h>

#include <string>
#include <vector>
#include <functional>
#include <magic_enum/magic_enum.hpp>


const std::vector<std::string>* FindEnumOptions(std::size_t handle);
void RegisterEnumOptions(std::size_t handle, const std::vector<std::string>& options);

template<typename T>
const std::vector<std::string>& GetEnumOptions() {
	std::size_t handle = typeid(T).hash_code();
	if (auto ret = FindEnumOptions(handle))
		return *ret;
	std::vector<std::string> options;
	for (auto& option : magic_enum::enum_names<T>()) {
		options.emplace_back(option);
	}
	RegisterEnumOptions(handle, options);
	return *FindEnumOptions(handle);
}


inline bool PlainShortcut(ImGuiKey key, bool allowDuringTextEdit = false) {
	ImGuiIO& io = ImGui::GetIO();
	return !io.KeyCtrl && !io.KeyShift && !io.KeyAlt && (!io.WantTextInput || allowDuringTextEdit) && ImGui::IsKeyPressedMap(key, false);
}

inline bool CtrlShortcut(ImGuiKey key, bool allowDuringTextEdit = false) {
	ImGuiIO& io = ImGui::GetIO();
	return io.KeyCtrl && !io.KeyShift && !io.KeyAlt && (!io.WantTextInput || allowDuringTextEdit) && ImGui::IsKeyPressedMap(key, false);
}

inline bool ShiftShortcut(ImGuiKey key, bool allowDuringTextEdit = false) {
	ImGuiIO& io = ImGui::GetIO();
	return !io.KeyCtrl && io.KeyShift && !io.KeyAlt && (!io.WantTextInput || allowDuringTextEdit) && ImGui::IsKeyPressedMap(key, false);
}

inline void ImGuiDisabledWrapper(bool disabled, std::function<void()> f) {
	if (disabled) {
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}
	f();
	if (disabled) {
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}
}

#define HandleTooltip(tooltip) do { \
	if (ImGui::IsItemHovered()) \
		ImGui::SetTooltip(tooltip); } while (0)

inline float GetClampedAvailableWidth(float maxWidth) {
	float avail = ImGui::GetContentRegionAvail().x;
	return avail < maxWidth ? avail : maxWidth;
}

template<typename T>
inline bool EnumWidget(const char* label, T* enumVar, float w = 0.f) {
	const auto& opts = GetEnumOptions<T>();
	if (w > 0.f) ImGui::PushItemWidth(w);
	bool ret = ImGui::Combo(label, (int*)enumVar, opts);
	if (w > 0.f) ImGui::PopItemWidth();
	return ret;
}

// Copy pasted internal ImGui::CheckboxFlagsT to get enum class support
// Embedded enum bitwise operator boilerplate due to clang having problems with using namespace magic_enum::bitwise_operators when the enums are inside editor namespace
template<typename T>
inline bool CheckboxFlagsE(const char* label, T* flags, T flags_value)
{
	bool all_on = (static_cast<T>(static_cast<std::underlying_type_t<T>>(*flags) & static_cast<std::underlying_type_t<T>>(flags_value))) == flags_value;
	bool any_on = (static_cast<T>(static_cast<std::underlying_type_t<T>>(*flags) & static_cast<std::underlying_type_t<T>>(flags_value))) != (T)0;
	bool pressed;
	if (!all_on && any_on)
	{
		ImGuiContext& g = *GImGui;
		ImGuiItemFlags backup_item_flags = g.CurrentItemFlags;
		g.CurrentItemFlags |= ImGuiItemFlags_MixedValue;
		pressed = ImGui::Checkbox(label, &all_on);
		g.CurrentItemFlags = backup_item_flags;
	}
	else
	{
		pressed = ImGui::Checkbox(label, &all_on);

	}
	if (pressed)
	{
		if (all_on)
			*flags = (static_cast<T>(static_cast<std::underlying_type_t<T>>(*flags) | static_cast<std::underlying_type_t<T>>(flags_value)));
		else
			*flags = (static_cast<T>(static_cast<std::underlying_type_t<T>>(*flags) & (~static_cast<std::underlying_type_t<T>>(flags_value))));
	}
	return pressed;
}

// ImGui SDL2 binding with OpenGL3 + shaders
// https://github.com/ocornut/imgui

struct SDL_Window;
typedef union SDL_Event SDL_Event;

IMGUI_API bool        ImGui_ImplSDLGL3_Init(SDL_Window* window);
IMGUI_API void        ImGui_ImplSDLGL3_Shutdown();
IMGUI_API void        ImGui_ImplSDLGL3_NewFrame();
IMGUI_API bool        ImGui_ImplSDLGL3_ProcessEvent(SDL_Event* event);

// Use if you want to reset your rendering device without losing ImGui state.
IMGUI_API void        ImGui_ImplSDLGL3_InvalidateDeviceObjects();
IMGUI_API bool        ImGui_ImplSDLGL3_CreateDeviceObjects();

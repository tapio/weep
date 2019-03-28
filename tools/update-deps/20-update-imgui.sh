#!/bin/bash -e

source "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"/config.sh

BRANCH="docking"

# ImGui
for f in LICENSE.txt imconfig.h imgui.cpp imgui.h imgui_demo.cpp imgui_draw.cpp imgui_internal.h imgui_tables.cpp imgui_widgets.cpp imstb_rectpack.h imstb_textedit.h imstb_truetype.h; do
	$CURL $GHBASEURL/ocornut/imgui/$BRANCH/$f -o imgui/$f &
done
$CURL $GHBASEURL/ocornut/imgui/$BRANCH/misc/cpp/imgui_stdlib.h -o imgui/imgui_stdlib.h &
$CURL $GHBASEURL/ocornut/imgui/$BRANCH/misc/cpp/imgui_stdlib.cpp -o imgui/imgui_stdlib.cpp &
$CURL $GHBASEURL/ocornut/imgui/$BRANCH/backends/imgui_impl_opengl3.h -o imgui/imgui_impl_opengl3.h &
$CURL $GHBASEURL/ocornut/imgui/$BRANCH/backends/imgui_impl_opengl3.cpp -o imgui/imgui_impl_opengl3.cpp &
$CURL $GHBASEURL/ocornut/imgui/$BRANCH/backends/imgui_impl_opengl3_loader.h -o imgui/imgui_impl_opengl3_loader.h &
$CURL $GHBASEURL/ocornut/imgui/$BRANCH/backends/imgui_impl_sdl.h -o imgui/imgui_impl_sdl.h &
$CURL $GHBASEURL/ocornut/imgui/$BRANCH/backends/imgui_impl_sdl.cpp -o imgui/imgui_impl_sdl.cpp &

wait

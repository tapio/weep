#!/bin/bash -e

source "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"/config.sh

# ImGui
for f in LICENSE.txt imconfig.h imgui.cpp imgui.h imgui_demo.cpp imgui_draw.cpp imgui_internal.h imgui_tables.cpp imgui_widgets.cpp imstb_rectpack.h imstb_textedit.h imstb_truetype.h; do
	$WGET $GHBASEURL/ocornut/imgui/master/$f -O imgui/$f &
done
$WGET $GHBASEURL/ocornut/imgui/master/misc/cpp/imgui_stdlib.h -O imgui/imgui_stdlib.h &
$WGET $GHBASEURL/ocornut/imgui/master/misc/cpp/imgui_stdlib.cpp -O imgui/imgui_stdlib.cpp &
$WGET $GHBASEURL/ocornut/imgui/master/backends/imgui_impl_opengl3.h -O imgui/imgui_impl_opengl3.h &
$WGET $GHBASEURL/ocornut/imgui/master/backends/imgui_impl_opengl3.cpp -O imgui/imgui_impl_opengl3.cpp &
$WGET $GHBASEURL/ocornut/imgui/master/backends/imgui_impl_opengl3_loader.h -O imgui/imgui_impl_opengl3_loader.h &
$WGET $GHBASEURL/ocornut/imgui/master/backends/imgui_impl_sdl.h -O imgui/imgui_impl_sdl.h &
$WGET $GHBASEURL/ocornut/imgui/master/backends/imgui_impl_sdl.cpp -O imgui/imgui_impl_sdl.cpp &

wait

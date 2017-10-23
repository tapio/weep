#!/bin/bash -e

source "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"/config.sh

# ImGui
for f in imconfig.h imgui.cpp imgui.h imgui_demo.cpp imgui_draw.cpp imgui_internal.h LICENSE.txt stb_rect_pack.h stb_textedit.h stb_truetype.h; do
	$WGET $GHBASEURL/ocornut/imgui/master/$f -O imgui/$f &
done
$WGET $GHBASEURL/ocornut/imgui/master/examples/sdl_opengl3_example/imgui_impl_sdl_gl3.h -O imgui/imgui_impl_sdl_gl3.h &
$WGET $GHBASEURL/ocornut/imgui/master/examples/sdl_opengl3_example/imgui_impl_sdl_gl3.cpp -O imgui/imgui_impl_sdl_gl3.cpp &

wait
patch --verbose -p0 < "$PATCHDIR/20-ImGUIImpl.patch"

#!/bin/bash -e

ROOT="$(dirname "$(readlink -f "$0")")"/..
GHBASEURL=https://raw.githubusercontent.com
WGET="wget -nv"

cd "$ROOT/third-party"

# IQM
$WGET $GHBASEURL/lsalzman/iqm/master/iqm.h -O iqm/iqm.h &

# Json11
$WGET $GHBASEURL/dropbox/json11/master/json11.hpp -O json11/json11.hpp &
$WGET $GHBASEURL/dropbox/json11/master/json11.cpp -O json11/json11.cpp &
$WGET $GHBASEURL/dropbox/json11/master/LICENSE.txt -O json11/LICENSE.txt &

# Remotery
$WGET $GHBASEURL/Celtoys/Remotery/master/lib/Remotery.h -O remotery/Remotery.h &
$WGET $GHBASEURL/Celtoys/Remotery/master/lib/Remotery.c -O remotery/Remotery.c &
$WGET $GHBASEURL/Celtoys/Remotery/master/LICENSE -O remotery/LICENSE &

# stb_image
$WGET $GHBASEURL/nothings/stb/master/stb_image.h -O stb_image/stb_image.h &
$WGET $GHBASEURL/nothings/stb/master/stb_image_write.h -O stb_image/stb_image_write.h &

# gif-h
$WGET $GHBASEURL/ginsweater/gif-h/master/gif.h -O gif-h/gif.h &
$WGET $GHBASEURL/ginsweater/gif-h/master/LICENSE -O gif-h/LICENSE &

# ImGui
for f in imconfig.h imgui.cpp imgui.h imgui_demo.cpp imgui_draw.cpp imgui_internal.h LICENSE stb_rect_pack.h stb_textedit.h stb_truetype.h; do
	$WGET $GHBASEURL/ocornut/imgui/master/$f -O imgui/$f &
done
$WGET $GHBASEURL/ocornut/imgui/master/examples/sdl_opengl3_example/imgui_impl_sdl_gl3.h -O imgui/imgui_impl_sdl_gl3.h &
$WGET $GHBASEURL/ocornut/imgui/master/examples/sdl_opengl3_example/imgui_impl_sdl_gl3.cpp -O imgui/imgui_impl_sdl_gl3.cpp &

# GLM
GLMPATH=/tmp/glm
rm -rf "$GLMPATH"
git clone --depth=1 https://github.com/g-truc/glm "$GLMPATH"

rm -r glm
mkdir -p glm
cp -vr "$GLMPATH/glm/" .
cp -v "$GLMPATH/copying.txt" "glm/"
rm -v "glm/CMakeLists.txt"
rm -v "glm/detail/glm.cpp"
rm -v "glm/detail/dummy.cpp"


# Bullet
BULLETPATH=/tmp/bullet3
rm -rf "$BULLETPATH"
git clone --depth=1 https://github.com/bulletphysics/bullet3 "$BULLETPATH"

rm -r "bullet"
mkdir -p "bullet"
cp -vr "$BULLETPATH/src/BulletCollision" "bullet/"
cp -vr "$BULLETPATH/src/BulletDynamics" "bullet/"
#cp -vr "$BULLETPATH/src/BulletSoftBody" "bullet/"
cp -vr "$BULLETPATH/src/LinearMath" "bullet/"
cp -v "$BULLETPATH/src/btBulletCollisionCommon.h" "bullet/"
cp -v "$BULLETPATH/src/btBulletDynamicsCommon.h" "bullet/"
cp -v "$BULLETPATH/LICENSE.txt" "bullet/"
rm -vr "bullet/BulletDynamics/Featherstone"
rm -vr "bullet/BulletDynamics/MLCPSolvers"
find bullet -type f \( -name 'CMakeLists.txt' -o -name 'premake4.lua' \) -delete


# SoLoud
SOLOUDPATH=/tmp/soloud
rm -rf "$SOLOUDPATH"
git clone --depth=1 https://github.com/jarikomppa/soloud "$SOLOUDPATH"

rm -r soloud
mkdir -p soloud/src/audiosource soloud/src/backend
cp -v "$SOLOUDPATH/LICENSE" "soloud/"
cp -vr "$SOLOUDPATH/include/" "soloud/"
cp -vr "$SOLOUDPATH/src/audiosource/wav/" "soloud/src/audiosource/"
cp -vr "$SOLOUDPATH/src/backend/null/" "soloud/src/backend/"
cp -vr "$SOLOUDPATH/src/backend/sdl2_static/" "soloud/src/backend/"
cp -vr "$SOLOUDPATH/src/core/" "soloud/src/"
cp -vr "$SOLOUDPATH/src/filter/" "soloud/src/"


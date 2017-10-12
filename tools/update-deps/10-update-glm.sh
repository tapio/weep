#!/bin/bash -e

source "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"/config.sh

# GLM
GLMPATH=/tmp/glm
rm -rf "$GLMPATH"
git clone --branch=0.9.8 --depth=1 https://github.com/g-truc/glm "$GLMPATH"

rm -r glm
mkdir -p glm
cp -vr "$GLMPATH/glm/" .
cp -v "$GLMPATH/copying.txt" "glm/"
rm -v "glm/CMakeLists.txt"
rm -v "glm/detail/glm.cpp"
rm -v "glm/detail/dummy.cpp"
#wget https://github.com/g-truc/glm/blob/manual/copying.txt


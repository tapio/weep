#!/bin/bash -e

source "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"/config.sh

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


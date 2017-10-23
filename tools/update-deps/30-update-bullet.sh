#!/bin/bash -e

source "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"/config.sh

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

patch -p0 < "$PATCHDIR/01-btRigidBodyConstructor.patch"


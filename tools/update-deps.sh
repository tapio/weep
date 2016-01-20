#!/bin/bash

ROOT="$(dirname "$(readlink -f "$0")")"/..
GHBASEURL=https://raw.githubusercontent.com

cd "$ROOT/third-party"

# IQM
wget $GHBASEURL/lsalzman/iqm/master/iqm.h -O iqm/iqm.h

# Json11
wget $GHBASEURL/dropbox/json11/master/json11.hpp -O json11/json11.hpp
wget $GHBASEURL/dropbox/json11/master/json11.cpp -O json11/json11.cpp
wget $GHBASEURL/dropbox/json11/master/LICENSE.txt -O json11/LICENSE.txt

# Remotery
wget $GHBASEURL/Celtoys/Remotery/master/lib/Remotery.h -O remotery/Remotery.h
wget $GHBASEURL/Celtoys/Remotery/master/lib/Remotery.c -O remotery/Remotery.c
wget $GHBASEURL/Celtoys/Remotery/master/LICENSE -O remotery/LICENSE

# stb_image
wget $GHBASEURL/nothings/stb/master/stb_image.h -O stb_image/stb_image.h


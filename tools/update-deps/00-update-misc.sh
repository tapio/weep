#!/bin/bash -e

source "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"/config.sh

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


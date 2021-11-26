#!/bin/bash -e

source "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"/config.sh

# IQM
$CURL $GHBASEURL/lsalzman/iqm/master/iqm.h -o iqm/iqm.h &

# Json11
$CURL $GHBASEURL/dropbox/json11/master/json11.hpp -o json11/json11.hpp &
$CURL $GHBASEURL/dropbox/json11/master/json11.cpp -o json11/json11.cpp &
$CURL $GHBASEURL/dropbox/json11/master/LICENSE.txt -o json11/LICENSE.txt &

# stb_image
$CURL $GHBASEURL/nothings/stb/master/stb_image.h -o stb_image/stb_image.h &
$CURL $GHBASEURL/nothings/stb/master/stb_image_write.h -o stb_image/stb_image_write.h &

# gif-h
$CURL $GHBASEURL/ginsweater/gif-h/master/gif.h -o gif-h/gif.h &
$CURL $GHBASEURL/ginsweater/gif-h/master/LICENSE -o gif-h/LICENSE &


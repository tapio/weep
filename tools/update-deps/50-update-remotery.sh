#!/bin/bash -e

source "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"/config.sh

# Remotery
REMOTERYPATH=/tmp/remotery
rm -rf "$REMOTERYPATH"
git clone --depth=1 https://github.com/Celtoys/Remotery "$REMOTERYPATH"

VISPATH="$ROOT/tools/remotery-vis"
rm -r "$VISPATH"
cp -vr "$REMOTERYPATH/vis/" "$VISPATH"

$WGET $GHBASEURL/Celtoys/Remotery/master/lib/Remotery.h -O remotery/Remotery.h &
$WGET $GHBASEURL/Celtoys/Remotery/master/lib/Remotery.c -O remotery/Remotery.c &
$WGET $GHBASEURL/Celtoys/Remotery/master/LICENSE -O remotery/LICENSE &

wait

cat << EOF > "$VISPATH/Readme.txt"
Remotery profiler web browser visualizer
https://github.com/Celtoys/Remotery/

EOF

#!/bin/bash -e

source "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"/config.sh

# Remotery
REMOTERYPATH=/tmp/remotery
rm -rf "$REMOTERYPATH"
git clone --depth=1 https://github.com/Celtoys/Remotery "$REMOTERYPATH"

VISPATH="$ROOT/tools/remotery-vis"
rm -r "$VISPATH"
cp -vr "$REMOTERYPATH/vis/" "$VISPATH"

$CURL $GHBASEURL/Celtoys/Remotery/master/lib/Remotery.h -o remotery/Remotery.h &
$CURL $GHBASEURL/Celtoys/Remotery/master/lib/Remotery.c -o remotery/Remotery.c &
$CURL $GHBASEURL/Celtoys/Remotery/master/LICENSE -o remotery/LICENSE &

wait

cat << EOF > "$VISPATH/Readme.txt"
Remotery profiler web browser visualizer
https://github.com/Celtoys/Remotery/

EOF

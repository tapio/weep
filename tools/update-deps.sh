#!/bin/bash -e

THISPATH="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
source "$THISPATH"/update-deps/config.sh

FILES="$THISPATH"/update-deps/*update-*.sh

function DoScript() {
	echo `basename "$1"`
	echo "======================="
	$1
	echo "======================="
	echo
}

if [ -z "$1" ]; then
	for i in $FILES; do
		DoScript "$i"
	done
else
	DoScript "$THISPATH"/update-deps/`basename "$1"`
fi

echo Done

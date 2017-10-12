#!/bin/bash -e

THISPATH="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
source "$THISPATH"/update-deps/config.sh

FILES="$THISPATH"/update-deps/*update-*.sh

for i in $FILES; do
	echo `basename "$i"`
	echo "======================="
	$i
	echo "======================="
	echo
done

echo Done

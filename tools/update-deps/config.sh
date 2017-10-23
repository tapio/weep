ROOT="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"/../..
PATCHDIR="$ROOT/tools/update-deps/patches"
GHBASEURL=https://raw.githubusercontent.com
WGET="wget -nv"

cd "$ROOT/third-party"


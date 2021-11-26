ROOT="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"/../..
PATCHDIR="$ROOT/tools/update-deps/patches"
GHBASEURL=https://raw.githubusercontent.com
CURL="curl --silent"

cd "$ROOT/third-party"


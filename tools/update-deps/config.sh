ROOT="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"/../..
PATCHDIR="$ROOT/tools/update-deps/patches"
GHBASEURL=https://raw.githubusercontent.com

if [ "$(expr substr $(uname -s) 1 5)" == "MINGW" ]; then
	WGET="$ROOT/tools/wget.exe -nv"
else
	WGET="wget -nv"
fi

cd "$ROOT/third-party"


#!/bin/sh
# ---------------------------------------------------------------------------
#  Installs the Emscripten SDK, which is what `make web` compiles with.
#  Normally invoked as:  make bootstrap
#
#  Everything lands in ./emsdk (~1.5 GB) inside this repo, which .gitignore
#  excludes. Nothing outside this folder is touched -- no system packages, no
#  changes to your shell profile, no sudo. Delete ./emsdk to undo it entirely.
#
#  Afterwards `make web` finds the compiler on its own; you do NOT need to
#  source emsdk_env.sh, though you can if you want emcc on your PATH:
#      . ./emsdk/emsdk_env.sh
#
#  Prerequisites: git, python3, and about 1.5 GB of disk.
# ---------------------------------------------------------------------------
set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "$ROOT"

missing=
for tool in git python3; do
	command -v "$tool" >/dev/null 2>&1 || missing="$missing $tool"
done
if [ -n "$missing" ]; then
	echo "error: emsdk needs:$missing" >&2
	echo "  macOS:          xcode-select --install   (git)   and python3 from python.org or brew" >&2
	echo "  Debian/Ubuntu:  sudo apt install git python3" >&2
	exit 1
fi

echo "Installing the Emscripten SDK into $ROOT/emsdk (~1.5 GB)."
echo "This is gitignored and self-contained; delete the folder to undo."
echo

if [ -d emsdk/.git ]; then
	echo "==> emsdk already present, updating"
	cd emsdk && git pull --ff-only
else
	echo "==> cloning emsdk"
	git clone --depth 1 https://github.com/emscripten-core/emsdk.git
	cd emsdk
fi

echo "==> installing the latest toolchain"
./emsdk install latest

echo "==> activating it"
./emsdk activate latest

echo
echo "Done. Now run:"
echo "    make web      # build  -> build/web/tank.html"
echo "    make serve    # build and serve on http://localhost:8000/tank.html"

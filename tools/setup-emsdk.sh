#!/bin/sh
# ---------------------------------------------------------------------------
#  Installs the Emscripten SDK, needed for the WebAssembly work described in
#  docs/PORTING.md. Not run automatically -- emcc is currently NOT installed
#  on this machine.
#
#  Installs into ./emsdk (about 1.5 GB). Nothing outside this folder is touched.
#
#    sh tools/setup-emsdk.sh
#    . ./emsdk/emsdk_env.sh      # per shell, afterwards
#    emcc -v
# ---------------------------------------------------------------------------
set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "$ROOT"

if [ -d emsdk ]; then
	echo "emsdk/ already exists -- updating"
	cd emsdk && git pull
else
	git clone https://github.com/emscripten-core/emsdk.git
	cd emsdk
fi

./emsdk install latest
./emsdk activate latest

echo
echo "Done. Activate it in each new shell with:"
echo "    . $ROOT/emsdk/emsdk_env.sh"
echo
echo "Then start with the Route A spike described in docs/PORTING.md section 2.3."

#!/bin/sh
# ---------------------------------------------------------------------------
#  Wraps docs/overview.html (the Artifact source, which has no <!doctype>,
#  <html>, <head> or <body> because the Artifact publisher supplies those)
#  into docs/overview.standalone.html -- a complete, self-contained document
#  that renders correctly when opened from disk or served from a repo.
#
#  Run this after editing docs/overview.html so the two never drift.
#
#    sh tools/build-overview.sh
# ---------------------------------------------------------------------------
set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
SRC="$ROOT/docs/overview.html"
OUT="$ROOT/docs/overview.standalone.html"

[ -f "$SRC" ] || { echo "missing $SRC" >&2; exit 1; }

{
	cat <<'HEAD'
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="description" content="How a 2007 fixed-function OpenGL tank game was brought back to life on Apple Silicon, and what a WebAssembly port would actually cost.">
<meta name="color-scheme" content="light dark">
<style>
/* minimal reset -- the Artifact host supplies one; a standalone file must not rely on it */
*,*::before,*::after{box-sizing:border-box;}
body{margin:0;}
img,svg{max-width:100%;height:auto;}
</style>
HEAD
	cat "$SRC"
	cat <<'TAIL'
</body>
</html>
TAIL
} > "$OUT"

# The <title> and <style> from the source live in <head>; everything after the
# first element opens the body. Browsers reconcile this correctly, but be explicit:
# insert the </head><body> boundary just before the first <div class="wrap">.
/usr/bin/sed -i '' 's|^<div class="wrap">$|</head>\n<body>\n<div class="wrap">|' "$OUT"

echo "wrote $OUT ($(wc -c < "$OUT" | tr -d ' ') bytes)"

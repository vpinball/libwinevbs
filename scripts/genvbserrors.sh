#!/bin/bash
#
# Regenerate the English VBScript error-string table in
# wine/dlls/vbscript/vbscript_main.c from the vendored Wine resource file
# wine/dlls/vbscript/vbscript.rc.
#
# libwinevbs does not compile/link Wine's string resources (that would pull in
# the resource and locale stack), so get_vbscript_string() looks the messages
# up from a static table instead of via LoadStringW. This script keeps that
# table in sync with Wine: after bumping the vendored Wine sources, re-run it so
# the table matches the new vbscript.rc.
#
# Usage: ./scripts/genvbserrors.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

python3 - "$ROOT" <<'PYEOF'
import re, sys, pathlib

root = pathlib.Path(sys.argv[1])
rc_path = root / "wine/dlls/vbscript/vbscript.rc"
src_path = root / "wine/dlls/vbscript/vbscript_main.c"

BEGIN = "BEGIN GENERATED ERROR STRINGS"
END = "END GENERATED ERROR STRINGS"

rc = rc_path.read_text(encoding="utf-8")

# Grab the English STRINGTABLE body (the only one; translations live in po files).
m = re.search(r"STRINGTABLE\s*\{(.*?)^\}", rc, re.DOTALL | re.MULTILINE)
if not m:
    sys.exit("no STRINGTABLE found in %s" % rc_path)

# Each entry is: <IDENT>  "<text>"  (Wine escapes an embedded quote as "").
entries = re.findall(r'^\s*([A-Za-z_]\w*)\s+"(.*)"\s*$', m.group(1), re.MULTILINE)
if not entries:
    sys.exit("no STRINGTABLE entries found in %s" % rc_path)

width = max(len(name) for name, _ in entries) + 1  # +1 for the trailing comma
rows = []
for name, text in entries:
    text = text.replace('""', '\\"')  # RC doubled-quote -> C escaped quote
    rows.append('        {{ {label:<{w}} L"{text}" }},'.format(
        label=name + ",", w=width, text=text))

src = src_path.read_text(encoding="utf-8").split("\n")
out, skipping = [], False
for line in src:
    if BEGIN in line:
        out.append(line)
        out.extend(rows)
        skipping = True
        continue
    if END in line:
        skipping = False
    if not skipping:
        out.append(line)

if skipping:
    sys.exit("found '%s' marker but no matching '%s' in %s" % (BEGIN, END, src_path))

src_path.write_text("\n".join(out), encoding="utf-8")
print("genvbserrors: wrote %d error strings to %s" % (len(rows), src_path.name))
PYEOF

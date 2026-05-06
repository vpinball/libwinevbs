#!/bin/bash
#
# Regenerate the IDispatch proxy stubs from Wine IDL files using the Java
# parser under tools/idl-parser. Output lands directly in wine/dlls/<module>/.
#
# Usage: ./scripts/genproxy.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PARSER_DIR="$SCRIPT_DIR/../tools/idl-parser"

cd "$PARSER_DIR"
gradle run --quiet

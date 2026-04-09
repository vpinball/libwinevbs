#!/bin/bash

# Compare libwinevbs Wine sources against upstream Wine to show local modifications.
# Usage: ./scripts/winediff.sh /path/to/wine

if [ -z "$1" ]; then
   echo "Usage: $0 <path-to-wine-source>"
   exit 1
fi

WINE_SRC="$1"
WINE_LOCAL="$(cd "$(dirname "$0")/.." && pwd)/wine"

find "$WINE_LOCAL" -type f | while read i; do
   # skip macOS artifacts
   if [[ "$i" == *".DS_Store" ]]; then
      continue
   fi

   RELATIVE_PATH="${i#$WINE_LOCAL/}"

   # skip generated parser files
   if [ "$RELATIVE_PATH" = "dlls/vbscript/parser.tab.c" ] ||
      [ "$RELATIVE_PATH" = "dlls/vbscript/parser.tab.h" ]; then
      continue
   fi

   FILE="$WINE_SRC/$RELATIVE_PATH"
   if [ -f "$FILE" ]; then
      #PATCH=$(diff -u --label "wine/$RELATIVE_PATH" --label "libwinevbs/$RELATIVE_PATH" -w --strip-trailing-cr "$FILE" "$i")
      PATCH=$(diff -u --label "wine/$RELATIVE_PATH" --label "libwinevbs/$RELATIVE_PATH" --strip-trailing-cr "$FILE" "$i")
      if [ -n "$PATCH" ]; then
         echo "$PATCH"
      fi
   else
      echo "# libwinevbs-only: $RELATIVE_PATH"
   fi
done

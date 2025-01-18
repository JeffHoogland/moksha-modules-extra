#!/bin/bash

# Path to the directory containing LINGUAS and POTFILES.in
TRANSLATION_DIR="."

# Read the LINGUAS file (single-line languages)
LINGUAS=$(cat "$TRANSLATION_DIR/LINGUAS")

# Iterate through each language code in LINGUAS
for LANG in $LINGUAS; do
    if [[ -n "$LANG" && "$LANG" != "#"* ]]; then
        # Generate the .po file for the language
        msginit --input="$TRANSLATION_DIR/diskio.pot" \
                --locale="$LANG" \
                --output-file="$TRANSLATION_DIR/$LANG.po" \
                --no-translator
    fi
done

#!/bin/bash
# -----------------------------------------------------------------------------
# Script Name: generate_game_data_dumps.sh
# Description: This script automates the generation of documentation (Structure)
#              and raw data dumps (Hex & Text) from the game's resource files.
#              It generates content in:
#              - doc/specs/RES_Structure/ (Structure Analysis)
#              - doc/specs/RES_Dump/      (Hex Dumps)
#              - doc/specs/RES_TXT/       (Text Extraction)
#              This is useful for regenerating analysis files that are excluded
#              from version control due to copyright.
#
# Usage:       ./generate_game_data_dumps.sh
# -----------------------------------------------------------------------------

# Ensure the script generates the output in the correct directory regardless of pwd
cd "$(dirname "$0")"

echo "WARNING: To ensure correct language settings are used, please make sure you have"
echo "         launched the game at least once to generate the user settings file"
echo "         (e.g., ~/.local/share/max-port/settings.ini)."
echo "         Otherwise, default settings (English) may be used."
echo ""

# 1. Build the analysis tool to ensure it's up-to-date
echo "Building analyze_max_res tool..."
if [ ! -d "RelWithDebInfo" ]; then
    echo "Error: RelWithDebInfo directory not found. Please run recompile.sh first."
    exit 1
fi

cd RelWithDebInfo || exit 1
cmake --build . --target analyze_max_res || { echo "Build failed."; exit 1; }
cd ..

# 2. Run the tool for MAX.RES (Auto-discovery enabled)
echo "Generating dumps for MAX.RES..."
./RelWithDebInfo/test/analyze_max_res --dump

# 3. Run the tool for PATCHES.RES
echo "Generating dumps for PATCHES.RES..."
if [ -f "assets/PATCHES.RES" ]; then
    ./RelWithDebInfo/test/analyze_max_res assets/PATCHES.RES --dump
else
    echo "Warning: assets/PATCHES.RES not found."
fi

echo "Done. Files generated in doc/specs/RES_Dump/ and doc/specs/RES_TXT/."

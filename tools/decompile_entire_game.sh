#!/bin/bash
# decompile_entire_game.sh — Batch decompile all 7,698 functions from SLES_551.47 and clean the output.

set -e

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
JAVA_HOME="/usr/local/Cellar/openjdk@21/21.0.12.1/libexec/openjdk.jdk/Contents/Home"
GHIDRA_HEADLESS="/usr/local/Cellar/ghidra/12.1.3/libexec/support/analyzeHeadless"
PROJECT_DIR="$HOME/Library/ghidra/projects"
PROJECT_NAME="SHO_R5900"
PROGRAM_NAME="SLES_551.47"

OUT_RAW="$REPO_ROOT/decomp/sho_sles_out/ghidra_full"
OUT_CLEAN="$REPO_ROOT/decomp/sho_sles_out/ghidra_clean"
INDEX_FILE="$REPO_ROOT/decomp/sles_r5900_index.tsv"
KNOWN_NAMES="$REPO_ROOT/docs/generated/known_names.txt"
SLES_BIN="$REPO_ROOT/game-iso/SHO/SLES_551.47"

export JAVA_HOME
export PATH="$JAVA_HOME/bin:$PATH"

mkdir -p "$OUT_RAW" "$OUT_CLEAN"

echo "[1/3] Decompiling all functions from $PROGRAM_NAME via Ghidra..."
"$GHIDRA_HEADLESS" \
    "$PROJECT_DIR" \
    "$PROJECT_NAME" \
    -process "$PROGRAM_NAME" \
    -noanalysis \
    -readOnly \
    -scriptPath "$REPO_ROOT/tools" \
    -postScript GhidraDecompileEverything.java "$OUT_RAW" "$INDEX_FILE"

echo ""
echo "[2/3] Normalizing and cleaning C code with sho_clean_decomp.py..."
python3 "$REPO_ROOT/tools/sho_clean_decomp.py" \
    "$OUT_RAW" \
    --names "$KNOWN_NAMES" \
    --sles "$SLES_BIN" \
    --out "$OUT_CLEAN"

echo ""
echo "[3/3] Done! Total files decompiled:"
ls -1 "$OUT_CLEAN"/*.c 2>/dev/null | wc -l

echo "[+] Decompilation complete. Raw: $OUT_RAW | Clean: $OUT_CLEAN"

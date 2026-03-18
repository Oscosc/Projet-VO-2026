#!/bin/bash

EXEC="./weyl_dense_corresponding"
DATA_BASE="datasets/dense_corresponding"
RES_BASE="results/dense_corresponding"

if [ ! -f "$EXEC" ]; then
    echo "[ERREUR] Executable named '$EXEC' do not exist. Please do a 'make dense_corresponding' before."
    exit 1
fi

declare -a DATASETS=(
    "2001 barn_1   im2.ppm im6.ppm 9 60 8 true"
    "2001 barn_1   im2.ppm im6.ppm 9 60 8 false"
    "2001 barn_2   im2.ppm im6.ppm 9 60 8 true"
    "2001 barn_2   im2.ppm im6.ppm 9 60 8 false"
    "2001 bull     im2.ppm im6.ppm 9 60 8 true"
    "2001 bull     im2.ppm im6.ppm 9 60 8 false"
    "2001 poster   im2.ppm im6.ppm 9 60 8 true"
    "2001 poster   im2.ppm im6.ppm 9 60 8 false"
    "2001 sawtooth im2.ppm im6.ppm 9 60 8 true"
    "2001 sawtooth im2.ppm im6.ppm 9 60 8 false"
    "2001 venus    im2.ppm im6.ppm 9 60 8 true"
    "2001 venus    im2.ppm im6.ppm 9 60 8 false"
)

echo "=========================================================="
echo "STARTING DENSE CORRESPONDING TESTS"
echo "=========================================================="

for data in "${DATASETS[@]}"; do
    read -r YEAR SCENE L_IMG R_IMG PATCH DMAX FACTOR LRCC <<< "$data"

    IN_DIR="$DATA_BASE/$YEAR/$SCENE"
    OUT_DIR="$RES_BASE/$YEAR/$SCENE"

    mkdir -p "$OUT_DIR"

    L_PATH="$IN_DIR/$L_IMG"
    R_PATH="$IN_DIR/$R_IMG"

    L_BASE="${L_IMG%.*}"
    R_BASE="${R_IMG%.*}"
    
    L_BASE="${L_BASE/im/disp}"
    R_BASE="${R_BASE/im/disp}"

    if [ "$LRCC" = "true" ]; then
        SUFFIX="_LRCC.png"
    else
        SUFFIX="_RAW.png"
    fi

    L_OUT_PATH="$OUT_DIR/${L_BASE}${SUFFIX}"
    R_OUT_PATH="$OUT_DIR/${R_BASE}${SUFFIX}"

    echo "[TEST] Scene: $SCENE ($YEAR) | Patch: ${PATCH}x${PATCH} | Dmax: $DMAX"
    echo "  -> Command is running..."
    
    $EXEC "$L_PATH" "$R_PATH" "$PATCH" "$DMAX" "$FACTOR" "$LRCC" "$L_OUT_PATH" "$R_OUT_PATH"
    
    echo "  -> Saved in : $OUT_DIR"
    echo "----------------------------------------------------------"
done

echo "All scenes have been tested with succes !"
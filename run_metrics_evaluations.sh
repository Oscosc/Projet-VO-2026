#!/bin/bash

EXEC="./weyl_metrics"
DATA_BASE="datasets/dense_corresponding"
RES_BASE="results/dense_corresponding"

if [ ! -f "$EXEC" ]; then
    echo "[ERREUR] Executable named '$EXEC' do not exist. Please compile it before."
    exit 1
fi

declare -a DATASETS=(
    "2001 barn_1   im2.ppm im6.ppm 8"
    "2001 barn_2   im2.ppm im6.ppm 8"
    "2001 bull     im2.ppm im6.ppm 8"
    "2001 poster   im2.ppm im6.ppm 8"
    "2001 sawtooth im2.ppm im6.ppm 8"
    "2001 venus    im2.ppm im6.ppm 8"
    "2003 cones    im2.ppm im6.ppm 4"
    "2003 teddy    im2.ppm im6.ppm 4"
    "2005 Art      view1_third.png view5_third.png 3"
)

echo "=========================================================="
echo "STARTING DENSE CORRESPONDING EVALUATION"
echo "=========================================================="

for data in "${DATASETS[@]}"; do
    read -r YEAR SCENE L_IMG R_IMG DISP_COEFF <<< "$data"

    IN_DIR="$DATA_BASE/$YEAR/$SCENE"
    OUT_DIR="$RES_BASE/$YEAR/$SCENE"

    if [[ "$YEAR" == "2001" || "$YEAR" == "2003" ]]; then
        GT_EXT=".pgm"
    else
        GT_EXT=".png"
    fi

    echo "----------------------------------------------------------"
    echo "[EVAL] Scene: $SCENE ($YEAR)"

    for IMG in "$L_IMG" "$R_IMG"; do
        
        BASE="${IMG%.*}"
        BASE="${BASE/im/disp}"
        
        GT_PATH="$IN_DIR/${BASE}${GT_EXT}"

        echo "  -> Image traitée : $BASE"

        if [ ! -f "$GT_PATH" ]; then
            echo "     [ERREUR] Vérité de terrain introuvable : $GT_PATH"
            continue
        fi

        for TYPE in "RAW" "LRCC"; do
            EST_PATH="$OUT_DIR/${BASE}_${TYPE}.png"

            if [ ! -f "$EST_PATH" ]; then
                echo "     [AVERTISSEMENT] Résultat $TYPE introuvable : $EST_PATH"
                continue
            fi

            echo "     * Testing $TYPE version..."
            $EXEC "$GT_PATH" "$EST_PATH" $DISP_COEFF
        done
    done
done

echo "=========================================================="
echo "Evaluation complete!"
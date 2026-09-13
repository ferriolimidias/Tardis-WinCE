#!/usr/bin/env bash
set -euo pipefail

BMP="${1:-$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)/res/skin.bmp}"

if [ ! -f "$BMP" ]; then
    echo "BMP_VALID=NAO"
    echo "arquivo ausente: $BMP" >&2
    exit 1
fi

le16() {
    od -An -t u1 -j "$1" -N 2 "$BMP" | awk '{print $1 + 256*$2}'
}

le32() {
    od -An -t u1 -j "$1" -N 4 "$BMP" | awk '{print $1 + 256*$2 + 65536*$3 + 16777216*$4}'
}

sig=$(od -An -t x1 -N 2 "$BMP" | tr -d ' \n')
file_size=$(stat -c '%s' "$BMP")
declared_size=$(le32 2)
pixel_offset=$(le32 10)
dib_size=$(le32 14)
width=$(le32 18)
height=$(le32 22)
planes=$(le16 26)
bpp=$(le16 28)
compression=$(le32 30)
image_size=$(le32 34)

valid=SIM
if [ "$sig" != "424d" ] || [ "$width" -ne 480 ] || [ "$height" -ne 272 ] ||
   [ "$planes" -ne 1 ] || [ "$bpp" -ne 24 ] || [ "$compression" -ne 0 ] ||
   [ "$pixel_offset" -ne 54 ] || [ "$dib_size" -ne 40 ] ||
   [ "$file_size" -ne "$declared_size" ] || [ "$file_size" -ne 391734 ]; then
    valid=NAO
fi
if [ "$image_size" -ne 0 ] && [ "$image_size" -ne 391680 ]; then
    valid=NAO
fi

echo "BMP_SIGNATURE=$sig"
echo "BMP_WIDTH=$width"
echo "BMP_HEIGHT=$height"
echo "BMP_BPP=$bpp"
echo "BMP_COMPRESSION=$compression"
echo "BMP_FILE_SIZE=$file_size"
echo "BMP_IMAGE_SIZE=$image_size"
echo "BMP_VALID=$valid"

[ "$valid" = SIM ]

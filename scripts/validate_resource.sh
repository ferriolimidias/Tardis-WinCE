#!/usr/bin/env bash
set -euo pipefail

EXE="${1:?usage: validate_resource.sh Tardis.exe [objdump]}"
OBJDUMP="${2:-arm-mingw32ce-objdump}"
dump="$($OBJDUMP -p "$EXE")"

found=NAO
in_bitmap=0
while IFS= read -r line; do
    case "$line" in
        *"Entry: ID: 0x000002"*) in_bitmap=1 ;;
        *"Entry: ID: 0x0000c8"*)
            if [ "$in_bitmap" -eq 1 ]; then found=SIM; fi
            ;;
    esac
done <<< "$dump"

echo "RESOURCE_BITMAP_ID=200"
echo "RESOURCE_BITMAP_FOUND=$found"
[ "$found" = SIM ]

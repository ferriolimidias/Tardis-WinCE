#!/usr/bin/env bash
set -euo pipefail

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
OUT="$ROOT/output"
LOG="$ROOT/logs"
OBJDUMP="${OBJDUMP:-arm-mingw32ce-objdump}"
mkdir -p "$LOG"

validate_one() {
    local exe="$1"
    local name
    name="$(basename "$exe")"
    [ -s "$exe" ] || { echo "$name ausente" >&2; return 1; }
    if command -v file >/dev/null 2>&1; then
        file "$exe" | tee "$LOG/$name.file.txt"
    else
        echo "file: indisponível na imagem" | tee "$LOG/$name.file.txt"
    fi
    "$OBJDUMP" -f "$exe" | tee "$LOG/$name.objdump-f.txt"
    "$OBJDUMP" -p "$exe" | tee "$LOG/$name.objdump-p.txt"
    if ! grep -Eiq 'PE32|pei-arm|file format pei-arm|architecture: arm' "$LOG/$name.file.txt" "$LOG/$name.objdump-f.txt" "$LOG/$name.objdump-p.txt"; then
        echo "$name não foi reconhecido como PE ARM" >&2
        return 2
    fi
    if ! grep -Eiq 'architecture: arm|pei-arm' "$LOG/$name.objdump-f.txt" "$LOG/$name.file.txt"; then
        echo "$name não tem arquitetura ARM" >&2
        return 3
    fi
    if ! grep -Eiq '0x?9|Windows CE|WINCE' "$LOG/$name.objdump-p.txt"; then
        echo "$name não confirma subsystem Windows CE/9 no objdump" >&2
        return 4
    fi
}

validate_one "$OUT/hello_tardis.exe"
validate_one "$OUT/Tardis.exe"
validate_one "$OUT/serial_probe.exe"
sha256sum "$OUT/hello_tardis.exe" "$OUT/Tardis.exe" "$OUT/serial_probe.exe" | tee "$ROOT/SHA256.txt"

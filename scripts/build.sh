#!/usr/bin/env bash
set -euo pipefail

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
SRC="$ROOT/src"
OUT="$ROOT/output"
LOG="$ROOT/logs"
CC="${CC:-arm-mingw32ce-gcc}"
OBJDUMP="${OBJDUMP:-arm-mingw32ce-objdump}"
WINDRES="${WINDRES:-arm-mingw32ce-windres}"

mkdir -p "$OUT" "$LOG"
command -v "$CC" > "$LOG/compiler-path.txt"
"$CC" --version | tee "$LOG/compiler-version.txt"
"$CC" -dumpmachine | tee "$LOG/compiler-target.txt"
"$CC" -dumpspecs > "$LOG/compiler-specs.txt"
command -v "$WINDRES" > "$LOG/resource-compiler-path.txt"

COMMON_FLAGS=(-O2 -mwin32 -D_WIN32_WCE=0x0600 -D_WIN32_IE=0x0400 -DUNICODE -D_UNICODE)

probe_arch() {
    local label="$1"; shift
    printf 'int main(void){return 0;}\n' | "$CC" "${COMMON_FLAGS[@]}" "$@" -x c -c -o "$LOG/arch-probe.o" - >/dev/null 2>"$LOG/arch-probe.err"
}

ARCH_FLAGS=()
if probe_arch armv4t -march=armv4t -marm -mthumb-interwork; then
    ARCH_FLAGS=(-march=armv4t -marm -mthumb-interwork)
elif probe_arch armv4 -march=armv4 -marm; then
    ARCH_FLAGS=(-march=armv4 -marm)
else
    echo "CeGCC não aceitou nenhum alvo ARMv4/ARMv4T conservador" >&2
    cat "$LOG/arch-probe.err" >&2 || true
    exit 2
fi
printf '%s\n' "${ARCH_FLAGS[*]}" | tee "$LOG/selected-arch-flags.txt"

chmod +x "$ROOT/scripts/validate_bmp.sh"
"$ROOT/scripts/validate_bmp.sh" "$ROOT/res/skin.bmp" | tee "$LOG/skin-bmp-validation.txt"

# O alvo arm-mingw32ce fornece o startup/definições CE. Não distribuímos DLLs.
"$CC" "${COMMON_FLAGS[@]}" "${ARCH_FLAGS[@]}" \
    "$SRC/hello.c" -o "$OUT/hello_tardis.exe" \
    2>&1 | tee "$LOG/hello-build.txt"

if [ ! -s "$OUT/hello_tardis.exe" ]; then
    echo "hello_tardis.exe não foi produzido" >&2
    exit 3
fi

"$WINDRES" "$ROOT/res/skin.rc" -O coff -o "$LOG/skin.o"

"$CC" "${COMMON_FLAGS[@]}" "${ARCH_FLAGS[@]}" \
    "$SRC/main.c" "$LOG/skin.o" -o "$OUT/Tardis.exe" \
    2>&1 | tee "$LOG/tardis-build.txt"

if [ ! -s "$OUT/Tardis.exe" ]; then
    echo "Tardis.exe não foi produzido" >&2
    exit 4
fi

for exe in "$OUT/hello_tardis.exe" "$OUT/Tardis.exe"; do
    base="$(basename "$exe")"
    if command -v file >/dev/null 2>&1; then
        file "$exe" | tee "$LOG/$base.file.txt"
    else
        echo "file: indisponível na imagem" | tee "$LOG/$base.file.txt"
    fi
    "$OBJDUMP" -f "$exe" | tee "$LOG/$base.objdump-f.txt"
    "$OBJDUMP" -p "$exe" | tee "$LOG/$base.objdump-p.txt"
done

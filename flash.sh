#!/usr/bin/env bash
set -euo pipefail

SKETCH="$(dirname "$0")/Arduino/EBBTelescopeFocuser"
FQBN="STMicroelectronics:stm32:3dprinter:pnum=EBB42_V1_1,upload_method=dfuMethod,usb=CDCgen"
#STM32CP_BIN="~/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin"
DFU_VID="0483"
DFU_PID="df11"

#export PATH="$STM32CP_BIN:$PATH"

# ── helpers ────────────────────────────────────────────────────────────────

die()  { echo "ERROR: $*" >&2; exit 1; }
info() { echo "[+] $*"; }

board_in_dfu() {
    lsusb | grep -qi "${DFU_VID}:${DFU_PID}"
}

wait_for_dfu() {
    info "Waiting for board in DFU mode (${DFU_VID}:${DFU_PID}) ..."
    local dots=0
    while ! board_in_dfu; do
        printf "."
        (( dots++ ))
        sleep 1
    done
    [[ $dots -gt 0 ]] && echo
    info "Board detected in DFU mode."
}

# ── preflight ──────────────────────────────────────────────────────────────

command -v arduino-cli   >/dev/null || die "arduino-cli not found"
command -v STM32_Programmer.sh >/dev/null || die "STM32_Programmer.sh not found"
[[ -d "$SKETCH" ]]       || die "Sketch not found: $SKETCH"

# ── compile ────────────────────────────────────────────────────────────────

info "Compiling $SKETCH ..."
arduino-cli compile \
    --fqbn "$FQBN" \
    "$SKETCH"
info "Compilation successful."

# ── wait for DFU + upload ─────────────────────────────────────────────────

wait_for_dfu

info "Uploading firmware ..."
arduino-cli upload \
    --fqbn "$FQBN" \
    "$SKETCH"

info "Done. Unplug and replug the board — it will appear as /dev/ttyACM*."

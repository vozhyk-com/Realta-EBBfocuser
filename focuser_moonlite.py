#!/usr/bin/env python3
"""Interactive controller for EBB36 telescope focuser — Moonlite protocol."""

import sys
import time
import glob
import threading
import readline  # noqa: F401 — enables UP/DOWN history in input()
import serial

PORT_DEFAULT = "/dev/ttyACM0"
BAUD = 9600


class Focuser:
    def __init__(self, port):
        # pyserial asserts DTR on open, which STM32 USB CDC requires
        self._ser = serial.Serial(port, BAUD, timeout=2)
        self._lock = threading.Lock()
        time.sleep(1.0)   # let CDC enumerate and firmware settle
        self._ser.reset_input_buffer()

    def close(self):
        self._ser.close()

    def _query(self, cmd):
        with self._lock:
            self._ser.reset_input_buffer()
            self._ser.write(cmd.encode())
            time.sleep(0.3)
            resp = self._ser.read_until(b"#", size=32)
            return resp.decode(errors="replace").strip().rstrip("#")

    def _send(self, cmd):
        with self._lock:
            self._ser.write(cmd.encode())
        time.sleep(0.1)

    # ── queries ───────────────────────────────────────────────────────────────

    def version(self):
        return self._query(":GV#")

    def position(self):
        raw = self._query(":GP#")
        return int(raw, 16) if raw else 0

    def is_moving(self):
        return self._query(":GI#") == "01"

    def temperature(self):
        raw = self._query(":GT#")
        if not raw:
            return 0.0
        val = int(raw, 16)
        if val > 0x7FFF:        # signed 16-bit (negative temperatures)
            val -= 0x10000
        return val / 2.0

    def step_delay(self):
        raw = self._query(":GD#")
        return int(raw, 16) if raw else 0

    def status(self):
        sg, cs = self.sg_result()
        return {
            "position":        self.position(),
            "moving":          self.is_moving(),
            "temp_c":          self.temperature(),
            "step_delay":      self.step_delay(),
            "step_mode":       self.step_mode(),
            "stall_threshold": self.stall_threshold(),
            "sg_result":       sg,
            "cs_actual":       cs,
        }

    # ── move commands ─────────────────────────────────────────────────────────

    def move_to(self, pos):
        """Set target and start moving."""
        pos = max(0, min(0xFFFFFF, int(pos)))
        self._send(f":SN{pos:06X}#")
        self._send(":FG#")

    def move_by(self, delta):
        self.move_to(self.position() + int(delta))

    def halt(self):
        self._send(":FQ#")

    # ── position sync ─────────────────────────────────────────────────────────

    def sync_position(self, pos):
        """Set current position without moving (tells driver where we are)."""
        pos = max(0, min(0xFFFFFF, int(pos)))
        self._send(f":SP{pos:06X}#")

    # ── settings ──────────────────────────────────────────────────────────────

    def set_step_delay(self, val):
        """Step delay 1 (slow) … 32 (fast), saved to EEPROM."""
        val = max(1, min(32, int(val)))
        self._send(f":SD{val:02X}#")

    def step_mode(self):
        """Return 'half' or 'full'."""
        raw = self._query(":GH#")
        return "half" if raw == "FF" else "full"

    def set_half_step(self):
        """Switch to half-step mode (1 microstep per logical step). Saved to EEPROM."""
        self._send(":SH#")

    def set_full_step(self):
        """Switch to full-step mode (microstep-count pulses per logical step). Saved to EEPROM."""
        self._send(":SF#")

    # ── StallGuard ────────────────────────────────────────────────────────────

    def sg_result(self):
        """Read live sg_result (0-510) and cs_actual (0-31) for stall threshold tuning.
        sg_result drops toward 0 when motor stalls; cs_actual is static (reflects IRUN)."""
        raw = self._query(":GR#")
        if not raw:
            return 0, 0
        parts = raw.split(",")
        sg = int(parts[0], 16) if parts[0] else 0
        cs = int(parts[1], 16) if len(parts) > 1 and parts[1] else 0
        return sg, cs

    def stall_threshold(self):
        """sg_result threshold (0=disabled, 1-510=halt when sg_result ≤ this; higher = more sensitive)."""
        raw = self._query(":GK#")
        return int(raw, 16) if raw else 0

    def set_stall_threshold(self, val):
        """Set sg_result stall threshold (0-510) and save to EEPROM. 0 disables.
        Higher = more sensitive (triggers on lighter load). Start around 100-200.
        Only active at step delay ≥ 8 — StallGuard4 can't measure load at low speed."""
        val = max(0, min(510, int(val)))
        self._send(f":SK{val:03X}#")

    def stall_status(self):
        """True if the motor stalled since the last call (clears the flag on the board)."""
        return self._query(":GE#") == "01"

    # ── wait helper ───────────────────────────────────────────────────────────

    def wait_done(self, poll=0.4):
        while self.is_moving():
            pos = self.position()
            print(f"  moving ... {pos} (0x{pos:06X})", end="\r", flush=True)
            time.sleep(poll)
        print()


# ── CLI helpers ───────────────────────────────────────────────────────────────

def find_port():
    candidates = sorted(glob.glob("/dev/ttyACM*") + glob.glob("/dev/ttyUSB*"))
    return candidates[0] if candidates else PORT_DEFAULT

def print_status(s):
    pos = s["position"]
    thr = s["stall_threshold"]
    if thr == 0:
        stall_line = "disabled"
    elif s["step_delay"] >= 8:
        stall_line = f"{thr}  (active — halts when sg_result ≤ {thr})"
    else:
        stall_line = f"{thr}  (INACTIVE — needs step delay ≥ 8; StallGuard can't read load at low speed)"
    print(f"""
  Position   : {pos}  (0x{pos:06X})
  Moving     : {'yes' if s['moving'] else 'no'}
  Temperature: {s['temp_c']:.1f} °C
  Step delay : {s['step_delay']}  (1=slow, 32=fast)
  Step mode  : {s['step_mode']}
  sg_result  : {s['sg_result']}  cs_actual: {s['cs_actual']}
  Stall thr  : {stall_line}
""")

MENU = """
  Commands
  ────────────────────────────────────────
  s            status
  m <pos>      move to absolute position
  r <±n>       move relative by n steps
  h            halt
  p <pos>      sync position (no move)
  d <1-32>     set step delay (speed)
  w            wait until move finishes
  hs           switch to half-step mode
  fs           switch to full-step mode
  cr           read live sg_result (0-510) for stall threshold tuning
  k            get stall threshold
  sk <0-510>   set stall threshold (0=off; higher=more sensitive; start ~100-200; needs step delay ≥ 8)
  e            check stall flag (clears it)
  q            quit
  ────────────────────────────────────────
"""

def main():
    port = sys.argv[1] if len(sys.argv) > 1 else find_port()
    print(f"Connecting to {port} (Moonlite protocol) ...")
    try:
        f = Focuser(port)
    except OSError as e:
        print(f"Error: {e}")
        sys.exit(1)

    ver = f.version()
    print(f"Connected. Firmware version: {ver}")
    print_status(f.status())
    print(MENU)

    try:
        while True:
            try:
                line = input("focuser> ").strip()
            except (EOFError, KeyboardInterrupt):
                print()
                break

            if not line:
                continue

            parts = line.split(None, 1)
            cmd = parts[0].lower()
            arg = parts[1] if len(parts) > 1 else ""

            if cmd == "q":
                break
            elif cmd == "s":
                print_status(f.status())
            elif cmd == "m":
                if not arg:
                    print("  Usage: m <position>  (0 – 16777215)")
                    continue
                pos = int(arg)
                f.move_to(pos)
                print(f"  Moving to {pos} (0x{pos:06X}) ...")
            elif cmd == "r":
                if not arg:
                    print("  Usage: r <±steps>")
                    continue
                delta = int(arg)
                cur = f.position()
                target = cur + delta
                f.move_to(target)
                print(f"  Moving {delta:+d}  →  target {target} (0x{target:06X})")
            elif cmd == "h":
                f.halt()
                pos = f.position()
                print(f"  Halted at {pos} (0x{pos:06X})")
            elif cmd == "p":
                if not arg:
                    print("  Usage: p <position>")
                    continue
                pos = int(arg)
                f.sync_position(pos)
                print(f"  Position synced to {pos} (0x{pos:06X})")
            elif cmd == "d":
                if not arg:
                    print("  Usage: d <1-32>")
                    continue
                val = int(arg)
                f.set_step_delay(val)
                print(f"  Step delay set to {val}")
            elif cmd == "w":
                print("  Waiting for move to finish ...")
                f.wait_done()
                pos = f.position()
                print(f"  Done. Position: {pos} (0x{pos:06X})")
            elif cmd == "hs":
                f.set_half_step()
                print("  Switched to half-step mode (saved to EEPROM)")
            elif cmd == "fs":
                f.set_full_step()
                print("  Switched to full-step mode (saved to EEPROM)")
            elif cmd == "cr":
                sg, cs = f.sg_result()
                print(f"  sg_result = {sg:3d} / 510   cs_actual = {cs} / 31"
                      f"  {'(StallGuard active)' if sg > 0 else '(sg_result=0: TCOOLTHRS not set or motor stopped)'}")
            elif cmd == "k":
                val = f.stall_threshold()
                if val == 0:
                    print("  Stall detection disabled")
                else:
                    print(f"  Stall threshold: {val}  (halts when sg_result ≤ {val} for 3 samples)")
            elif cmd == "sk":
                if not arg:
                    print("  Usage: sk <0-510>  (0=disabled; lower=more sensitive; start ~100-200)")
                    continue
                val = int(arg)
                f.set_stall_threshold(val)
                if val == 0:
                    print("  Stall detection disabled")
                else:
                    print(f"  Stall threshold set to {val}  (halts when sg_result ≤ {val})")
            elif cmd == "e":
                stalled = f.stall_status()
                print(f"  Stall flag: {'STALLED — motor was blocked' if stalled else 'OK (no stall detected)'}")
            else:
                print(f"  Unknown command '{cmd}'. Type 'q' to quit.")

    finally:
        f.close()
        print("Disconnected.")


if __name__ == "__main__":
    main()

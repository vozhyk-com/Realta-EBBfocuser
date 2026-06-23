# Arduino IDE firmware, C++

## Overview

The firmware implements the **Moonlite focuser protocol** so the focuser works with
the standard INDI MoonLite driver on Linux (and any other Moonlite-compatible client)
without a custom driver. It incorporates a non-blocking serial port listener designed to
await and process incoming commands efficiently. Commands are handled in a non-blocking
manner so the motor's movement can be halted at any time.

Leveraging the TMCStepper library by teemuatlut, our solution extends the base class and
introduces methods for precise motor control and regulation of the optional dew-heater
element. The serial parser (`SerialComms.h`) collects everything between a `:` and a `#`
into a buffer; `processMoonlite()` then takes the first two characters as the command
code and the remainder as hex parameters and dispatches the matching action.

## Protocol framing

* A command is `:` + a two-letter code + optional hex parameters + `#`, e.g. `:SN00C350#`.
* Numeric parameters and return values are **hexadecimal**, no `0x` prefix.
* Every query response is terminated with `#`.
* Positions use 6 hex digits (`GP`/`SN`/`SP`), supporting up to `FFFFFF` = 16,777,215 steps.
* Temperature is returned as the Moonlite raw value: **°C × 2**, signed 16-bit, as 4 hex
  digits (so `0x0030` = 48 → 24.0 °C).

## List of accepted commands

These are the standard Moonlite commands implemented by the firmware:

|Code|Description|Parameter|Example|Return value|
|----|-----------|---------|-------|------------|
|`GV`|Get firmware version|No|`:GV#`|`10#`|
|`GP`|Get current position (6 hex digits)|No|`:GP#`|`00C350#` (= 50000)|
|`GI`|Is the motor moving?|No|`:GI#`|`01#` (yes) or `00#` (no)|
|`GT`|Get temperature (Moonlite raw, °C × 2)|No|`:GT#`|`0030#` (= 24.0 °C)|
|`GD`|Get step delay (speed), 1–32|No|`:GD#`|`02#`|
|`GC`|Get current motor speed (same as step delay)|No|`:GC#`|`02#`|
|`GH`|Get step mode|No|`:GH#`|`FF#` (half step) or `00#` (full step)|
|`SN`|Set target position (does **not** start the move)|Yes, 6 hex digits|`:SN00C350#`|None|
|`FG`|Focus Go — start moving to the target set by `SN`|No|`:FG#`|None|
|`FQ`|Focus Quit — halt the motor and save position to EEPROM|No|`:FQ#`|None|
|`SP`|Set current position without moving (sync)|Yes, 6 hex digits|`:SP00C350#`|None|
|`SD`|Set step delay (speed), 1–32 (hex)|Yes|`:SD02#`|None|
|`SF`|Set full-step mode|No|`:SF#`|None|
|`SH`|Set half-step mode|No|`:SH#`|None|

### Custom stall-detection extensions

In addition to the standard protocol, the firmware adds a few commands for tuning the
TMC2209 StallGuard-based stall detection. These are used by `focuser_moonlite.py` and are
ignored by the standard INDI driver.

|Code|Description|Parameter|Example|Return value|
|----|-----------|---------|-------|------------|
|`GR`|Diagnostic: `sg_result` (0–510) and `cs_actual` (0–31)|No|`:GR#`|`1F4,10#`|
|`GK`|Get stall threshold (0 = disabled, 1–510)|No|`:GK#`|`0000#`|
|`SK`|Set stall threshold (0–510, halt when `sg_result` ≤ value)|Yes|`:SK0064#`|None|
|`GE`|Get stall-detected flag (clears on read)|No|`:GE#`|`01#` or `00#`|

Commands `SC` (temperature calibration / backlash) and `PO` are accepted but currently
ignored.

## Settings storage

Unlike the old custom protocol, motor settings persist in the board's EEPROM and survive
power cycles. The layout is:

|Address|Contents|Default|
|-------|--------|-------|
|0|First-run flag|`6`|
|1–4|Current position (long)|50000|
|10–11|Motor current (mA)|500|
|20–21|Microsteps (half-step mode)|8|
|30|Step delay (Moonlite units)|2|
|31|Step mode (0 = full, 1 = half)|1|
|32–33|Stall threshold (`sg_result`, 0 = disabled)|0|

On first boot (or after the flag is cleared) the defaults above are written automatically.

## Future plans/changes

The code doesn't use any of the common C++ style naming conventions, its my first C++
program so only discovered them at the end of creating this project. For example some
conventions use underscores in member names i.e. memberName_ is a private member of a
class.

There is also no error checking on the text being sent between the PC and the
microcontroller. The Moonlite protocol has no checksum, but extremely long strings sent
over the serial connection are bounded by the parser's fixed buffer, so the
microcontroller no longer risks running out of RAM.

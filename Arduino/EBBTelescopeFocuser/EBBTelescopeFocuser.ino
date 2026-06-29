#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <math.h>
#include "MotorClass.h"
#include "SerialComms.h"

#define D_STEP_PIN      PD0
#define D_UART_PIN      PA15
#define D_ENABLE_PIN    PD2
#define D_HEATER_PIN    PB13
#define D_THERM_PIN     PA3       // TH0 header: 4.7 kΩ pull-up to 3.3V, NTC to GND
#define D_DRIVER_ADDRESS 0b00
#define D_R_SENSE       0.11f

// Motor: NEMA 17 17HS4023 (1.8°, 200 steps/rev)
//   Rated current : 0.7 A/phase   Phase resistance : ~4 Ω   Inductance : ~3.2 mH/phase
//   Rated voltage : 0.7 A × 4 Ω ≈ 2.8 V  (informational only — the TMC2209 is a
//                   current-chopper driver, so motor voltage is not programmed; the
//                   coil voltage is regulated automatically from the EBB36 bus supply.)
// rms_current() takes the RMS coil current; peak phase current = RMS × √2. To keep the
// peak at the 0.7 A rating we run 700 / √2 ≈ 495 mA RMS (rounded to 500). This is plenty
// of torque for a focuser and keeps the motor cool near the optics.
#define D_MOTOR_RMS_MA  500

// NTC 100 kΩ @ 25 °C, β = 3950, pull-up 4.7 kΩ
#define NTC_BETA        3950.0f
#define NTC_R0          100000.0f
#define NTC_T0_K        298.15f
#define NTC_R_PULLUP    4700.0f

// EEPROM layout:
//   0        : first-run flag (6)
//   1 .. 4   : CurrentPosition (long)
//   10 .. 11 : motor current mA (int)
//   20 .. 21 : microsteps for half-step mode (int)
//   30       : step delay (byte, Moonlite units; default 2)
//   31       : step mode (byte, 0=full step / 1 microstep, 1=half step / MyMotor.steps; default 1)
//   32 .. 33 : sg_result stall threshold (uint16_t, 0=disabled, 1-510=halt when sg_result ≤ this)

// Returns temperature as Moonlite raw (°C × 2, signed 16-bit).
static int16_t readTempMoonlite()
{
  long sum = 0;
  for (int i = 0; i < 8; i++) sum += analogRead(D_THERM_PIN);
  float adc = sum / 8.0f;
  if (adc <= 0 || adc >= 4095) return 0;
  float r   = NTC_R_PULLUP * adc / (4095.0f - adc);
  float t_k = 1.0f / (1.0f / NTC_T0_K + logf(r / NTC_R0) / NTC_BETA);
  return (int16_t)((t_k - 273.15f) * 2.0f);
}

SoftwareSerial G_TMC_SERIAL(D_UART_PIN, D_UART_PIN);
Motor MyMotor(&G_TMC_SERIAL, D_R_SENSE, D_DRIVER_ADDRESS);

static byte stepDelay = 2;   // Moonlite :GD# / :SD# value (1-32)
static bool halfStep  = true; // true = MyMotor.steps microsteps, false = 1 microstep (full)

// ── Moonlite command dispatch ────────────────────────────────────────────────

void processMoonlite()
{
  if (!ML_READY) return;
  ML_READY = false;

  // ML_BUF contains everything between ':' and '#'
  // First two chars = command code, rest = parameters (hex digits)
  char  cmd[3]  = { ML_BUF[0], ML_BUF[1], '\0' };
  char *params  = ML_BUF + 2;
  char  resp[12];

  // ── queries ────────────────────────────────────────────────────────────
  if (strcmp(cmd, "GV") == 0)
  {
    // Firmware version
    Serial.print("10#");
  }
  else if (strcmp(cmd, "GP") == 0)
  {
    // Current position (6 hex digits, supports up to 16 777 215)
    snprintf(resp, sizeof(resp), "%06lX#", (unsigned long)MyMotor.CurrentPosition);
    Serial.print(resp);
  }
  else if (strcmp(cmd, "GI") == 0)
  {
    // Is moving?
    Serial.print(MyMotor.IsMoving ? "01#" : "00#");
  }
  else if (strcmp(cmd, "GT") == 0)
  {
    // Temperature from NTC on PA3 (Moonlite: raw = °C × 2, signed 16-bit)
    int16_t t = readTempMoonlite();
    snprintf(resp, sizeof(resp), "%04X#", (uint16_t)t);
    Serial.print(resp);
  }
  else if (strcmp(cmd, "GD") == 0)
  {
    // Step delay
    snprintf(resp, sizeof(resp), "%02X#", stepDelay);
    Serial.print(resp);
  }
  else if (strcmp(cmd, "GC") == 0)
  {
    // Current motor speed (same as step delay for Moonlite)
    snprintf(resp, sizeof(resp), "%02X#", stepDelay);
    Serial.print(resp);
  }
  else if (strcmp(cmd, "GH") == 0)
  {
    // Step mode: FF = half step, 00 = full step
    Serial.print(halfStep ? "FF#" : "00#");
  }

  // ── move commands ──────────────────────────────────────────────────────
  else if (strcmp(cmd, "SN") == 0)
  {
    // Set new target position (does NOT start the move)
    MyMotor.MoveTarget = (long)strtoul(params, NULL, 16);
  }
  else if (strcmp(cmd, "FG") == 0)
  {
    // Focus Go — start moving to previously set target
    if (MyMotor.CurrentPosition != MyMotor.MoveTarget)
    {
      MyMotor.stallDetected = false;
      MyMotor.stallCount    = 0;
      MyMotor.stallWarmup   = 0;
      // Stay in StealthChop: StallGuard4 (SG_RESULT) is only valid in StealthChop,
      // not spreadCycle. StealthChop is configured in setup() and never switched away.
      MyMotor.IsMoving      = true;
    }
  }
  else if (strcmp(cmd, "FQ") == 0)
  {
    // Focus Quit — halt and save position to EEPROM
    MyMotor.Halt();
  }

  // ── position sync ──────────────────────────────────────────────────────
  else if (strcmp(cmd, "SP") == 0)
  {
    // Set current position without moving
    MyMotor.CurrentPosition = (long)strtoul(params, NULL, 16);
    MyMotor.MoveTarget      = MyMotor.CurrentPosition;
    EEPROM.put(1, MyMotor.CurrentPosition);
  }

  // ── settings ───────────────────────────────────────────────────────────
  else if (strcmp(cmd, "SD") == 0)
  {
    // Set step delay (1-32 in hex)
    byte v = (byte)strtol(params, NULL, 16);
    if (v > 0 && v <= 32)
    {
      stepDelay = v;
      MyMotor.moveDelayMs = (uint16_t)(32 / stepDelay);
      EEPROM.put(30, stepDelay);
    }
  }
  else if (strcmp(cmd, "SF") == 0)
  {
    // Full step: MyMotor.steps physical pulses per logical step = 1 mechanical full step
    halfStep = false;
    MyMotor.stepsPerLogical = (byte)MyMotor.steps;
    EEPROM.put(31, (byte)0);
  }
  else if (strcmp(cmd, "SH") == 0)
  {
    // Half step: 1 physical pulse per logical step = 1 microstep
    halfStep = true;
    MyMotor.stepsPerLogical = 1;
    EEPROM.put(31, (byte)1);
  }
  // ── stall detection (sg_result threshold) ─────────────────────────────
  else if (strcmp(cmd, "GR") == 0)
  {
    // Diagnostic: sg_result (0-510) and cs_actual (0-31) for stall threshold tuning.
    // sg_result drops toward 0 when stalled; cs_actual is static (reflects IRUN setting).
    // 0x000 for sg_result when TCOOLTHRS is not set = StallGuard disabled.
    char dbg[16];
    snprintf(dbg, sizeof(dbg), "%03X,%02X#",
             (unsigned int)MyMotor.SG_RESULT(),
             (unsigned int)MyMotor.cs_actual());
    Serial.print(dbg);
  }
  else if (strcmp(cmd, "GK") == 0)
  {
    // Get sg_result stall threshold (0 = disabled, 1-510 = halt when sg_result ≤ this)
    snprintf(resp, sizeof(resp), "%04X#", MyMotor.stallThreshold);
    Serial.print(resp);
  }
  else if (strcmp(cmd, "SK") == 0)
  {
    // Set sg_result stall threshold (0-510) and save to EEPROM.
    // Higher = more sensitive (triggers sooner). 0 = disabled.
    uint16_t v = (uint16_t)strtoul(params, NULL, 16);
    if (v > 510) v = 510;
    MyMotor.stallThreshold = v;
    EEPROM.put(32, v);
  }
  else if (strcmp(cmd, "GE") == 0)
  {
    // Get stall-detected flag (clears on read)
    Serial.print(MyMotor.stallDetected ? "01#" : "00#");
    MyMotor.stallDetected = false;
  }

  else if (strcmp(cmd, "SC") == 0 || strcmp(cmd, "PO") == 0)
  {
    // Calibration / backlash — not implemented, ignore
  }
}

// ── Arduino setup / loop ─────────────────────────────────────────────────────

void setup()
{
  byte isFirstRun;
  EEPROM.get(0, isFirstRun);

  if (isFirstRun != 6)
  {
    EEPROM.put(1,  (long)50000);  // CurrentPosition
    EEPROM.put(10, (int)D_MOTOR_RMS_MA);  // motor RMS current mA (17HS4023: 0.7 A peak)
    EEPROM.put(20, (int)8);       // microsteps (half-step mode)
    EEPROM.put(30, (byte)2);      // step delay
    EEPROM.put(31, (byte)1);      // step mode: half step
    EEPROM.put(32, (uint16_t)0);  // sg_result stall threshold: disabled
    EEPROM.put(0,  (byte)6);
  }

  EEPROM.get(1,  MyMotor.CurrentPosition);
  EEPROM.get(10, MyMotor.current);
  EEPROM.get(20, MyMotor.steps);
  EEPROM.get(30, stepDelay);
  if (stepDelay == 0 || stepDelay > 32)
  {
    stepDelay = 2;
    EEPROM.put(30, stepDelay);
  }

  byte savedMode;
  EEPROM.get(31, savedMode);
  halfStep = (savedMode != 0);
  MyMotor.stepsPerLogical = halfStep ? 1 : (byte)MyMotor.steps;
  MyMotor.moveDelayMs     = (uint16_t)(32 / stepDelay);

  EEPROM.get(32, MyMotor.stallThreshold);
  // Sanitize: 0xFFFF = unwritten; >510 = out of sg_result range → disable
  if (MyMotor.stallThreshold > 510)
  {
    MyMotor.stallThreshold = 0;
    EEPROM.put(32, (uint16_t)0);
  }

  MyMotor.MoveTarget = MyMotor.CurrentPosition;
  MyMotor.IsMoving   = false;

  pinMode(D_STEP_PIN,   OUTPUT);
  pinMode(D_ENABLE_PIN, OUTPUT);
  pinMode(D_THERM_PIN,  INPUT_ANALOG);
  analogReadResolution(12);

  MyMotor.engageMotor(true);
  MyMotor.setHeaterPWM(0);

  Serial.begin(9600);

  // 19200 baud: TMC2209 8-byte response = 4.17 ms < 5 ms abort_window, and the
  // 57600 Hz timer ISR is slow enough that USB CDC is never starved.
  // Do NOT call listen() before writes — it deadlocks on STM32G0B1 (timer ISR
  // at write()'s setSpeed() frequency never fires when pre-armed by listen()).
  // Instead, write() starts the timer itself; we kill it with listen/stopListening
  // after all init writes finish so the timer does not run between operations.
  _g_tmc_serial = &G_TMC_SERIAL;
  G_TMC_SERIAL.begin(19200);
  MyMotor.beginSerial(19200);
  MyMotor.begin();
  MyMotor.toff(5);
  MyMotor.rms_current(MyMotor.current);  // RMS mA; 500 ≈ 0.7 A peak (17HS4023 rated)
  MyMotor.microsteps(MyMotor.steps);
  MyMotor.en_spreadCycle(false);
  MyMotor.pwm_autoscale(true);
  // Enable StallGuard4 at all motor speeds (TSTEP ≥ 1 satisfies the threshold).
  // Without this, sg_result reads 0 regardless of load and stall detection never fires.
  MyMotor.TCOOLTHRS(0xFFFFF);
  // Stop the SoftwareSerial timer that write() left running.
  G_TMC_SERIAL.listen();
  G_TMC_SERIAL.stopListening();
}

void loop()
{
  P_PROCESS_SERIAL_PORT();
  processMoonlite();

  if (MyMotor.IsMoving)
    MyMotor.Move();
}

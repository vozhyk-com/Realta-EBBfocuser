#include <EEPROM.h>;
#include <SoftwareSerial.h>
#include <TMCStepper.h>

#define D_STEP_PIN PD0
#define D_UART_PIN PA15
#define D_ENABLE_PIN PD2
#define D_HEATER_PIN PB13

// Pointer to the SoftwareSerial used for TMC UART — set in setup() before begin().
// preReadCommunication() calls listen() so the STM32 timer-based SoftwareSerial
// automatically switches PA15 from OUTPUT (TX) to INPUT_PULLUP (RX) after TX completes.
static SoftwareSerial * _g_tmc_serial = nullptr;

// Override the weak TMCStepper read hooks to enable half-duplex RX on STM32 SoftwareSerial.
//
// On STM32, SoftwareSerial uses a hardware timer (TIM7) for bit-banging.  The ISR
// auto-switches PA15 from OUTPUT (TX) to INPUT_PULLUP (RX) after the last stop bit
// + HALFDUPLEX_SWITCH_DELAY — but only if active_listener == this, which listen() sets.
//
// The default preReadCommunication() only calls listen() when SWSerial != nullptr (the
// SW_RX/TX constructor path).  We use the HWSerial/Stream path, so SWSerial is nullptr
// and the switch never happens — PA15 stays OUTPUT and TMC2209 can't respond.
//
// Fix: override the weak symbols so preReadCommunication() calls listen() explicitly.
// postReadCommunication() calls stopListening() to kill the timer (keeps USB healthy).
//
// Writes are intentionally NOT bracketed with listen/stopListening here because calling
// listen() before write() on STM32G0B1 causes a deadlock: listen() arms the timer ISR
// as active_listener, but at 19200 baud × OVERSAMPLE=3 = 57600 Hz the ISR fires
// reliably and clears active_out normally — this is handled by write()'s own setSpeed().
// The timer is stopped in setup() with an explicit listen/stopListening pair after all
// motor-init writes complete.
void TMC2208Stepper::preReadCommunication() {
  if (_g_tmc_serial != nullptr) _g_tmc_serial->listen();
}
void TMC2208Stepper::postReadCommunication() {
  if (_g_tmc_serial != nullptr) _g_tmc_serial->stopListening();
}


class Motor: public TMC2209Stepper
{
   public:
      Motor(Stream * SerialPort, float RS, uint8_t addr);
      void Halt();
      void SetMoveTarget(long Position);
      boolean Move();
      void setHeaterPWM(byte HeaterValue);
      void engageMotor(boolean Engage);
      byte CurrentHeaterValue;
      boolean IsMoving;
      boolean IsEngaged;
      long CurrentPosition;
      long MoveTarget;
      boolean move_direction;
      int current;
      int steps;
      byte stepsPerLogical;    // physical pulses per logical step: 1=half-step, steps=full-step
      uint16_t moveDelayMs;    // extra delay per logical step (Moonlite :SD# value − 1)
      uint16_t stallThreshold; // SG_RESULT halt level (1-510); 0 = disabled; stall when SG_RESULT <= this
      bool stallDetected;      // set when stall halts the motor; cleared by :GE#
      byte stallCount;         // consecutive below-threshold sg_result readings
      uint16_t stallWarmup;    // steps taken in current move; detection skipped until > STALL_WARMUP
};

// SG_RESULT() reads the TMC2209 SG_RESULT register (0-510): StallGuard4 load indicator.
// It drops toward 0 when the motor stalls; halt when it falls at or below stallThreshold.
// StallGuard4 is only valid in StealthChop and only above a minimum step rate (it reads
// back-EMF), so detection is gated to fast moves and samples sparingly — each SG_RESULT()
// is a blocking ~6 ms UART read that pauses the step train.
static const uint16_t STALL_WARMUP            = 50;   // logical steps before sampling starts
static const uint8_t  STALL_SAMPLE_EVERY      = 32;   // sample 1 step in N (fewer UART reads = smoother)
static const uint8_t  STALL_CONFIRM           = 3;    // consecutive low samples required to halt
static const uint16_t STALL_MAX_MOVE_DELAY_MS = 4;    // detection skipped at slower speeds (StallGuard invalid)

Motor::Motor(Stream * SerialPort, float RS, uint8_t addr): TMC2209Stepper(SerialPort, RS, addr)
{
  stepsPerLogical = 1;
  moveDelayMs     = 0;
  stallThreshold  = 0;
  stallDetected   = false;
  stallCount      = 0;
  stallWarmup     = 0;
}


void Motor::Halt()
{
     IsMoving    = false;
     stallCount  = 0;
     stallWarmup = 0;
     // StealthChop stays configured from setup() — no chopper-mode switching here.
     EEPROM.put(1, CurrentPosition);
}

void Motor::SetMoveTarget(long Position)
{
  if(CurrentPosition != Position)
  {
     IsMoving = true;
     MoveTarget = Position;
  }
}

boolean Motor::Move()
{
          long difference;
          boolean new_direction;
          difference = MoveTarget - CurrentPosition;

          if(!IsEngaged)
          {
            engageMotor(true);
          }

          if(difference == 0)
          {
            Halt();
            return false;
          }
          else
          {
            if(difference > 0)
            {
              new_direction = true;
              CurrentPosition = CurrentPosition + 1;
            }
            else
            {
              new_direction = false;
              CurrentPosition = CurrentPosition - 1;
            }

            if(new_direction != move_direction)
            {
              move_direction = new_direction;
              shaft(move_direction);
            }

            for (byte p = 0; p < stepsPerLogical; p++)
            {
              digitalWrite(D_STEP_PIN, HIGH);
              delayMicroseconds(120);
              digitalWrite(D_STEP_PIN, LOW);
              delayMicroseconds(120);
            }
            if (moveDelayMs > 0) delay(moveDelayMs);

            // StallGuard4 stall detection. Only runs on fast moves: SG_RESULT is
            // meaningless at low step rates, and skipping it there also avoids the
            // micro-stops caused by the blocking UART read. Requires StealthChop
            // (set in setup, never switched away) + TCOOLTHRS.
            if (stallThreshold > 0 && moveDelayMs <= STALL_MAX_MOVE_DELAY_MS)
            {
              stallWarmup++;
              if (stallWarmup > STALL_WARMUP && (stallWarmup % STALL_SAMPLE_EVERY) == 0)
              {
                uint16_t sg = SG_RESULT();
                // sg==0 means "no valid reading" (too slow / CRC fail), not a stall:
                // a real stall drops sg toward the threshold, which sits well above 0.
                // Ignoring 0 prevents false halts from glitched reads.
                if (sg > 0 && sg <= stallThreshold)
                {
                  if (++stallCount >= STALL_CONFIRM)
                  {
                    stallDetected = true;
                    Halt();
                    return false;
                  }
                }
                else
                {
                  stallCount = 0;
                }
              }
            }

            return true;

          }
}

void Motor::setHeaterPWM(byte HeaterValue)
{
  analogWrite(D_HEATER_PIN, HeaterValue);
  CurrentHeaterValue = HeaterValue;
}

void Motor::engageMotor(boolean Engage)
{
  IsEngaged = Engage;
  if(Engage)
  {
  digitalWrite(D_ENABLE_PIN, LOW);
  }
  else
  {
  digitalWrite(D_ENABLE_PIN, HIGH);
  }
}

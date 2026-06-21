// Moonlite focuser protocol parser.
// Collects characters between ':' and '#' into ML_BUF, then sets ML_READY.

static char    ML_BUF[16];
static uint8_t ML_LEN;
static bool    ML_READY;
static bool    ML_IN_CMD;

void P_PROCESS_SERIAL_PORT()
{
  while (Serial.available())
  {
    char c = (char)Serial.read();

    if (c == ':')
    {
      ML_IN_CMD = true;
      ML_LEN    = 0;
    }
    else if (c == '#' && ML_IN_CMD)
    {
      ML_BUF[ML_LEN] = '\0';
      ML_READY  = true;
      ML_IN_CMD = false;
      break;
    }
    else if (ML_IN_CMD && ML_LEN < (uint8_t)(sizeof(ML_BUF) - 1))
    {
      ML_BUF[ML_LEN++] = c;
    }
  }
}

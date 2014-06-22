#include "TargetSystem.h"
#include "TimerDriver.h"

int main()
{
  InitTimers();

  // Set PB0(OC0A) to output
  DDRB |= (1<<DDB0);

  TimerInstance* timer = CreateTimer();
  SetTimerCycleTimeMilliSec(
      timer,
      500
      );
  SetTimerCompareOutputMode(
      timer,
      SYSTEM_TIMER_OUTPUT_A,
      SYSTEM_TIMER_OUTPUT_MODE_TOGGLE
      );
  StartTimer(timer);

  while(1)
  {
  }

  return 0;
}

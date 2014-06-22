#include "TargetSystem.h"
#include "TimerDriver.h"

int main()
{
  InitTimers();

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

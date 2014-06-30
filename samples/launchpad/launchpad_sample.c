#include <stdint.h>

#include "TargetSystem.h"
#include "TimerDriver.h"

static void ToggleLED();

static uint8_t events [SYSTEM_NUM_EVENTS] = {FALSE};

int main()
{
  // Initialize timer driver
  InitTimers();

  TimerInstance* timer = CreateTimer();
  SetTimerCycleTimeMilliSec(
      timer,
      500
      );
  SetTimerCycleHandler(
      timer,
      ToggleLED
      );
  StartTimer(timer);

  System_EventType eventIter = 0;
  System_EventCallback currentEventCallback = NULL;
  
  while(1)
  {
    for(
        eventIter = 0;
        eventIter < SYSTEM_NUM_EVENTS;
        ++eventIter
       )
    {
      if (events[eventIter] == FALSE)
      {
        continue;
      }

      currentEventCallback = System_GetEventCallback(eventIter);

      if (currentEventCallback != NULL)
      {
        (*currentEventCallback)(eventIter);
      }

      events[eventIter] = FALSE;
    }
  }

  return 0;
}

void
ToggleLED()
{
}

#include <stdlib.h>
#include <stdint.h>
#include <avr/interrupt.h>

#include "TargetSystem.h"
#include "TimerDriver.h"

static void ToggleLED();

static uint8_t events [SYSTEM_NUM_EVENTS] = {FALSE};

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
  SetTimerCycleHandler(
      timer,
      ToggleLED
      );
  StartTimer(timer);

  System_EventType eventIter = 0;
  System_EventType currentEvent = SYSTEM_EVENT_INVALID;
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

      currentEventCallback = System_GetEventCallback(currentEvent);

      if (currentEventCallback != NULL)
      {
        (*currentEventCallback)(currentEvent);
      }

      events[eventIter] = FALSE;
    }
  }

  return 0;
}

void
ToggleLED()
{
  PINB |= (1<<PINB0);
}

ISR(TIM0_COMPA_vect)
{
  events[SYSTEM_EVENT_TIMER0_COMPAREMATCH] = TRUE;
}

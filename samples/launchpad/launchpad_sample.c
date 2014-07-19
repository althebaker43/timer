#include <stdint.h>
#include <isr_compat.h>

#include "TargetSystem.h"
#include "TimerDriver.h"

static void ToggleLED1();
static void ToggleLED2();

static uint8_t events [SYSTEM_NUM_EVENTS] = {FALSE};

int main()
{
  // Initialize LEDs
  P1DIR |= 1;
  P1OUT &= ~(1);
  P4DIR |= (1<<7);
  P4OUT &= ~(1<<7);

  // Initialize timer driver
  InitTimers();

  // Initialize timer 1
  TimerInstance* timer1 = CreateTimer();
  SetTimerCycleTimeMilliSec(
      timer1,
      500
      );
  SetTimerCycleHandler(
      timer1,
      ToggleLED1
      );

  // Initialize timer 2
  TimerInstance* timer2 = CreateTimer();
  SetTimerCycleTimeMilliSec(
      timer2,
      333
      );
  SetTimerCycleHandler(
      timer2,
      ToggleLED2
      );

  // Start timers
  StartTimer(timer1);
  StartTimer(timer2);

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
ToggleLED1()
{
  P1OUT ^= 1;
}

void
ToggleLED2()
{
  P4OUT ^= (1<<7);
}

ISR(TIMER0_A0, Timer1ServiceRoutine)
{
  events[SYSTEM_EVENT_TIMER0_COMPAREMATCH] = TRUE;
}

ISR(TIMER1_A0, Timer2ServiceRoutine)
{
  events[SYSTEM_EVENT_TIMER1_COMPAREMATCH] = TRUE;
}

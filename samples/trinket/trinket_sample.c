#include <stdlib.h>
#include <stdint.h>
#include <avr/interrupt.h>

#include "TargetSystem.h"
#include "TimerDriver.h"

static void ToggleLED();

static uint8_t events [SYSTEM_NUM_EVENTS] = {FALSE};

int main()
{
  // Enable interrupts
  sei();

  // Initialize timer driver
  InitTimers();

  // Set PORTB0 (OC0A) to output
  DDRB |= (1<<DDB0);

  // Start with PORTB0 set high
  PORTB |= (1<<PORTB0);

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
  PINB |= (1<<PINB0);
}

ISR(TIM0_COMPA_vect)
{
  events[SYSTEM_EVENT_TIMER0_COMPAREMATCH] = TRUE;
}

#include "TargetSystem.h"


static void (*timerCompareMatchCallbacks [SYSTEM_NUM_EVENTS])(System_EventType);


void
System_RegisterCallback(
    void (*callback)(System_EventType),
    System_EventType  event
    )
{
  if (event < SYSTEM_NUM_EVENTS)
  {
    timerCompareMatchCallbacks[event] = callback;
  }
}

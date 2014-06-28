#include "TargetSystem.h"


static void (*callbacks [SYSTEM_NUM_EVENTS])(System_EventType);


void
System_RegisterCallback(
    void (*callback)(System_EventType),
    System_EventType  event
    )
{
  if (event < SYSTEM_NUM_EVENTS)
  {
    callbacks[event] = callback;
  }
}

System_EventCallback
System_GetEventCallback(
    System_EventType  event
    )
{
  if (event < SYSTEM_NUM_EVENTS)
  {
    return callbacks[event];
  }
  else
  {
    return NULL;
  }
}

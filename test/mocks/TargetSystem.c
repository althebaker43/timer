#include "TargetSystem.h"

/**
 * Core clock frequency
 *
 * \note The default is 1MHz
 */
static unsigned long int coreClockFrequency = 1000000;

// Mock system settings
static System_TimerClockSource system_clockSources [SYSTEM_NUM_TIMERS];
static unsigned int system_compareValues [SYSTEM_NUM_TIMERS];
static System_TimerCompareOutputMode system_outputModes [SYSTEM_NUM_TIMERS];
static System_TimerWaveGenMode system_waveGenModes [SYSTEM_NUM_TIMERS];

static unsigned int system_events [SYSTEM_NUM_EVENTS] = {FALSE};
static System_EventCallback system_eventCallbacks [SYSTEM_NUM_EVENTS]; /**< Pointers to timer compare match event callback functions */


unsigned long int
System_TimerGetSourceFrequency(
    System_TimerClockSource clockSource
    )
{
  switch (clockSource)
  {
    case SYSTEM_TIMER_CLKSOURCE_INT:          return coreClockFrequency; break;
    case SYSTEM_TIMER_CLKSOURCE_INT_PRE8:     return (coreClockFrequency / 8); break;
    case SYSTEM_TIMER_CLKSOURCE_INT_PRE64:    return (coreClockFrequency / 64); break;
    case SYSTEM_TIMER_CLKSOURCE_INT_PRE256:   return (coreClockFrequency / 256); break;
    case SYSTEM_TIMER_CLKSOURCE_INT_PRE1024:  return (coreClockFrequency / 1024); break;
    default:
      return 0;
      break;
  };
}

unsigned int
System_TimerSetClockSource(
    System_TimerID          timer,
    System_TimerClockSource clockSource
    )
{
  system_clockSources[timer] = clockSource;
  return TRUE;
}

unsigned int
System_TimerSetCompareMatch(
    System_TimerID  timer,
    unsigned int    compareValue
    )
{
  system_compareValues[timer] = compareValue;
  return TRUE;
}

unsigned int
System_TimerSetCompareOutputMode(
    System_TimerID                timer,
    System_TimerCompareOutputMode outputMode
    )
{
  system_outputModes[timer] = outputMode;
  return TRUE;
}

unsigned int
System_TimerSetWaveGenMode(
    System_TimerID          timer,
    System_TimerWaveGenMode waveGenMode
    )
{
  system_waveGenModes[timer] = waveGenMode;
  return TRUE;
}

void
System_RegisterCallback(
    void (*callback)(System_EventType),
    System_EventType  event
    )
{
  if (event < SYSTEM_NUM_EVENTS)
  {
    system_eventCallbacks[event] = callback;
  }
}

unsigned int
System_EnableEvent(
    System_EventType  event
    )
{
  system_events[event] = TRUE;
  return TRUE;
}

unsigned int
System_DisableEvent(
    System_EventType  event
    )
{
  system_events[event] = FALSE;
  return TRUE;
}

System_EventType
System_GetTimerCallbackEvent(
    System_TimerID  timerID
    )
{
  switch (timerID)
  {
    case SYSTEM_TIMER0: return SYSTEM_EVENT_TIMER0_COMPAREMATCH; break;
    case SYSTEM_TIMER1: return SYSTEM_EVENT_TIMER1_COMPAREMATCH; break;
    case SYSTEM_TIMER2: return SYSTEM_EVENT_TIMER2_COMPAREMATCH; break;
    default: return SYSTEM_EVENT_INVALID; break;
  };
}

// Test accessors (not for production use)

System_TimerClockSource
System_TimerGetClockSource(
    System_TimerID  timer
    )
{
  return system_clockSources[timer];
}

unsigned int
System_TimerGetCompareValue(
    System_TimerID  timer
    )
{
  return system_compareValues[timer];
}

System_TimerCompareOutputMode
System_TimerGetCompareOutputMode(
    System_TimerID  timer
    )
{
  return system_outputModes[timer];
}

System_TimerWaveGenMode
System_TimerGetWaveGenMode(
    System_TimerID  timer
    )
{
  return system_waveGenModes[timer];
}

unsigned int
System_GetEvent(
    System_EventType  event
    )
{
  return system_events[event];
}

System_EventCallback
System_GetEventCallback(
    System_EventType  event
    )
{
  return system_eventCallbacks[event];
}

// Test manipulators (not for production use)

void
System_SetCoreClockFrequency(
    unsigned long int newFrequency
    )
{
  coreClockFrequency = newFrequency;
}

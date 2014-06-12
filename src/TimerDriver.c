#include "TimerDriver.h"
#include "TargetSystem.h"
#include <stdlib.h>

/**
 * Minimum frequency for a millisecond counter
 */
#define MAX_IDEAL_FREQ_MS_COUNTER 256000  // 1000 Hz * 256 (timer bit-width)

struct TimerInstance_struct
{
  TimerStatus             status;                 /**< Current status of the timer */
  System_TimerClockSource clockSource;            /**< Clock source currently used for this timer */
  uint8_t                 compareMatch;           /**< Value to trigger a compare match on */
  unsigned int            compareMatchesPerCycle; /**< Number of compare matches per timer cycle */
};

static int timersInitialized = FALSE;
static TimerInstance timerInstances [SYSTEM_NUM_TIMERS];
static int timerInstancesInUse [SYSTEM_NUM_TIMERS];
static unsigned int numTimerInstances = 0;

void
InitTimers()
{
  if (timersInitialized == TRUE)
  {
    return;
  }

  int timerIdx;
  for(
      timerIdx = 0;
      timerIdx < SYSTEM_NUM_TIMERS;
      timerIdx++
     )
  {
    timerInstancesInUse[timerIdx] = FALSE;
  }

  timersInitialized = TRUE;
  numTimerInstances = 0;
}

TimerInstance*
CreateTimer()
{
  if (
      (timersInitialized == FALSE) ||
      (numTimerInstances >= SYSTEM_NUM_TIMERS)
     )
  {
    return NULL;
  }

  int timerIdx;
  for(
      timerIdx = 0;
      timerIdx < SYSTEM_NUM_TIMERS;
      timerIdx++
     )
  {
    if (timerInstancesInUse[timerIdx] == FALSE)
    {
      TimerInstance* newTimer = &timerInstances[timerIdx];
      newTimer->status = TIMER_STATUS_STOPPED;
      timerInstancesInUse[timerIdx] = TRUE;
      numTimerInstances++;
      return newTimer;
    }
  }

  return NULL;
}

void
DestroyTimer(TimerInstance** instance)
{
  if(
      (instance == NULL) ||
      (*instance == NULL)
    )
  {
    return;
  }

  StopTimer(*instance);

  int timerIdx;
  for(
      timerIdx = 0;
      timerIdx < SYSTEM_NUM_TIMERS;
      timerIdx++
     )
  {
    if ((*instance) == &timerInstances[timerIdx])
    {
      timerInstancesInUse[timerIdx] = FALSE;
    }
  }

  *instance = NULL;
  numTimerInstances--;
}

void
DestroyAllTimers()
{
  int timerIdx;
  for(
      timerIdx = 0;
      timerIdx < SYSTEM_NUM_TIMERS;
      timerIdx++
     )
  {
    timerInstancesInUse[timerIdx] = FALSE;
  }

  numTimerInstances = 0;
}

TimerStatus
GetTimerStatus(TimerInstance* instance)
{
  int timerIdx;
  for(
      timerIdx = 0;
      timerIdx < SYSTEM_NUM_TIMERS;
      timerIdx++
     )
  {
    if (
        (instance == &timerInstances[timerIdx]) &&
        (timerInstancesInUse[timerIdx] == TRUE)
       )
    {
      return instance->status;
    }
  }
  
  return TIMER_STATUS_INVALID;
}

int
GetTimerClockSource(TimerInstance* instance)
{
  return instance->clockSource;
}

uint8_t
GetTimerCompareMatch(TimerInstance* instance)
{
  return instance->compareMatch;
}

int
GetTimerCompareMatchesPerCycle(TimerInstance* instance)
{
  return instance->compareMatchesPerCycle;
}

void
StartTimer(TimerInstance* instance)
{
  instance->status = TIMER_STATUS_RUNNING;
  System_TimerSetClockSource(SYSTEM_TIMER_CLKSOURCE_INT);
}

void
StopTimer(TimerInstance* instance)
{
  instance->status = TIMER_STATUS_STOPPED;
  System_TimerSetClockSource(SYSTEM_TIMER_CLKSOURCE_OFF);
}

int
SetTimerCycleTimeMilliSec(
    TimerInstance*  instance,
    unsigned int    numMilliSec
    )
{
  if (numMilliSec == 0)
  {
    return FALSE;
  }

  unsigned int idealFrequency = (unsigned int)(MAX_IDEAL_FREQ_MS_COUNTER / numMilliSec);
  unsigned int clockSourceFrequency = 0;

  int clockSourceIter;
  for(
      clockSourceIter = 0;
      clockSourceIter < NUM_TIMER_CLKSOURCES;
      clockSourceIter++
     )
  {
    clockSourceFrequency = System_TimerGetSourceFrequency(clockSourceIter);
    if (clockSourceFrequency == 0)
    {
      continue;
    }
    
    if (idealFrequency >= clockSourceFrequency)
    {
      instance->clockSource = clockSourceIter;
      instance->compareMatch = (uint8_t)((numMilliSec * clockSourceFrequency) / 1000);

      System_TimerSetClockSource(instance->clockSource);
      System_TimerSetCompareMatch(instance->compareMatch);

      return TRUE;
    }
  }

  return FALSE;
}

int
SetTimerCycleTimeSec(
    TimerInstance*  instance,
    unsigned int    numSec
    )
{
  instance->clockSource = SYSTEM_TIMER_CLKSOURCE_INT_PRE1024;
  instance->compareMatch = 244;
  instance->compareMatchesPerCycle = numSec * 4;
  
  System_TimerSetClockSource(instance->clockSource);
  System_TimerSetCompareMatch(instance->compareMatch);
  return TRUE;
}

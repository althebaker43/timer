#include "TimerDriver.h"
#include "TargetSystem.h"
#include <stdlib.h>

struct TimerInstance_struct
{
  TimerStatus status; /**< Current status of the timer */
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

void
StartTimer(TimerInstance* instance)
{
  instance->status = TIMER_STATUS_RUNNING;
  System_TimerSelectClock(TIMER_CLOCK_SELECT_ON);
}

void
StopTimer(TimerInstance* instance)
{
  instance->status = TIMER_STATUS_STOPPED;
  System_TimerSelectClock(TIMER_CLOCK_SELECT_OFF);
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

  unsigned int numClockCyclesPerMilliSec = (unsigned int)(0.001 * SYSTEM_IO_CLOCK_FREQ);
  unsigned int idealPrescaleFactor = (unsigned int)((numMilliSec * numClockCyclesPerMilliSec) / 256);
  unsigned int compareValue = 0;

  int prescalerIdx;
  for(
      prescalerIdx = 0;
      prescalerIdx < SYSTEM_NUM_TIMER_PRESCALERS;
      prescalerIdx++
     )
  {
    if (idealPrescaleFactor <= System_TimerHWPrescalers[prescalerIdx])
    {
      System_TimerSelectClock(TIMER_CLOCK_SELECT_ON + prescalerIdx);

      compareValue = (unsigned int)((numMilliSec * numClockCyclesPerMilliSec) / System_TimerHWPrescalers[prescalerIdx]); 
      System_TimerSetOutputCompare(compareValue);
      return TRUE;
    }
  }

  return FALSE;
}

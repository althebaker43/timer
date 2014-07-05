#include <stdlib.h>
#include <limits.h>

#include "TimerDriver.h"
#include "TargetSystem.h"

/**
 * Minimum frequency for a millisecond counter
 */
#define MAX_IDEAL_FREQ_MS_COUNTER 256000  // 1000 Hz * 256 (timer bit-width)

struct TimerInstance_struct
{
  System_TimerID                id;                     /**< System ID of timer */
  TimerStatus                   status;                 /**< Current status of the timer */
  System_TimerClockSource       clockSource;            /**< Clock source currently used for this timer */
  unsigned int                       compareMatch;           /**< Value to trigger a compare match on */
  unsigned int                       compareMatchesPerCycle; /**< Number of compare matches per timer cycle */
  System_TimerCompareOutputMode compareOutputMode;      /**< Compare output mode */
  unsigned int                       numCompareMatches;      /**< Number of compare matches counted in current cycle */
  unsigned int                       numCycles;              /**< Number of cycles counted */
  TimerCycleHandler             cycleHandler;           /**< Handler function to call for each cycle completion */
};

static unsigned int timersInitialized = FALSE;
static TimerInstance timerInstances [SYSTEM_NUM_TIMERS];
static unsigned int timerInstancesInUse [SYSTEM_NUM_TIMERS];
static unsigned int numTimerInstances = 0;

/**
 * Callback function for timer compare match events
 */
static void TimerCompareMatchCallback();

/**
 * Function for getting the timer's system ID
 */
static System_TimerID GetSystemID(TimerInstance*);

void
InitTimers()
{
  if (timersInitialized == TRUE)
  {
    return;
  }

  unsigned int timerIdx;
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

  unsigned int timerIdx;
  for(
      timerIdx = 0;
      timerIdx < SYSTEM_NUM_TIMERS;
      timerIdx++
     )
  {
    if (timerInstancesInUse[timerIdx] == FALSE)
    {
      TimerInstance* newTimer = &timerInstances[timerIdx];
      
      newTimer->id = timerIdx;
      newTimer->status = TIMER_STATUS_STOPPED;
      newTimer->clockSource = SYSTEM_TIMER_CLKSOURCE_OFF;
      newTimer->compareMatch = 0;
      newTimer->compareMatchesPerCycle = 1;
      newTimer->compareOutputMode = SYSTEM_TIMER_OUTPUT_MODE_NONE;
      newTimer->numCompareMatches = 0;
      newTimer->numCycles = 0;

      StopTimer(newTimer);

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

  unsigned int timerIdx;
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
  unsigned int timerIdx;
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
  unsigned int timerIdx;
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

unsigned int
GetTimerClockSource(TimerInstance* instance)
{
  return instance->clockSource;
}

unsigned int
GetTimerCompareMatch(TimerInstance* instance)
{
  return instance->compareMatch;
}

unsigned int
GetTimerCompareMatchesPerCycle(TimerInstance* instance)
{
  return instance->compareMatchesPerCycle;
}

unsigned int
StartTimer(TimerInstance* instance)
{
  if (
      (instance->compareMatch == 0) &&
      (instance->compareMatchesPerCycle)
     )
  {
    return FALSE;
  }

  System_TimerID id = GetSystemID(instance);
  System_EventType event = System_GetTimerCallbackEvent(id);

  System_RegisterCallback(
      TimerCompareMatchCallback,
      event
      );
  System_EnableEvent(event);

  System_TimerSetWaveGenMode(SYSTEM_TIMER_WAVEGEN_MODE_CTC);

  System_TimerSetClockSource(instance->clockSource);
  instance->status = TIMER_STATUS_RUNNING;

  return TRUE;
}

void
StopTimer(TimerInstance* instance)
{
  instance->status = TIMER_STATUS_STOPPED;
  System_TimerSetClockSource(SYSTEM_TIMER_CLKSOURCE_OFF);

  System_TimerID id = GetSystemID(instance);
  System_EventType event = System_GetTimerCallbackEvent(id);
  System_DisableEvent(event);
}

unsigned int
SetTimerCycleTimeMilliSec(
    TimerInstance*    instance,
    unsigned int      numMilliSec
    )
{
  if (numMilliSec == 0)
  {
    return FALSE;
  }

  unsigned long int idealFrequency = 0;
  unsigned int numMilliSecPerSubCycle = 0;
  unsigned long int clockSourceFrequency = 0;
  instance->compareMatchesPerCycle = 0;

  for (;;)
  {
    instance->compareMatchesPerCycle++;
    numMilliSecPerSubCycle = numMilliSec / instance->compareMatchesPerCycle;
    idealFrequency = (unsigned long int)(MAX_IDEAL_FREQ_MS_COUNTER / numMilliSecPerSubCycle);

    unsigned int clockSourceIter;
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
        instance->compareMatch = (unsigned int)((numMilliSecPerSubCycle * clockSourceFrequency) / 1000);

        System_TimerSetClockSource(instance->clockSource);
        System_TimerSetCompareMatch(instance->compareMatch);

        return TRUE;
      }
    }
  }

  return FALSE;
}

unsigned int
SetTimerCycleTimeSec(
    TimerInstance*      instance,
    unsigned short int  numSec
    )
{
  unsigned int MAX_NUM_SEC = UINT_MAX / 1000;

  if (numSec > MAX_NUM_SEC)
  {
    return FALSE;
  }

  return SetTimerCycleTimeMilliSec(
      instance,
      ((unsigned long int) numSec * 1000)
      );
}

unsigned int
GetTimerCompareOutputMode(
    TimerInstance*  instance,
    unsigned int         output
    )
{
  return instance->compareOutputMode;
}

unsigned int
SetTimerCompareOutputMode(
    TimerInstance* instance,
    unsigned int        output,
    unsigned int        mode
    )
{
  if (System_TimerSetCompareOutputMode(mode) == TRUE)
  {
    instance->compareOutputMode = mode;
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

unsigned int
GetNumTimerCompareMatches(
    TimerInstance*  instance
    )
{
  return instance->numCompareMatches;
}

unsigned int
GetNumTimerCycles(
    TimerInstance*  instance
    )
{
  return instance->numCycles;
}

static void
TimerCompareMatchCallback(
    System_EventType  event
    )
{
  unsigned int timerIdx;
  for(
      timerIdx = 0;
      timerIdx < SYSTEM_NUM_TIMERS;
      timerIdx++
     )
  {
    if (System_GetTimerCallbackEvent(GetSystemID(&timerInstances[timerIdx])) == event)
    {
      break;
    }
  }
  
  if (timerIdx == SYSTEM_NUM_TIMERS)
  {
    return;
  }

  TimerInstance* instance = &timerInstances[timerIdx];

  if (instance->numCompareMatches == instance->compareMatchesPerCycle - 1)
  {
    instance->numCompareMatches = 0;
    instance->numCycles++;

    if (instance->cycleHandler != NULL)
    {
      (*(instance->cycleHandler))();
    }
  }
  else
  {
    instance->numCompareMatches++;
  }

  return;
}

System_TimerID
GetSystemID(
    TimerInstance*  instance
    )
{
  return instance->id;
}

TimerCycleHandler
GetTimerCycleHandler(
    TimerInstance*  instance
    )
{
  return instance->cycleHandler;
}

unsigned int
SetTimerCycleHandler(
    TimerInstance*    instance,
    TimerCycleHandler handler
    )
{
  instance->cycleHandler = handler;
  return TRUE;
}

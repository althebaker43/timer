#include "TimerDriver.h"
#include "TargetSystem.h"
#include <stdlib.h>

/**
 * Minimum frequency for a millisecond counter
 */
#define MAX_IDEAL_FREQ_MS_COUNTER 256000  // 1000 Hz * 256 (timer bit-width)

struct TimerInstance_struct
{
  TimerStatus                   status;                 /**< Current status of the timer */
  System_TimerClockSource       clockSource;            /**< Clock source currently used for this timer */
  uint8_t                       compareMatch;           /**< Value to trigger a compare match on */
  uint8_t                       compareMatchesPerCycle; /**< Number of compare matches per timer cycle */
  System_TimerCompareOutputMode compareOutputMode;      /**< Compare output mode */
};

static uint8_t timersInitialized = FALSE;
static TimerInstance timerInstances [SYSTEM_NUM_TIMERS];
static uint8_t timerInstancesInUse [SYSTEM_NUM_TIMERS];
static uint8_t numTimerInstances = 0;

void
InitTimers()
{
  if (timersInitialized == TRUE)
  {
    return;
  }

  uint8_t timerIdx;
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

  uint8_t timerIdx;
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

  uint8_t timerIdx;
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
  uint8_t timerIdx;
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
  uint8_t timerIdx;
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

uint8_t
GetTimerClockSource(TimerInstance* instance)
{
  return instance->clockSource;
}

uint8_t
GetTimerCompareMatch(TimerInstance* instance)
{
  return instance->compareMatch;
}

uint8_t
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

uint8_t
SetTimerCycleTimeMilliSec(
    TimerInstance*  instance,
    uint16_t        numMilliSec
    )
{
  if (numMilliSec == 0)
  {
    return FALSE;
  }

  uint32_t idealFrequency = 0;
  uint16_t numMilliSecPerSubCycle = 0;
  uint32_t clockSourceFrequency = 0;
  instance->compareMatchesPerCycle = 0;

  for (;;)
  {
    instance->compareMatchesPerCycle++;
    numMilliSecPerSubCycle = numMilliSec / instance->compareMatchesPerCycle;
    idealFrequency = (uint32_t)(MAX_IDEAL_FREQ_MS_COUNTER / numMilliSecPerSubCycle);

    uint8_t clockSourceIter;
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
        instance->compareMatch = (uint8_t)((numMilliSecPerSubCycle * clockSourceFrequency) / 1000);

        System_TimerSetClockSource(instance->clockSource);
        System_TimerSetCompareMatch(instance->compareMatch);

        return TRUE;
      }
    }
  }

  return FALSE;
}

uint8_t
SetTimerCycleTimeSec(
    TimerInstance*  instance,
    uint8_t         numSec
    )
{
  return SetTimerCycleTimeMilliSec(
      instance,
      (numSec * 1000)
      );
}

uint8_t
GetTimerCompareOutputMode(
    TimerInstance*  instance,
    uint8_t         output
    )
{
  return instance->compareOutputMode;
}

uint8_t
SetTimerCompareOutputMode(
    TimerInstance* instance,
    uint8_t        output,
    uint8_t        mode
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

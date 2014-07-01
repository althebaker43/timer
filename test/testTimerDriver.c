#include <stdlib.h>
#include <stdint.h>
#include "unity_fixture.h"
#include "TimerDriver.h"
#include "TargetSystem.h"

TEST_GROUP(TimerDriver);

static TimerInstance** timers = NULL;

/**
 * Core clock frequency
 *
 * \note The default is 1MHz
 */
static uint32_t coreClockFrequency = 1000000;

// Mock system settings
static System_TimerClockSource system_clockSource;
static uint8_t system_compareValue;
static System_TimerCompareOutputMode system_outputMode;
static System_TimerWaveGenMode system_waveGenMode;
static uint8_t system_events [SYSTEM_NUM_EVENTS] = {FALSE};
static void (*system_eventCallbacks [SYSTEM_NUM_EVENTS])(System_EventType); /**< Pointers to timer compare match event callback functions */

uint32_t
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

uint8_t
System_TimerSetClockSource(
    System_TimerClockSource clockSource
    )
{
  system_clockSource = clockSource;
  return TRUE;
}

uint8_t
System_TimerSetCompareMatch(
    uint8_t compareValue
    )
{
  system_compareValue = compareValue;
  return TRUE;
}

uint8_t
System_TimerSetCompareOutputMode(
    System_TimerCompareOutputMode outputMode
    )
{
  system_outputMode = outputMode;
  return TRUE;
}

uint8_t
System_TimerSetWaveGenMode(
    System_TimerWaveGenMode waveGenMode
    )
{
  system_waveGenMode = waveGenMode;
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

uint8_t
System_EnableEvent(
    System_EventType  event
    )
{
  system_events[event] = TRUE;
  return TRUE;
}

uint8_t
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

static uint8_t numCustomTimerCycles = 0;

static void
CustomTimerCycleCounter()
{
  numCustomTimerCycles++;
}

static void testCreateAllTimers()
{
  InitTimers();
  timers = (TimerInstance**)malloc((sizeof(TimerInstance*)) * SYSTEM_NUM_TIMERS);

  uint8_t timerIdx;
  for(
      timerIdx = 0;
      timerIdx < SYSTEM_NUM_TIMERS;
      timerIdx++
     )
  {
    timers[timerIdx] = CreateTimer();
  }
}

static void testDestroyAllTimers()
{
  if (timers == NULL)
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
    DestroyTimer(&timers[timerIdx]);
  }

  free(timers);
  timers = NULL;
}

TEST_SETUP(TimerDriver)
{
  system_clockSource = SYSTEM_TIMER_CLKSOURCE_INVALID;
  system_compareValue = 0;
  system_outputMode = SYSTEM_TIMER_OUTPUT_MODE_NONE;
  system_waveGenMode = SYSTEM_TIMER_WAVEGEN_MODE_CTC;
  coreClockFrequency = 1000000; // Default core frequency to 1MHz
  numCustomTimerCycles = 0;

  uint8_t eventIdx;
  for(
      eventIdx = 0;
      eventIdx < SYSTEM_NUM_EVENTS;
      eventIdx++
     )
  {
    system_eventCallbacks[eventIdx] = NULL;
    system_events[eventIdx] = FALSE;
  }
}

TEST_TEAR_DOWN(TimerDriver)
{
  testDestroyAllTimers(); // Clears this framework's timers
  DestroyAllTimers();     // Resets the timer driver
}

TEST(TimerDriver, NoTimersBeforeInit)
{
  TEST_ASSERT_NULL(CreateTimer());

  InitTimers();
  
  TEST_ASSERT_NOT_NULL(CreateTimer());
}

TEST(TimerDriver, MultiInit)
{
  testCreateAllTimers();
  InitTimers();

  TEST_ASSERT_NULL(CreateTimer());
}

TEST(TimerDriver, CreateTimer)
{
  testCreateAllTimers();

  uint8_t timerIdx;
  for(
      timerIdx = 0;
      timerIdx < SYSTEM_NUM_TIMERS;
      timerIdx++
     )
  {
    TimerInstance* curTimer = timers[timerIdx];
    
    TEST_ASSERT_NOT_NULL(curTimer);
    TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCompareMatches(curTimer));
    TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(curTimer));
    TEST_ASSERT_EQUAL_UINT8(TIMER_STATUS_STOPPED, GetTimerStatus(curTimer));
    TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_OFF, GetTimerClockSource(curTimer));
    TEST_ASSERT_EQUAL_UINT8(0, GetTimerCompareMatch(curTimer));
    TEST_ASSERT_EQUAL_UINT8(1, GetTimerCompareMatchesPerCycle(curTimer));
    TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_OUTPUT_MODE_NONE, GetTimerCompareOutputMode(curTimer, SYSTEM_TIMER_OUTPUT_A));
  }
}

TEST(TimerDriver, DestroyTimer)
{
  testCreateAllTimers();
  DestroyTimer(&timers[0]);

  TEST_ASSERT_NULL(timers[0]);
}

TEST(TimerDriver, DestroyAllTimers)
{
  testCreateAllTimers();
  DestroyAllTimers();

  uint8_t timerIdx;
  for(
      timerIdx = 0;
      timerIdx < SYSTEM_NUM_TIMERS;
      timerIdx++
     )
  {
    timers[timerIdx] = CreateTimer();
    TEST_ASSERT_NOT_NULL(timers[timerIdx]);
  }
}

TEST(TimerDriver, NotEnoughHardware)
{
  testCreateAllTimers();

  TEST_ASSERT_NULL(CreateTimer());
}

TEST(TimerDriver, TrackNumOfTimers)
{
  testCreateAllTimers();
  DestroyTimer(&timers[0]);

  TEST_ASSERT_NOT_NULL(CreateTimer());
}

TEST(TimerDriver, NullTimerStatus)
{
  TEST_ASSERT_EQUAL_UINT8(TIMER_STATUS_INVALID, GetTimerStatus(NULL));
}

TEST(TimerDriver, InvalidTimerStatus)
{
  testCreateAllTimers();
  TimerInstance* invalidTimer = timers[0];
  DestroyTimer(&timers[0]);

  TEST_ASSERT_EQUAL_UINT8(TIMER_STATUS_INVALID, GetTimerStatus(invalidTimer));
}

TEST(TimerDriver, StoppedOnInit)
{
  testCreateAllTimers();

  TEST_ASSERT_EQUAL_UINT8(TIMER_STATUS_STOPPED, GetTimerStatus(timers[0]));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_OFF, system_clockSource);

  StartTimer(timers[0]);
  DestroyTimer(&timers[0]);

  timers[0] = CreateTimer();
  TEST_ASSERT_EQUAL_UINT8(TIMER_STATUS_STOPPED, GetTimerStatus(timers[0]));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_OFF, system_clockSource);
}

TEST(TimerDriver, ClearTimerOnCompareMatch)
{
  testCreateAllTimers();

  SetTimerCycleTimeMilliSec(
      timers[0],
      500
      );
  StartTimer(timers[0]);

  TEST_ASSERT_EQUAL(SYSTEM_TIMER_WAVEGEN_MODE_CTC, system_waveGenMode);
}

TEST(TimerDriver, StoppedOnDestroy)
{
  testCreateAllTimers();
  StartTimer(timers[0]);
  DestroyTimer(&timers[0]);

  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_OFF, system_clockSource);
}

TEST(TimerDriver, NoRunningWithoutTime)
{
  testCreateAllTimers();
  
  TEST_ASSERT_FALSE(StartTimer(timers[0]));

  SetTimerCycleTimeMilliSec(
      timers[0],
      500
      );
  
  TEST_ASSERT(StartTimer(timers[0]));
}

TEST(TimerDriver, RunningAfterStart)
{
  testCreateAllTimers();
  SetTimerCycleTimeMilliSec(
      timers[0],
      500
      );
  StartTimer(timers[0]);

  TEST_ASSERT_EQUAL_UINT8(TIMER_STATUS_RUNNING, GetTimerStatus(timers[0]));
  TEST_ASSERT(SYSTEM_TIMER_CLKSOURCE_OFF != system_clockSource);
}

TEST(TimerDriver, NoPowerReductionAfterStart)
{
  TEST_IGNORE_MESSAGE("Check for disabled power reduction not yet implemented.");
}

TEST(TimerDriver, StoppedAfterStop)
{
  testCreateAllTimers();
  StartTimer(timers[0]);
  StopTimer(timers[0]);

  TEST_ASSERT_EQUAL_UINT8(TIMER_STATUS_STOPPED, GetTimerStatus(timers[0]));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_OFF, system_clockSource);
}

TEST(TimerDriver, SetCycleTimeMilliSec)
{
  testCreateAllTimers();
  
  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 100));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, system_clockSource);
  TEST_ASSERT_EQUAL_UINT8(97, system_compareValue);
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(97, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(1, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 1));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE8, system_clockSource);
  TEST_ASSERT_EQUAL_UINT8(125, system_compareValue);
  TEST_ASSERT_EQUAL_UINT8(125, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE8, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(1, GetTimerCompareMatchesPerCycle(timers[0]));

  // Maximum number of milliseconds for 1MHz core clock without software divider
  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 262));
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, system_clockSource);
  TEST_ASSERT_EQUAL_UINT8(255, system_compareValue);
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(255, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(1, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT_TRUE(SetTimerCycleTimeMilliSec(timers[0], 263));
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, system_clockSource);
  TEST_ASSERT_EQUAL_UINT8(127, system_compareValue);
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(127, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(2, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT_TRUE(SetTimerCycleTimeMilliSec(timers[0], 500));
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, system_clockSource);
  TEST_ASSERT_EQUAL_UINT8(244, system_compareValue);
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(244, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(2, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT_FALSE(SetTimerCycleTimeMilliSec(timers[0], 0));

  // Change clock to 8MHz
  coreClockFrequency = 8000000;
  
  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 100));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, system_clockSource);
  TEST_ASSERT_EQUAL_UINT8(195, system_compareValue);
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(195, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(4, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 1));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE64, system_clockSource);
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE64, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(125, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(1, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 500));
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, system_clockSource);
  TEST_ASSERT_EQUAL_UINT8(242, system_compareValue);
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(242, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(16, GetTimerCompareMatchesPerCycle(timers[0]));
}

TEST(TimerDriver, ClockSourceSelection)
{
  coreClockFrequency = 8000000;

  TimerInstance* timer = CreateTimer();
  SetTimerCycleTimeMilliSec(
      timer,
      500
      );
  StartTimer(timer);
  
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, system_clockSource);
  TEST_ASSERT_EQUAL_UINT8(242, system_compareValue);
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timer));
  TEST_ASSERT_EQUAL_UINT8(242, GetTimerCompareMatch(timer));
  TEST_ASSERT_EQUAL_UINT8(16, GetTimerCompareMatchesPerCycle(timer));

  DestroyTimer(&timer);
}

TEST(TimerDriver, SetCycleTimeSec)
{
  testCreateAllTimers();

  TEST_ASSERT(SetTimerCycleTimeSec(timers[0], 1));
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, system_clockSource);
  TEST_ASSERT_EQUAL_UINT8(244, system_compareValue);
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(244, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(4, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT(SetTimerCycleTimeSec(timers[0], 2));
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, system_clockSource);
  TEST_ASSERT_EQUAL_UINT8(244, system_compareValue);
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(244, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(8, GetTimerCompareMatchesPerCycle(timers[0]));
}

TEST(TimerDriver, CycleTimeOverflow)
{
  testCreateAllTimers();

  TEST_ASSERT(
      SetTimerCycleTimeSec(
        timers[0],
        (UINT16_MAX) / 1000
        )
      );

  TEST_ASSERT_FALSE(
      SetTimerCycleTimeSec(
        timers[0],
        (((UINT16_MAX) / 1000) + 1)
        )
      );
}

TEST(TimerDriver, HiFreqAccuracy)
{
  TEST_IGNORE_MESSAGE("Accuracy for long timers with high clock frequency not yet implemented.");

  testCreateAllTimers();

  // Change clock to 8MHz
  coreClockFrequency = 8000000;

  TEST_ASSERT(SetTimerCycleTimeSec(timers[0], 1));
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, system_clockSource);
  TEST_ASSERT_EQUAL_UINT8(252, system_compareValue);
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(252, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(31, GetTimerCompareMatchesPerCycle(timers[0]));
}

TEST(TimerDriver, EnableCompareMatchEvents)
{
  testCreateAllTimers();

  TEST_ASSERT_FALSE(system_events[SYSTEM_EVENT_TIMER0_COMPAREMATCH]);

  SetTimerCycleTimeSec(timers[0], 1);

  StartTimer(timers[0]);
  TEST_ASSERT(system_events[SYSTEM_EVENT_TIMER0_COMPAREMATCH]);

  StopTimer(timers[0]);
  TEST_ASSERT_FALSE(system_events[SYSTEM_EVENT_TIMER0_COMPAREMATCH]);
}

TEST(TimerDriver, CountUpOnCompareMatch)
{
  testCreateAllTimers();

  TEST_ASSERT_NULL(system_eventCallbacks[0]);
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[0]));

  TEST_ASSERT(SetTimerCycleTimeSec(timers[0], 1));
  
  StartTimer(timers[0]);
  TEST_ASSERT_NOT_NULL(system_eventCallbacks[0]);
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[0]));

  (system_eventCallbacks[0])(SYSTEM_EVENT_TIMER0_COMPAREMATCH);
  TEST_ASSERT_EQUAL_UINT8(1, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[0]));

  (system_eventCallbacks[0])(SYSTEM_EVENT_TIMER0_COMPAREMATCH);
  TEST_ASSERT_EQUAL_UINT8(2, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[0]));

  (system_eventCallbacks[0])(SYSTEM_EVENT_TIMER0_COMPAREMATCH);
  TEST_ASSERT_EQUAL_UINT8(3, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[0]));

  (system_eventCallbacks[0])(SYSTEM_EVENT_TIMER0_COMPAREMATCH);
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(1, GetNumTimerCycles(timers[0]));
}

TEST(TimerDriver, CompareMatchMultiTimers)
{
  testCreateAllTimers();

  TEST_ASSERT_NULL(system_eventCallbacks[0]);
  TEST_ASSERT_NULL(system_eventCallbacks[1]);

  SetTimerCycleTimeSec(timers[0], 1);
  StartTimer(timers[0]);
  SetTimerCycleTimeMilliSec(timers[1], 500);
  StartTimer(timers[1]);

  TEST_ASSERT_NOT_NULL(system_eventCallbacks[0]);
  TEST_ASSERT_NOT_NULL(system_eventCallbacks[1]);
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCompareMatches(timers[1]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[1]));

  (*system_eventCallbacks[0])(SYSTEM_EVENT_TIMER0_COMPAREMATCH);
  TEST_ASSERT_EQUAL_UINT8(1, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCompareMatches(timers[1]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[1]));

  (*system_eventCallbacks[1])(SYSTEM_EVENT_TIMER1_COMPAREMATCH);
  TEST_ASSERT_EQUAL_UINT8(1, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(1, GetNumTimerCompareMatches(timers[1]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[1]));

  (*system_eventCallbacks[1])(SYSTEM_EVENT_TIMER1_COMPAREMATCH);
  TEST_ASSERT_EQUAL_UINT8(1, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCompareMatches(timers[1]));
  TEST_ASSERT_EQUAL_UINT8(1, GetNumTimerCycles(timers[1]));

  (*system_eventCallbacks[0])(SYSTEM_EVENT_TIMER0_COMPAREMATCH);
  TEST_ASSERT_EQUAL_UINT8(2, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCompareMatches(timers[1]));
  TEST_ASSERT_EQUAL_UINT8(1, GetNumTimerCycles(timers[1]));
}

TEST(TimerDriver, CompareOutputMode)
{
  testCreateAllTimers();

  uint8_t timerIdx;
  for(
      timerIdx = 0;
      timerIdx < SYSTEM_NUM_TIMERS;
      timerIdx++
     )
  {
    TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_OUTPUT_MODE_NONE, GetTimerCompareOutputMode(timers[timerIdx], SYSTEM_TIMER_OUTPUT_A));
  }
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_OUTPUT_MODE_NONE, system_outputMode);

  TEST_ASSERT(SetTimerCompareOutputMode(timers[0], SYSTEM_TIMER_OUTPUT_A, SYSTEM_TIMER_OUTPUT_MODE_SET));
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_OUTPUT_MODE_SET, GetTimerCompareOutputMode(timers[0], SYSTEM_TIMER_OUTPUT_A));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_OUTPUT_MODE_SET, system_outputMode);
}

TEST(TimerDriver, CustomCycleHandler)
{
  testCreateAllTimers();

  TEST_ASSERT_NULL(GetTimerCycleHandler(timers[0]));

  SetTimerCycleTimeMilliSec(timers[0], 250);
  TEST_ASSERT(SetTimerCycleHandler(timers[0], CustomTimerCycleCounter));

  TEST_ASSERT_EQUAL_HEX(CustomTimerCycleCounter, GetTimerCycleHandler(timers[0]));

  StartTimer(timers[0]);
  (*system_eventCallbacks[0])(SYSTEM_EVENT_TIMER0_COMPAREMATCH);
  TEST_ASSERT_EQUAL_UINT8(1, GetNumTimerCycles(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(1, numCustomTimerCycles);
}

#include <stdlib.h>
#include <stdint.h>
#include "unity_fixture.h"
#include "TimerDriver.h"
#include "TargetSystem.h"

TEST_GROUP(TimerDriver);

static TimerInstance** timers = NULL;

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
  System_TimerSetClockSource(SYSTEM_TIMER_CLKSOURCE_INVALID);
  System_TimerSetCompareMatch(0);
  System_TimerSetCompareOutputMode(SYSTEM_TIMER_OUTPUT_MODE_NONE);
  System_TimerSetWaveGenMode(SYSTEM_TIMER_WAVEGEN_MODE_CTC);
  System_SetCoreClockFrequency(1000000);
  numCustomTimerCycles = 0;

  uint8_t eventIdx;
  for(
      eventIdx = 0;
      eventIdx < SYSTEM_NUM_EVENTS;
      eventIdx++
     )
  {
    System_DisableEvent(eventIdx);
    System_RegisterCallback(NULL, eventIdx);
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
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_OFF, System_TimerGetClockSource());

  StartTimer(timers[0]);
  DestroyTimer(&timers[0]);

  timers[0] = CreateTimer();
  TEST_ASSERT_EQUAL_UINT8(TIMER_STATUS_STOPPED, GetTimerStatus(timers[0]));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_OFF, System_TimerGetClockSource());
}

TEST(TimerDriver, ClearTimerOnCompareMatch)
{
  testCreateAllTimers();

  SetTimerCycleTimeMilliSec(
      timers[0],
      500
      );
  StartTimer(timers[0]);

  TEST_ASSERT_EQUAL(SYSTEM_TIMER_WAVEGEN_MODE_CTC, System_TimerGetWaveGenMode());
}

TEST(TimerDriver, StoppedOnDestroy)
{
  testCreateAllTimers();
  StartTimer(timers[0]);
  DestroyTimer(&timers[0]);

  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_OFF, System_TimerGetClockSource());
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
  TEST_ASSERT(SYSTEM_TIMER_CLKSOURCE_OFF != System_TimerGetClockSource());
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
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_OFF, System_TimerGetClockSource());
}

TEST(TimerDriver, SetCycleTimeMilliSec)
{
  testCreateAllTimers();
  
  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 100));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, System_TimerGetClockSource());
  TEST_ASSERT_EQUAL_UINT8(97, System_TimerGetCompareValue());
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(97, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(1, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 1));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE8, System_TimerGetClockSource());
  TEST_ASSERT_EQUAL_UINT8(125, System_TimerGetCompareValue());
  TEST_ASSERT_EQUAL_UINT8(125, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE8, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(1, GetTimerCompareMatchesPerCycle(timers[0]));

  // Maximum number of milliseconds for 1MHz core clock without software divider
  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 262));
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, System_TimerGetClockSource());
  TEST_ASSERT_EQUAL_UINT8(255, System_TimerGetCompareValue());
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(255, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(1, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT_TRUE(SetTimerCycleTimeMilliSec(timers[0], 263));
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, System_TimerGetClockSource());
  TEST_ASSERT_EQUAL_UINT8(127, System_TimerGetCompareValue());
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(127, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(2, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT_TRUE(SetTimerCycleTimeMilliSec(timers[0], 500));
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, System_TimerGetClockSource());
  TEST_ASSERT_EQUAL_UINT8(244, System_TimerGetCompareValue());
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(244, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(2, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT_FALSE(SetTimerCycleTimeMilliSec(timers[0], 0));

  // Change clock to 8MHz
  System_SetCoreClockFrequency(8000000);
  
  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 100));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, System_TimerGetClockSource());
  TEST_ASSERT_EQUAL_UINT8(195, System_TimerGetCompareValue());
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(195, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(4, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 1));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE64, System_TimerGetClockSource());
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE64, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(125, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(1, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 500));
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, System_TimerGetClockSource());
  TEST_ASSERT_EQUAL_UINT8(242, System_TimerGetCompareValue());
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(242, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(16, GetTimerCompareMatchesPerCycle(timers[0]));
}

TEST(TimerDriver, ClockSourceSelection)
{
  System_SetCoreClockFrequency(8000000);

  TimerInstance* timer = CreateTimer();
  SetTimerCycleTimeMilliSec(
      timer,
      500
      );
  StartTimer(timer);
  
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, System_TimerGetClockSource());
  TEST_ASSERT_EQUAL_UINT8(242, System_TimerGetCompareValue());
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timer));
  TEST_ASSERT_EQUAL_UINT8(242, GetTimerCompareMatch(timer));
  TEST_ASSERT_EQUAL_UINT8(16, GetTimerCompareMatchesPerCycle(timer));

  DestroyTimer(&timer);
}

TEST(TimerDriver, SetCycleTimeSec)
{
  testCreateAllTimers();

  TEST_ASSERT(SetTimerCycleTimeSec(timers[0], 1));
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, System_TimerGetClockSource());
  TEST_ASSERT_EQUAL_UINT8(244, System_TimerGetCompareValue());
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(244, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(4, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT(SetTimerCycleTimeSec(timers[0], 2));
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, System_TimerGetClockSource());
  TEST_ASSERT_EQUAL_UINT8(244, System_TimerGetCompareValue());
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
  System_SetCoreClockFrequency(8000000);

  TEST_ASSERT(SetTimerCycleTimeSec(timers[0], 1));
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, System_TimerGetClockSource());
  TEST_ASSERT_EQUAL_UINT8(252, System_TimerGetCompareValue());
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(252, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(31, GetTimerCompareMatchesPerCycle(timers[0]));
}

TEST(TimerDriver, EnableCompareMatchEvents)
{
  testCreateAllTimers();

  TEST_ASSERT_FALSE(System_GetEvent(SYSTEM_EVENT_TIMER0_COMPAREMATCH));

  SetTimerCycleTimeSec(timers[0], 1);

  StartTimer(timers[0]);
  TEST_ASSERT(System_GetEvent(SYSTEM_EVENT_TIMER0_COMPAREMATCH));

  StopTimer(timers[0]);
  TEST_ASSERT_FALSE(System_GetEvent(SYSTEM_EVENT_TIMER0_COMPAREMATCH));
}

TEST(TimerDriver, CountUpOnCompareMatch)
{
  testCreateAllTimers();

  TEST_ASSERT_NULL(System_GetEventCallback(SYSTEM_EVENT_TIMER0_COMPAREMATCH));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[0]));

  TEST_ASSERT(SetTimerCycleTimeSec(timers[0], 1));
  
  StartTimer(timers[0]);
  TEST_ASSERT_NOT_NULL(System_GetEventCallback(SYSTEM_EVENT_TIMER0_COMPAREMATCH));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[0]));

  (*(System_GetEventCallback(SYSTEM_EVENT_TIMER0_COMPAREMATCH)))(SYSTEM_EVENT_TIMER0_COMPAREMATCH);
  TEST_ASSERT_EQUAL_UINT8(1, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[0]));

  (*(System_GetEventCallback(SYSTEM_EVENT_TIMER0_COMPAREMATCH)))(SYSTEM_EVENT_TIMER0_COMPAREMATCH);
  TEST_ASSERT_EQUAL_UINT8(2, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[0]));

  (*(System_GetEventCallback(SYSTEM_EVENT_TIMER0_COMPAREMATCH)))(SYSTEM_EVENT_TIMER0_COMPAREMATCH);
  TEST_ASSERT_EQUAL_UINT8(3, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[0]));

  (*(System_GetEventCallback(SYSTEM_EVENT_TIMER0_COMPAREMATCH)))(SYSTEM_EVENT_TIMER0_COMPAREMATCH);
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(1, GetNumTimerCycles(timers[0]));
}

TEST(TimerDriver, CompareMatchMultiTimers)
{
  testCreateAllTimers();

  TEST_ASSERT_NULL(System_GetEventCallback(SYSTEM_EVENT_TIMER0_COMPAREMATCH));
  TEST_ASSERT_NULL(System_GetEventCallback(SYSTEM_EVENT_TIMER1_COMPAREMATCH));

  SetTimerCycleTimeSec(timers[0], 1);
  StartTimer(timers[0]);
  SetTimerCycleTimeMilliSec(timers[1], 500);
  StartTimer(timers[1]);

  TEST_ASSERT_NOT_NULL(System_GetEventCallback(SYSTEM_EVENT_TIMER0_COMPAREMATCH));
  TEST_ASSERT_NOT_NULL(System_GetEventCallback(SYSTEM_EVENT_TIMER1_COMPAREMATCH));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCompareMatches(timers[1]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[1]));

  (*(System_GetEventCallback(SYSTEM_EVENT_TIMER0_COMPAREMATCH)))(SYSTEM_EVENT_TIMER0_COMPAREMATCH);
  TEST_ASSERT_EQUAL_UINT8(1, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCompareMatches(timers[1]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[1]));

  (*(System_GetEventCallback(SYSTEM_EVENT_TIMER1_COMPAREMATCH)))(SYSTEM_EVENT_TIMER1_COMPAREMATCH);
  TEST_ASSERT_EQUAL_UINT8(1, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(1, GetNumTimerCompareMatches(timers[1]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[1]));

  (*(System_GetEventCallback(SYSTEM_EVENT_TIMER1_COMPAREMATCH)))(SYSTEM_EVENT_TIMER1_COMPAREMATCH);
  TEST_ASSERT_EQUAL_UINT8(1, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCycles(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(0, GetNumTimerCompareMatches(timers[1]));
  TEST_ASSERT_EQUAL_UINT8(1, GetNumTimerCycles(timers[1]));

  (*(System_GetEventCallback(SYSTEM_EVENT_TIMER0_COMPAREMATCH)))(SYSTEM_EVENT_TIMER0_COMPAREMATCH);
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
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_OUTPUT_MODE_NONE, System_TimerGetCompareOutputMode());

  TEST_ASSERT(SetTimerCompareOutputMode(timers[0], SYSTEM_TIMER_OUTPUT_A, SYSTEM_TIMER_OUTPUT_MODE_SET));
  TEST_ASSERT_EQUAL_UINT8(SYSTEM_TIMER_OUTPUT_MODE_SET, GetTimerCompareOutputMode(timers[0], SYSTEM_TIMER_OUTPUT_A));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_OUTPUT_MODE_SET, System_TimerGetCompareOutputMode());
}

TEST(TimerDriver, CustomCycleHandler)
{
  testCreateAllTimers();

  TEST_ASSERT_NULL(GetTimerCycleHandler(timers[0]));

  SetTimerCycleTimeMilliSec(timers[0], 250);
  TEST_ASSERT(SetTimerCycleHandler(timers[0], CustomTimerCycleCounter));

  TEST_ASSERT_EQUAL_HEX(CustomTimerCycleCounter, GetTimerCycleHandler(timers[0]));

  StartTimer(timers[0]);
  (*(System_GetEventCallback(SYSTEM_EVENT_TIMER0_COMPAREMATCH)))(SYSTEM_EVENT_TIMER0_COMPAREMATCH);
  TEST_ASSERT_EQUAL_UINT8(1, GetNumTimerCycles(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(1, numCustomTimerCycles);
}

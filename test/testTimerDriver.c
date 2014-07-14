#include <stdlib.h>
#include <limits.h>

#include "unity_fixture.h"
#include "TimerDriver.h"
#include "TargetSystem.h"

TEST_GROUP(TimerDriver);

static TimerInstance** timers = NULL;

static unsigned int numCustomTimerCycles = 0;

static void
CustomTimerCycleCounter()
{
  numCustomTimerCycles++;
}

static void testCreateAllTimers()
{
  InitTimers();
  timers = (TimerInstance**)malloc((sizeof(TimerInstance*)) * SYSTEM_NUM_TIMERS);

  unsigned int timerIdx;
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

  unsigned int timerIdx;
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
  timers = NULL;
  numCustomTimerCycles = 0;
  System_SetCoreClockFrequency(1000000);

  unsigned int timerIdx;
  for(
      timerIdx = 0;
      timerIdx < SYSTEM_NUM_TIMERS;
      timerIdx++
     )
  {
    System_SetMaxTimerValue(
        timerIdx,
        256 // 8-bit timer
        );
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

  System_TimerID timerIdx;
  for(
      timerIdx = 0;
      timerIdx < SYSTEM_NUM_TIMERS;
      timerIdx++
     )
  {
    TimerInstance* curTimer = timers[timerIdx];
    
    TEST_ASSERT_NOT_NULL(curTimer);
    TEST_ASSERT_EQUAL(0, GetNumTimerCompareMatches(curTimer));
    TEST_ASSERT_EQUAL(0, GetNumTimerCycles(curTimer));
    TEST_ASSERT_EQUAL(TIMER_STATUS_STOPPED, GetTimerStatus(curTimer));
    TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_OFF, GetTimerClockSource(curTimer));
    TEST_ASSERT_EQUAL(0, GetTimerCompareMatch(curTimer));
    TEST_ASSERT_EQUAL(1, GetTimerCompareMatchesPerCycle(curTimer));
    TEST_ASSERT_EQUAL(SYSTEM_TIMER_OUTPUT_MODE_NONE, GetTimerCompareOutputMode(curTimer, SYSTEM_TIMER_OUTPUT_A));

    TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_OFF, System_TimerGetClockSource(timerIdx));
    TEST_ASSERT_EQUAL(0, System_TimerGetCompareValue(timerIdx));
    TEST_ASSERT_EQUAL(SYSTEM_TIMER_OUTPUT_MODE_NONE, System_TimerGetCompareOutputMode(timerIdx));
    TEST_ASSERT_EQUAL(SYSTEM_TIMER_WAVEGEN_MODE_CTC, System_TimerGetWaveGenMode(timerIdx));
  }

  System_EventType eventIdx;
  for(
      eventIdx = 0;
      eventIdx < SYSTEM_NUM_EVENTS;
      eventIdx++
     )
  {
    TEST_ASSERT_FALSE(System_GetEvent(eventIdx));
    TEST_ASSERT_NULL(System_GetEventCallback(eventIdx));
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

  System_TimerID timerIdx;
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
  TEST_ASSERT_EQUAL(TIMER_STATUS_INVALID, GetTimerStatus(NULL));
}

TEST(TimerDriver, InvalidTimerStatus)
{
  testCreateAllTimers();
  TimerInstance* invalidTimer = timers[0];
  DestroyTimer(&timers[0]);

  TEST_ASSERT_EQUAL(TIMER_STATUS_INVALID, GetTimerStatus(invalidTimer));
}

TEST(TimerDriver, StoppedOnInit)
{
  testCreateAllTimers();

  TEST_ASSERT_EQUAL(TIMER_STATUS_STOPPED, GetTimerStatus(timers[0]));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_OFF, System_TimerGetClockSource(GetTimerSystemID(timers[0])));

  StartTimer(timers[0]);
  DestroyTimer(&timers[0]);

  timers[0] = CreateTimer();
  TEST_ASSERT_EQUAL(TIMER_STATUS_STOPPED, GetTimerStatus(timers[0]));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_OFF, System_TimerGetClockSource(GetTimerSystemID(timers[0])));
}

TEST(TimerDriver, ClearTimerOnCompareMatch)
{
  testCreateAllTimers();

  SetTimerCycleTimeMilliSec(
      timers[0],
      500
      );
  StartTimer(timers[0]);

  TEST_ASSERT_EQUAL(SYSTEM_TIMER_WAVEGEN_MODE_CTC, System_TimerGetWaveGenMode(GetTimerSystemID(timers[0])));
}

TEST(TimerDriver, StoppedOnDestroy)
{
  testCreateAllTimers();
  StartTimer(timers[0]);
  System_TimerID timerID = GetTimerSystemID(timers[0]);
  DestroyTimer(&timers[0]);

  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_OFF, System_TimerGetClockSource(timerID));
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

  TEST_ASSERT_EQUAL(TIMER_STATUS_RUNNING, GetTimerStatus(timers[0]));
  TEST_ASSERT(SYSTEM_TIMER_CLKSOURCE_OFF != System_TimerGetClockSource(GetTimerSystemID(timers[0])));
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

  TEST_ASSERT_EQUAL(TIMER_STATUS_STOPPED, GetTimerStatus(timers[0]));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_OFF, System_TimerGetClockSource(GetTimerSystemID(timers[0])));
}

TEST(TimerDriver, SetCycleTimeMilliSec)
{
  testCreateAllTimers();
  
  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 100));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, System_TimerGetClockSource(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(97, System_TimerGetCompareValue(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL(97, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(1, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 1));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE8, System_TimerGetClockSource(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(125, System_TimerGetCompareValue(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(125, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE8, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL(1, GetTimerCompareMatchesPerCycle(timers[0]));

  // Maximum number of milliseconds for 1MHz core clock without software divider
  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 262));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, System_TimerGetClockSource(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(255, System_TimerGetCompareValue(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL(255, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(1, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT_TRUE(SetTimerCycleTimeMilliSec(timers[0], 263));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, System_TimerGetClockSource(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(127, System_TimerGetCompareValue(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL(127, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(2, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT_TRUE(SetTimerCycleTimeMilliSec(timers[0], 500));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, System_TimerGetClockSource(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(244, System_TimerGetCompareValue(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL(244, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(2, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT_FALSE(SetTimerCycleTimeMilliSec(timers[0], 0));

  // Change clock to 8MHz
  System_SetCoreClockFrequency(8000000);
  
  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 100));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, System_TimerGetClockSource(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(195, System_TimerGetCompareValue(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL(195, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(4, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 1));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE64, System_TimerGetClockSource(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE64, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL(125, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(1, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 500));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, System_TimerGetClockSource(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(242, System_TimerGetCompareValue(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL(242, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(16, GetTimerCompareMatchesPerCycle(timers[0]));
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
  
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, System_TimerGetClockSource(GetTimerSystemID(timer)));
  TEST_ASSERT_EQUAL(242, System_TimerGetCompareValue(GetTimerSystemID(timer)));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timer));
  TEST_ASSERT_EQUAL(242, GetTimerCompareMatch(timer));
  TEST_ASSERT_EQUAL(16, GetTimerCompareMatchesPerCycle(timer));

  DestroyTimer(&timer);
}

TEST(TimerDriver, SetCycleTimeSec)
{
  testCreateAllTimers();

  TEST_ASSERT(SetTimerCycleTimeSec(timers[0], 1));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, System_TimerGetClockSource(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(244, System_TimerGetCompareValue(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL(244, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(4, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT(SetTimerCycleTimeSec(timers[0], 2));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, System_TimerGetClockSource(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(244, System_TimerGetCompareValue(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL(244, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(8, GetTimerCompareMatchesPerCycle(timers[0]));
}

TEST(TimerDriver, CycleTimeOverflow)
{
  testCreateAllTimers();

  TEST_ASSERT(
      SetTimerCycleTimeSec(
        timers[0],
        (unsigned int)((UINT_MAX) / 1000)
        )
      );

  TEST_ASSERT_FALSE(
      SetTimerCycleTimeSec(
        timers[0],
        (unsigned int)(((UINT_MAX) / 1000) + 1)
        )
      );
}

TEST(TimerDriver, FastClock)
{
  testCreateAllTimers();

  unsigned long int MAX_IDEAL_FREQ_MS_COUNTER = System_TimerGetMaxValue(GetTimerSystemID(timers[0])) * 1000;
  unsigned long int MAX_CORE_CLOCK_FREQ = ((MAX_IDEAL_FREQ_MS_COUNTER + 1) * 1024) - 1;

  // Maximum core clock frequency that timer can count 1 ms with
  System_SetCoreClockFrequency(MAX_CORE_CLOCK_FREQ);
  TEST_ASSERT(SetTimerCycleTimeSec(timers[0], 1));

  System_SetCoreClockFrequency(MAX_CORE_CLOCK_FREQ + 1);
  TEST_ASSERT_FALSE(SetTimerCycleTimeSec(timers[0], 1));
}

TEST(TimerDriver, MaxTimerValue)
{
  testCreateAllTimers();

  System_SetMaxTimerValue(
      GetTimerSystemID(timers[0]),
      65536 // 16-bit timer
      );
  
  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 100));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE8, System_TimerGetClockSource(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(12500, System_TimerGetCompareValue(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE8, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL(12500, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(1, GetTimerCompareMatchesPerCycle(timers[0]));
}

TEST(TimerDriver, HiFreqAccuracy)
{
  TEST_IGNORE_MESSAGE("Accuracy for long timers with high clock frequency not yet implemented.");

  testCreateAllTimers();

  // Change clock to 8MHz
  System_SetCoreClockFrequency(8000000);

  TEST_ASSERT(SetTimerCycleTimeSec(timers[0], 1));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, System_TimerGetClockSource(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(252, System_TimerGetCompareValue(GetTimerSystemID(timers[0])));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL(252, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(31, GetTimerCompareMatchesPerCycle(timers[0]));
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
  TEST_ASSERT_EQUAL(0, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL(0, GetNumTimerCycles(timers[0]));

  TEST_ASSERT(SetTimerCycleTimeSec(timers[0], 1));
  
  StartTimer(timers[0]);
  TEST_ASSERT_NOT_NULL(System_GetEventCallback(SYSTEM_EVENT_TIMER0_COMPAREMATCH));
  TEST_ASSERT_EQUAL(0, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL(0, GetNumTimerCycles(timers[0]));

  (*(System_GetEventCallback(SYSTEM_EVENT_TIMER0_COMPAREMATCH)))(SYSTEM_EVENT_TIMER0_COMPAREMATCH);
  TEST_ASSERT_EQUAL(1, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL(0, GetNumTimerCycles(timers[0]));

  (*(System_GetEventCallback(SYSTEM_EVENT_TIMER0_COMPAREMATCH)))(SYSTEM_EVENT_TIMER0_COMPAREMATCH);
  TEST_ASSERT_EQUAL(2, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL(0, GetNumTimerCycles(timers[0]));

  (*(System_GetEventCallback(SYSTEM_EVENT_TIMER0_COMPAREMATCH)))(SYSTEM_EVENT_TIMER0_COMPAREMATCH);
  TEST_ASSERT_EQUAL(3, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL(0, GetNumTimerCycles(timers[0]));

  (*(System_GetEventCallback(SYSTEM_EVENT_TIMER0_COMPAREMATCH)))(SYSTEM_EVENT_TIMER0_COMPAREMATCH);
  TEST_ASSERT_EQUAL(0, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL(1, GetNumTimerCycles(timers[0]));
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
  TEST_ASSERT_EQUAL(0, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL(0, GetNumTimerCycles(timers[0]));
  TEST_ASSERT_EQUAL(0, GetNumTimerCompareMatches(timers[1]));
  TEST_ASSERT_EQUAL(0, GetNumTimerCycles(timers[1]));

  (*(System_GetEventCallback(SYSTEM_EVENT_TIMER0_COMPAREMATCH)))(SYSTEM_EVENT_TIMER0_COMPAREMATCH);
  TEST_ASSERT_EQUAL(1, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL(0, GetNumTimerCycles(timers[0]));
  TEST_ASSERT_EQUAL(0, GetNumTimerCompareMatches(timers[1]));
  TEST_ASSERT_EQUAL(0, GetNumTimerCycles(timers[1]));

  (*(System_GetEventCallback(SYSTEM_EVENT_TIMER1_COMPAREMATCH)))(SYSTEM_EVENT_TIMER1_COMPAREMATCH);
  TEST_ASSERT_EQUAL(1, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL(0, GetNumTimerCycles(timers[0]));
  TEST_ASSERT_EQUAL(1, GetNumTimerCompareMatches(timers[1]));
  TEST_ASSERT_EQUAL(0, GetNumTimerCycles(timers[1]));

  (*(System_GetEventCallback(SYSTEM_EVENT_TIMER1_COMPAREMATCH)))(SYSTEM_EVENT_TIMER1_COMPAREMATCH);
  TEST_ASSERT_EQUAL(1, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL(0, GetNumTimerCycles(timers[0]));
  TEST_ASSERT_EQUAL(0, GetNumTimerCompareMatches(timers[1]));
  TEST_ASSERT_EQUAL(1, GetNumTimerCycles(timers[1]));

  (*(System_GetEventCallback(SYSTEM_EVENT_TIMER0_COMPAREMATCH)))(SYSTEM_EVENT_TIMER0_COMPAREMATCH);
  TEST_ASSERT_EQUAL(2, GetNumTimerCompareMatches(timers[0]));
  TEST_ASSERT_EQUAL(0, GetNumTimerCycles(timers[0]));
  TEST_ASSERT_EQUAL(0, GetNumTimerCompareMatches(timers[1]));
  TEST_ASSERT_EQUAL(1, GetNumTimerCycles(timers[1]));
}

TEST(TimerDriver, CompareOutputMode)
{
  testCreateAllTimers();

  System_TimerID timerIdx;
  for(
      timerIdx = 0;
      timerIdx < SYSTEM_NUM_TIMERS;
      timerIdx++
     )
  {
    TEST_ASSERT_EQUAL(SYSTEM_TIMER_OUTPUT_MODE_NONE, GetTimerCompareOutputMode(timers[timerIdx], SYSTEM_TIMER_OUTPUT_A));
    TEST_ASSERT_EQUAL(SYSTEM_TIMER_OUTPUT_MODE_NONE, System_TimerGetCompareOutputMode(timerIdx));
  }

  TEST_ASSERT(SetTimerCompareOutputMode(timers[0], SYSTEM_TIMER_OUTPUT_A, SYSTEM_TIMER_OUTPUT_MODE_SET));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_OUTPUT_MODE_SET, GetTimerCompareOutputMode(timers[0], SYSTEM_TIMER_OUTPUT_A));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_OUTPUT_MODE_SET, System_TimerGetCompareOutputMode(GetTimerSystemID(timers[0])));
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
  TEST_ASSERT_EQUAL(1, GetNumTimerCycles(timers[0]));
  TEST_ASSERT_EQUAL(1, numCustomTimerCycles);
}

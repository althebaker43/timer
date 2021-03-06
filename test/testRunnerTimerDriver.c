#include "unity_fixture.h"

TEST_GROUP_RUNNER(TimerDriver)
{
  RUN_TEST_CASE(TimerDriver, NoTimersBeforeInit);
  RUN_TEST_CASE(TimerDriver, MultiInit);
  RUN_TEST_CASE(TimerDriver, CreateTimer);
  RUN_TEST_CASE(TimerDriver, DestroyTimer);
  RUN_TEST_CASE(TimerDriver, DestroyAllTimers);
  RUN_TEST_CASE(TimerDriver, NotEnoughHardware);
  RUN_TEST_CASE(TimerDriver, TrackNumOfTimers);
  RUN_TEST_CASE(TimerDriver, NullTimerStatus);
  RUN_TEST_CASE(TimerDriver, InvalidTimerStatus);
  RUN_TEST_CASE(TimerDriver, StoppedOnInit);
  RUN_TEST_CASE(TimerDriver, ClearTimerOnCompareMatch);
  RUN_TEST_CASE(TimerDriver, StoppedOnDestroy);
  RUN_TEST_CASE(TimerDriver, NoRunningWithoutTime);
  RUN_TEST_CASE(TimerDriver, RunningAfterStart);
  RUN_TEST_CASE(TimerDriver, NoPowerReductionAfterStart);
  RUN_TEST_CASE(TimerDriver, StoppedAfterStop);
  RUN_TEST_CASE(TimerDriver, SetCycleTimeMilliSec);
  RUN_TEST_CASE(TimerDriver, ClockSourceSelection);
  RUN_TEST_CASE(TimerDriver, SetCycleTimeSec);
  RUN_TEST_CASE(TimerDriver, CycleTimeOverflow);
  RUN_TEST_CASE(TimerDriver, HiFreqAccuracy);
  RUN_TEST_CASE(TimerDriver, FastClock);
  RUN_TEST_CASE(TimerDriver, MaxTimerValue);
  RUN_TEST_CASE(TimerDriver, EnableCompareMatchEvents);
  RUN_TEST_CASE(TimerDriver, CountUpOnCompareMatch);
  RUN_TEST_CASE(TimerDriver, CompareMatchMultiTimers);
  RUN_TEST_CASE(TimerDriver, CompareOutputMode);
  RUN_TEST_CASE(TimerDriver, CustomCycleHandler);
  RUN_TEST_CASE(TimerDriver, SingleShot);
  RUN_TEST_CASE(TimerDriver, SingleShotAutoStart);
  RUN_TEST_CASE(TimerDriver, NoSingleShotWithoutConfig);
  RUN_TEST_CASE(TimerDriver, StopAfterSingleShot);
  RUN_TEST_CASE(TimerDriver, ResetOnNextSingleShot);
}

static void RunAllTests()
{
  RUN_TEST_GROUP(TimerDriver);
}

int main(
    int argc,
    char** argv
    )
{
  return UnityMain(argc, argv, RunAllTests);
}

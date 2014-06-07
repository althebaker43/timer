#include <stdlib.h>
#include "unity_fixture.h"
#include "TimerDriver.h"
#include "TargetSystem.h"

TEST_GROUP(TimerDriver);

static TimerInstance** timers = NULL;

///**
// * General timer/counter control register
// */
//static char GTCCR;
//enum GTCCR_Bits
//{
//  PSR0 = 0, /**< Prescaler reset timer/counter 0 */
//  PSR1,
//  FOC1A,
//  FOC1B,
//  COM1B0,
//  COM1B1,
//  PWM1B,
//  TSM       /**< Timer/counter synchronization mode */
//};
//
///**
// * Timer 0 control register A
// */
//static char TCCR0A;
//enum TCCR0A_Bits
//{
//  WGM00 = 0,
//  WGM01,
//  COM0B0 = 4,
//  COM0B1,
//  COM0A0,
//  COM0A1
//};

/**
 * Timer 0 control register B
 */
static char TCCR0B;
enum TCCR0B_Bits
{
  CS00 = 0,
  CS01,
  CS02,
//  WGM02,
//  FOC0B = 6,
//  FOC0A
};

///**
// * Timer 0 register
// */
//static char TCNT0;
//
///**
// * Output compare register A
// */
//static char OCR0A;
//
///**
// * Output compare register B
// */
//static char OCR0B;
//
///**
// * Timer 0 interrupt mask register
// */
//static char TIMSK;
//enum TIMSK_Bits
//{
//  TOIE0 = 1,
//  TOIE1,
//  OCIE0B,
//  OCIE0A,
//  OCIE1B,
//  OCIE1A
//};
//
///**
// * Timer 0 interrupt flag register
// */
//static char TIFR;
//enum TIFR_Bits
//{
//  TOV0 = 1,
//  TOV1,
//  OCF0B,
//  OCF0A,
//  OCF1B,
//  OCF1A
//};

inline void System_TimerSelectClock(
    int clock_source
    )
{
  switch (clock_source)
  {
    case TIMER_CLOCK_SELECT_OFF:  TCCR0B = 0x00; break;
    case TIMER_CLOCK_SELECT_ON:   TCCR0B = 0xFF; break;
    default: break;
  };
}

static void testCreateAllTimers()
{
  InitTimers();
  timers = (TimerInstance**)malloc((sizeof(TimerInstance*)) * SYSTEM_NUM_TIMERS);

  int timerIdx;
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

  int timerIdx;
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
}

TEST_TEAR_DOWN(TimerDriver)
{
  testDestroyAllTimers();
  DestroyAllTimers();
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

  int timerIdx;
  for(
      timerIdx = 0;
      timerIdx < SYSTEM_NUM_TIMERS;
      timerIdx++
     )
  {
    TEST_ASSERT_NOT_NULL(timers[timerIdx]);
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

  int timerIdx;
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
  TEST_ASSERT_EQUAL(GetTimerStatus(NULL), TIMER_STATUS_INVALID);
}

TEST(TimerDriver, InvalidTimerStatus)
{
  testCreateAllTimers();
  TimerInstance* invalidTimer = timers[0];
  DestroyTimer(&timers[0]);

  TEST_ASSERT_EQUAL(GetTimerStatus(invalidTimer), TIMER_STATUS_INVALID);
}

TEST(TimerDriver, StoppedOnInit)
{
  testCreateAllTimers();

  TEST_ASSERT_EQUAL(GetTimerStatus(timers[0]), TIMER_STATUS_STOPPED);
  TEST_ASSERT_BITS_LOW(((1<<CS02) | (1<<CS01) | (1<<CS00)), TCCR0B);

  StartTimer(timers[0]);
  DestroyTimer(&timers[0]);

  timers[0] = CreateTimer();
  TEST_ASSERT_EQUAL(GetTimerStatus(timers[0]), TIMER_STATUS_STOPPED);
  TEST_ASSERT_BITS_LOW(((1<<CS02) | (1<<CS01) | (1<<CS00)), TCCR0B);
}

TEST(TimerDriver, StoppedOnDestroy)
{
  testCreateAllTimers();
  StartTimer(timers[0]);
  DestroyTimer(&timers[0]);

  TEST_ASSERT_BITS_LOW(((1<<CS02) | (1<<CS01) | (1<<CS00)), TCCR0B);
}

TEST(TimerDriver, RunningAfterStart)
{
  testCreateAllTimers();
  StartTimer(timers[0]);

  TEST_ASSERT_EQUAL(GetTimerStatus(timers[0]), TIMER_STATUS_RUNNING);
  TEST_ASSERT((TCCR0B & ((1<<CS02) | (1<<CS01) | (1<<CS00))) != 0);
}

TEST(TimerDriver, StoppedAfterStop)
{
  testCreateAllTimers();
  StartTimer(timers[0]);
  StopTimer(timers[0]);

  TEST_ASSERT_EQUAL(GetTimerStatus(timers[0]), TIMER_STATUS_STOPPED);
  TEST_ASSERT_BITS_LOW(((1<<CS02) | (1<<CS01) | (1<<CS00)), TCCR0B);
}

TEST(TimerDriver, SetCycleTime)
{
  TEST_IGNORE();
}

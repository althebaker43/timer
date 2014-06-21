#include <stdlib.h>
#include <stdint.h>
#include "unity_fixture.h"
#include "TimerDriver.h"
#include "TargetSystem.h"

TEST_GROUP(TimerDriver);

static TimerInstance** timers = NULL;

/**
 * General timer/counter control register
 */
static uint8_t GTCCR;
enum GTCCR_Bits
{
  PSR0 = 0, /**< Prescaler reset timer/counter 0 */
  PSR1,
  FOC1A,
  FOC1B,
  COM1B0,
  COM1B1,
  PWM1B,
  TSM       /**< Timer/counter synchronization mode */
};

/**
 * Timer 0 control register A
 */
static uint8_t TCCR0A;
enum TCCR0A_Bits
{
  WGM00 = 0,
  WGM01,
  COM0B0 = 4,
  COM0B1,
  COM0A0,
  COM0A1
};

/**
 * Timer 0 control register B
 */
static uint8_t TCCR0B;
enum TCCR0B_Bits
{
  CS00 = 0,
  CS01,
  CS02,
  WGM02,
  FOC0B = 6,
  FOC0A
};

/**
 * Timer 0 register
 */
static uint8_t TCNT0;

/**
 * Output compare register A
 */
static uint8_t OCR0A;

/**
 * Output compare register B
 */
static uint8_t OCR0B;

/**
 * Timer 0 interrupt mask register
 */
static uint8_t TIMSK;
enum TIMSK_Bits
{
  TOIE0 = 1,
  TOIE1,
  OCIE0B,
  OCIE0A,
  OCIE1B,
  OCIE1A
};

/**
 * Timer 0 interrupt flag register
 */
static uint8_t TIFR;
enum TIFR_Bits
{
  TOV0 = 1,
  TOV1,
  OCF0B,
  OCF0A,
  OCF1B,
  OCF1A
};

/**
 * Power reduction register
 */
static uint8_t PRR;
enum PRR_Bits
{
  PRADC = 0,
  PRUSI,
  PRTIM0,
  PRTIM1
};

/**
 * Core clock frequency
 *
 * \note The default is 1MHz
 */
static unsigned long int coreClockFrequency = 1000000;

inline unsigned int System_TimerGetSourceFrequency(
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

inline int System_TimerSetClockSource(
    System_TimerClockSource clockSource
    )
{
  switch (clockSource)
  {
    case SYSTEM_TIMER_CLKSOURCE_OFF:
      TCCR0B = 0;
      break;

    case SYSTEM_TIMER_CLKSOURCE_INT:
      TCCR0B = (1<<CS00);
      break;

    case SYSTEM_TIMER_CLKSOURCE_INT_PRE8:
      TCCR0B = (1<<CS01);
      break;

    case SYSTEM_TIMER_CLKSOURCE_INT_PRE64:
      TCCR0B = (1<<CS01) | (1<<CS00);
      break;

    case SYSTEM_TIMER_CLKSOURCE_INT_PRE256:
      TCCR0B = (1<<CS02);
      break;

    case SYSTEM_TIMER_CLKSOURCE_INT_PRE1024:
      TCCR0B = (1<<CS02) | (1<<CS00);
      break;

    default:
      return FALSE;
      break;
  };

  return TRUE;
}

inline int System_TimerSetCompareMatch(
    uint8_t compareValue
    )
{
  OCR0A = compareValue;
  return TRUE;
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
  GTCCR = 0;
  TCCR0A = 0;
  TCCR0B = 0;
  TCNT0 = 0;
  OCR0A = 0;
  OCR0B = 0;
  TIMSK = 0;
  TIFR = 0;
  PRR = 0;
  coreClockFrequency = 1000000; // Default core frequency to 1MHz
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
  TEST_ASSERT_BITS_LOW(((1<<CS02) | (1<<CS01) | (1<<CS00)), TCCR0B);

  StartTimer(timers[0]);
  DestroyTimer(&timers[0]);

  timers[0] = CreateTimer();
  TEST_ASSERT_EQUAL(TIMER_STATUS_STOPPED, GetTimerStatus(timers[0]));
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

  TEST_ASSERT_EQUAL(TIMER_STATUS_RUNNING, GetTimerStatus(timers[0]));
  TEST_ASSERT((TCCR0B & ((1<<CS02) | (1<<CS01) | (1<<CS00))) != 0);
}

TEST(TimerDriver, NoPowerReductionAfterStart)
{
  testCreateAllTimers();
  StartTimer(timers[0]);

  TEST_ASSERT_BITS_LOW(((1<<PRTIM1) | (1<<PRTIM0)), PRR);
}

TEST(TimerDriver, StoppedAfterStop)
{
  testCreateAllTimers();
  StartTimer(timers[0]);
  StopTimer(timers[0]);

  TEST_ASSERT_EQUAL(TIMER_STATUS_STOPPED, GetTimerStatus(timers[0]));
  TEST_ASSERT_BITS_LOW(((1<<CS02) | (1<<CS01) | (1<<CS00)), TCCR0B);
}

TEST(TimerDriver, SetCycleTimeMilliSec)
{
  testCreateAllTimers();
  
  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 100));
  TEST_ASSERT_EQUAL_UINT8(0x05, (TCCR0B & ((1<<CS02) | (1<<CS01) | (1<<CS00))));
  TEST_ASSERT_EQUAL_UINT8(97, OCR0A);
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(97, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(1, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 1));
  TEST_ASSERT_EQUAL_UINT8(0x02, (TCCR0B & ((1<<CS02) | (1<<CS01) | (1<<CS00))));
  TEST_ASSERT_EQUAL_UINT8(125, OCR0A);
  TEST_ASSERT_EQUAL_UINT8(125, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE8, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL(1, GetTimerCompareMatchesPerCycle(timers[0]));

  // Maximum number of milliseconds for 1MHz core clock without software divider
  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 262));
  TEST_ASSERT_EQUAL_UINT8(0x05, (TCCR0B & ((1<<CS02) | (1<<CS01) | (1<<CS00))));
  TEST_ASSERT_EQUAL_UINT8(255, OCR0A);
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(255, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(1, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT_TRUE(SetTimerCycleTimeMilliSec(timers[0], 263));
  TEST_ASSERT_EQUAL_UINT8(0x05, (TCCR0B & ((1<<CS02) | (1<<CS01) | (1<<CS00))));
  TEST_ASSERT_EQUAL_UINT8(127, OCR0A);
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(127, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(2, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT_TRUE(SetTimerCycleTimeMilliSec(timers[0], 500));
  TEST_ASSERT_EQUAL_UINT8(0x05, (TCCR0B & ((1<<CS02) | (1<<CS01) | (1<<CS00))));
  TEST_ASSERT_EQUAL_UINT8(244, OCR0A);
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(244, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(2, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT_FALSE(SetTimerCycleTimeMilliSec(timers[0], 0));

  // Change clock to 8MHz
  coreClockFrequency = 8000000;
  
  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 100));
  TEST_ASSERT_EQUAL_UINT8(0x05, (TCCR0B & ((1<<CS02) | (1<<CS01) | (1<<CS00))));
  TEST_ASSERT_EQUAL_UINT8(195, OCR0A);
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(195, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(4, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT(SetTimerCycleTimeMilliSec(timers[0], 1));
  TEST_ASSERT_EQUAL_UINT8(0x03, (TCCR0B & ((1<<CS02) | (1<<CS01) | (1<<CS00))));
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE64, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(125, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(1, GetTimerCompareMatchesPerCycle(timers[0]));
}

TEST(TimerDriver, SetCycleTimeSec)
{
  testCreateAllTimers();

  TEST_ASSERT(SetTimerCycleTimeSec(timers[0], 1));
  TEST_ASSERT_EQUAL_UINT8(0x05, (TCCR0B & ((1<<CS02) | (1<<CS01) | (1<<CS00))));
  TEST_ASSERT_EQUAL_UINT8(244, OCR0A);
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(244, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(4, GetTimerCompareMatchesPerCycle(timers[0]));

  TEST_ASSERT(SetTimerCycleTimeSec(timers[0], 2));
  TEST_ASSERT_EQUAL_UINT8(0x05, (TCCR0B & ((1<<CS02) | (1<<CS01) | (1<<CS00))));
  TEST_ASSERT_EQUAL_UINT8(244, OCR0A);
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(244, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(8, GetTimerCompareMatchesPerCycle(timers[0]));
}

TEST(TimerDriver, HiFreqAccuracy)
{
  TEST_IGNORE_MESSAGE("Accuracy for long timers with high clock frequency not yet implemented.");

  testCreateAllTimers();

  // Change clock to 8MHz
  coreClockFrequency = 8000000;

  TEST_ASSERT(SetTimerCycleTimeSec(timers[0], 1));
  TEST_ASSERT_EQUAL_UINT8(0x05, (TCCR0B & ((1<<CS02) | (1<<CS01) | (1<<CS00))));
  TEST_ASSERT_EQUAL_UINT8(252, OCR0A);
  TEST_ASSERT_EQUAL(SYSTEM_TIMER_CLKSOURCE_INT_PRE1024, GetTimerClockSource(timers[0]));
  TEST_ASSERT_EQUAL_UINT8(252, GetTimerCompareMatch(timers[0]));
  TEST_ASSERT_EQUAL(31, GetTimerCompareMatchesPerCycle(timers[0]));
}

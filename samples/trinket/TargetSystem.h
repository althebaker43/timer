#ifndef TARGET_SYSTEM
#define TARGET_SYSTEM

#include <stdint.h>
#include <avr/io.h>

#define TRUE 1
#define FALSE 0

/**
 * Total number of timer modules in the system
 */
#define SYSTEM_NUM_TIMERS 1

#define SYSTEM_CORE_CLOCK_FREQUENCY 8000000

/**
 * Enumeration of different clock sources for timer 0
 *
 * \note These must be sorted from highest to lowest in frequency
 */
typedef enum System_TimerClockSource_enum
{
  SYSTEM_TIMER_CLKSOURCE_INT,
  SYSTEM_TIMER_CLKSOURCE_INT_PRE8,
  SYSTEM_TIMER_CLKSOURCE_INT_PRE64,
  SYSTEM_TIMER_CLKSOURCE_INT_PRE256,
  SYSTEM_TIMER_CLKSOURCE_INT_PRE1024,
  SYSTEM_TIMER_CLKSOURCE_OFF,         // Disconnected from clock
  NUM_TIMER_CLKSOURCES,
  SYSTEM_TIMER_CLKSOURCE_INVALID
} System_TimerClockSource;

/**
 * Enumeration of timer compare output pins
 */
typedef enum System_TimerCompareOutput_enum
{
  SYSTEM_TIMER_OUTPUT_A
} System_TimerCompareOutput;

/**
 * Enumeration of timer compare output modes
 */
typedef enum System_TimerCompareOutputMode_enum
{
  SYSTEM_TIMER_OUTPUT_MODE_NONE,   /**< Outputs disconnected */
  SYSTEM_TIMER_OUTPUT_MODE_SET,
  SYSTEM_TIMER_OUTPUT_MODE_CLEAR,
  SYSTEM_TIMER_OUTPUT_MODE_TOGGLE
} System_TimerCompareOutputMode;

/**
 * Provides the frequency in Hz for a given clock source
 *
 * \return Frequency of given clock source, or zero if invalid
 */
static inline uint32_t System_TimerGetSourceFrequency(
    System_TimerClockSource clockSource
    )
{
  switch (clockSource)
  {
    case SYSTEM_TIMER_CLKSOURCE_INT:          return SYSTEM_CORE_CLOCK_FREQUENCY; break;
    case SYSTEM_TIMER_CLKSOURCE_INT_PRE8:     return (SYSTEM_CORE_CLOCK_FREQUENCY / 8); break;
    case SYSTEM_TIMER_CLKSOURCE_INT_PRE64:    return (SYSTEM_CORE_CLOCK_FREQUENCY / 64); break;
    case SYSTEM_TIMER_CLKSOURCE_INT_PRE256:   return (SYSTEM_CORE_CLOCK_FREQUENCY / 256); break;
    case SYSTEM_TIMER_CLKSOURCE_INT_PRE1024:  return (SYSTEM_CORE_CLOCK_FREQUENCY / 1024); break;
    default:
      return 0;
      break;
  };
}

/**
 * Sets the clock source for a timer
 *
 * \return Nonzero if configuration was successful, zero otherwise
 */
static inline uint8_t System_TimerSetClockSource(
    System_TimerClockSource clockSource
    )
{
  TCCR0B &= ~((1<<CS02) | (1<<CS01) | (1<<CS00));

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

/**
 * Sets the timer compare match value
 *
 * \return Nonzero if configuration was successful, zero otherwise
 */
static inline uint8_t System_TimerSetCompareMatch(
    uint8_t compareValue
    )
{
  OCR0B = compareValue;
  return TRUE;
}

/**
 * Sets the timer compare output mode
 *
 * \return Nonzero if the configuration was successful, zero otherwise
 */
static inline uint8_t System_TimerSetCompareOutputMode(
    System_TimerCompareOutputMode outputMode
    )
{
  switch (outputMode)
  {
    case SYSTEM_TIMER_OUTPUT_MODE_NONE:
      TCCR0A = 0;
      break;

    case SYSTEM_TIMER_OUTPUT_MODE_SET:
      TCCR0A = (1<<COM0A1) | (1<<COM0A0);
      break;

    case SYSTEM_TIMER_OUTPUT_MODE_CLEAR:
      TCCR0A = (1<<COM0A1);
      break;

    case SYSTEM_TIMER_OUTPUT_MODE_TOGGLE:
      TCCR0A = (1<<COM0A0);
      break;

    default:
      return FALSE;
      break;
  };

  return TRUE;
}

#endif /* TARGET_SYSTEM */

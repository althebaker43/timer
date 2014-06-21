#ifndef TARGET_SYSTEM
#define TARGET_SYSTEM

#include <stdint.h>

#define TRUE 1
#define FALSE 0

/**
 * Total number of timer modules in the system
 */
#define SYSTEM_NUM_TIMERS 3

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
uint32_t System_TimerGetSourceFrequency(
    System_TimerClockSource
    );

/**
 * Sets the clock source for a timer
 *
 * \return Nonzero if configuration was successful, zero otherwise
 */
uint8_t System_TimerSetClockSource(
    System_TimerClockSource
    );

/**
 * Sets the timer compare match value
 *
 * \return Nonzero if configuration was successful, zero otherwise
 */
uint8_t System_TimerSetCompareMatch(
    uint8_t
    );

/**
 * Sets the timer compare output mode
 *
 * \return Nonzero if the configuration was successful, zero otherwise
 */
uint8_t System_TimerSetCompareOutputMode(
    System_TimerCompareOutputMode
    );

#endif /* TARGET_SYSTEM */

#ifndef TARGET_SYSTEM
#define TARGET_SYSTEM

#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>

#define TRUE 1
#define FALSE 0

#define SYSTEM_CORE_CLOCK_FREQUENCY 8000000

/**
 * Enumeration of all timer modules in system
 */
typedef enum System_TimerID_enum
{
  SYSTEM_TIMER0,
  SYSTEM_NUM_TIMERS
} System_TimerID;

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
 * Enumeration of timer waveform generation modes
 */
typedef enum System_TimerWaveGenMode_enum
{
  SYSTEM_TIMER_WAVEGEN_MODE_CTC /**< Clear timer on compare-match */
} System_TimerWaveGenMode;

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
 * Enumeration of all system events (interrupts)
 */
typedef enum System_EventType_enum
{
  SYSTEM_EVENT_TIMER0_COMPAREMATCH,
  SYSTEM_EVENT_TIMER1_COMPAREMATCH,
  SYSTEM_EVENT_TIMER2_COMPAREMATCH,
  SYSTEM_NUM_EVENTS,
  SYSTEM_EVENT_INVALID
} System_EventType;

/**
 * Typedef for system event callback functions
 */
typedef void (*System_EventCallback)(System_EventType);

/**
 * Provides the frequency in Hz for a given clock source
 *
 * \return Frequency of given clock source, or zero if invalid
 */
static inline unsigned long int
System_TimerGetSourceFrequency(
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
static inline unsigned int
System_TimerSetClockSource(
    System_TimerID          timer,
    System_TimerClockSource clockSource
    )
{
  TCCR0B &= ~((1<<CS02) | (1<<CS01) | (1<<CS00));

  switch (clockSource)
  {
    case SYSTEM_TIMER_CLKSOURCE_OFF:
      break;

    case SYSTEM_TIMER_CLKSOURCE_INT:
      TCCR0B |= (1<<CS00);
      break;

    case SYSTEM_TIMER_CLKSOURCE_INT_PRE8:
      TCCR0B |= (1<<CS01);
      break;

    case SYSTEM_TIMER_CLKSOURCE_INT_PRE64:
      TCCR0B |= (1<<CS01) | (1<<CS00);
      break;

    case SYSTEM_TIMER_CLKSOURCE_INT_PRE256:
      TCCR0B |= (1<<CS02);
      break;

    case SYSTEM_TIMER_CLKSOURCE_INT_PRE1024:
      TCCR0B |= (1<<CS02) | (1<<CS00);
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
static inline unsigned int
System_TimerSetCompareMatch(
    System_TimerID  timer,
    unsigned int    compareValue
    )
{
  OCR0A = compareValue;
  return TRUE;
}

/**
 * Sets the timer compare output mode
 *
 * \return Nonzero if the configuration was successful, zero otherwise
 */
static inline unsigned int
System_TimerSetCompareOutputMode(
    System_TimerID                timer,
    System_TimerCompareOutputMode outputMode
    )
{
  TCCR0A &= ~((1<<COM0A1) | (1<<COM0A0));

  switch (outputMode)
  {
    case SYSTEM_TIMER_OUTPUT_MODE_NONE:
      break;

    case SYSTEM_TIMER_OUTPUT_MODE_SET:
      TCCR0A |= (1<<COM0A1) | (1<<COM0A0);
      break;

    case SYSTEM_TIMER_OUTPUT_MODE_CLEAR:
      TCCR0A |= (1<<COM0A1);
      break;

    case SYSTEM_TIMER_OUTPUT_MODE_TOGGLE:
      TCCR0A |= (1<<COM0A0);
      break;

    default:
      return FALSE;
      break;
  };

  return TRUE;
}

/**
 * Sets the timer waveform generation mode
 *
 * \return Nonzero if the configuration was successful, zero otherwise
 */
static inline unsigned int
System_TimerSetWaveGenMode(
    System_TimerID          timer,
    System_TimerWaveGenMode waveGenMode
    )
{
  TCCR0A &= ~((1<<WGM01) | (1<<WGM00));
  TCCR0B &= ~((1<<WGM02));

  switch (waveGenMode)
  {
    case SYSTEM_TIMER_WAVEGEN_MODE_CTC:
      TCCR0A |= ((1<<WGM01));
      break;

    default:
      return FALSE;
      break;
  };

  return TRUE;
}

/**
 * Registers a callback function to call when a given event occurs
 */
void
System_RegisterCallback(
    System_EventCallback  callback, /**< Pointer to callback function to register */
    System_EventType      event     /**< Type of event to register the callback for */
    );

/**
 * Gets a callback registered with the given event, if any
 */
System_EventCallback
System_GetEventCallback(
    System_EventType  event /**< Event to get callback function for */
    );

/**
 * Enables interrupts for a given event
 */
static inline unsigned int
System_EnableEvent(
    System_EventType  event /**< Type of event to enable interrupts for */
    )
{
  switch (event)
  {
    case SYSTEM_EVENT_TIMER0_COMPAREMATCH: TIMSK |= (1<<OCIE0A);
    
    default:
      return FALSE;
      break;
  };

  return TRUE;
}

/**
 * Disables interrupts for a given event
 */
static inline unsigned int
System_DisableEvent(
    System_EventType  event /** Type of event to disable interrupts for */
    )
{
  switch (event)
  {
    case SYSTEM_EVENT_TIMER0_COMPAREMATCH: TIMSK &= ~((1<<OCIE0A));
    
    default:
      return FALSE;
      break;
  };

  return TRUE;
}

/**
 * Provides the callback event type for the given timer
 */
static inline System_EventType
System_GetTimerCallbackEvent(
    System_TimerID  timer
    )
{
  switch (timer)
  {
    case SYSTEM_TIMER0: return SYSTEM_EVENT_TIMER0_COMPAREMATCH; break;
    default: return SYSTEM_EVENT_INVALID; break;
  };
}

#endif /* TARGET_SYSTEM */

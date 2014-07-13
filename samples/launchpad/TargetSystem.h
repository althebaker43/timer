#ifndef TARGET_SYSTEM
#define TARGET_SYSTEM

#include <stdlib.h>
#include <stdint.h>
#include <msp430f5529.h>

#define TRUE 1
#define FALSE 0

#define SYSTEM_SUB_CLOCK_FREQUENCY 8000000 // Subsystem clock

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
  SYSTEM_TIMER_CLKSOURCE_SUB,
  SYSTEM_TIMER_CLKSOURCE_SUB_PRE2,
  SYSTEM_TIMER_CLKSOURCE_SUB_PRE4,
  SYSTEM_TIMER_CLKSOURCE_SUB_PRE8,
  SYSTEM_TIMER_CLKSOURCE_OFF,         // Disconnected from clock
  NUM_TIMER_CLKSOURCES,
  SYSTEM_TIMER_CLKSOURCE_INVALID
} System_TimerClockSource;

/**
 * Enumeration of timer compare output pins
 */
typedef enum System_TimerCompareOutput_enum
{
  SYSTEM_TIMER_OUTPUT_0
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
    case SYSTEM_TIMER_CLKSOURCE_SUB:      return (SYSTEM_SUB_CLOCK_FREQUENCY); break; 
    case SYSTEM_TIMER_CLKSOURCE_SUB_PRE2: return (SYSTEM_SUB_CLOCK_FREQUENCY / 2); break;
    case SYSTEM_TIMER_CLKSOURCE_SUB_PRE4: return (SYSTEM_SUB_CLOCK_FREQUENCY / 4); break;
    case SYSTEM_TIMER_CLKSOURCE_SUB_PRE8: return (SYSTEM_SUB_CLOCK_FREQUENCY / 8); break;
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
  // Stop timer when configuring clock source
  unsigned int TA0CTL_MC_copy = TA0CTL & ((MC1) | (MC0));
  TA0CTL &= ~((MC1) | (MC0));

  TA0CTL &= ~((ID1) | (ID0));

  switch (clockSource)
  {
    case SYSTEM_TIMER_CLKSOURCE_SUB:
      break;

    case SYSTEM_TIMER_CLKSOURCE_SUB_PRE2:
      TA0CTL |= (ID0);
      break;

    case SYSTEM_TIMER_CLKSOURCE_SUB_PRE4:
      TA0CTL |= (ID1);
      break;

    case SYSTEM_TIMER_CLKSOURCE_SUB_PRE8:
      TA0CTL |= (ID1) | (ID0);
      break;

    case SYSTEM_TIMER_CLKSOURCE_OFF:
      break;

    default:
      return FALSE;
      break;
  };

  // Reset divider logic
  TA0CTL |= TACLR;

  // Restart timer
  TA0CTL |= TA0CTL_MC_copy;

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
  TA0CCR0 = compareValue;
  TA0CCTL0 &= ~(CAP);
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
  TA0CCTL0 &= ~((OUTMOD2) | (OUTMOD1) | (OUTMOD0) | (OUT));
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
  TA0CTL &= ~((MC1) | (MC0));
  TA0CTL |= (MC0);
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
  // Stop timer when configuring clock source
  unsigned int TA0CTL_MC_copy = TA0CTL & ((MC1) | (MC0));
  TA0CTL &= ~((MC1) | (MC0));

  switch (event)
  {
    case SYSTEM_EVENT_TIMER0_COMPAREMATCH:
      TA0CCTL0 |= (CCIE);
      break;
    
    default:
      return FALSE;
      break;
  };

  // Restart timer
  TA0CTL |= TA0CTL_MC_copy;

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
  // Stop timer when configuring clock source
  unsigned int TA0CTL_MC_copy = TA0CTL & ((MC1) | (MC0));
  TA0CTL &= ~((MC1) | (MC0));

  switch (event)
  {
    case SYSTEM_EVENT_TIMER0_COMPAREMATCH:
      TA0CCTL0 &= ~(CCIE);
      break;

    default:
      return FALSE;
      break;
  };

  // Restart timer
  TA0CTL |= TA0CTL_MC_copy;

  return TRUE;
}

/**
 * Provides the callback event type for the given timer
 */
static inline System_EventType
System_GetTimerCallbackEvent(
    System_TimerID  timerID
    )
{
  switch (timerID)
  {
    case SYSTEM_TIMER0: return SYSTEM_EVENT_TIMER0_COMPAREMATCH; break;
    
    default:
      return SYSTEM_EVENT_INVALID;
      break;
  };
}

#endif /* TARGET_SYSTEM */

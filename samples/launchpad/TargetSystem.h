#ifndef TARGET_SYSTEM
#define TARGET_SYSTEM

#include <stdlib.h>
#include <stdint.h>
#include <msp430f5529.h>

#define TRUE 1
#define FALSE 0

#define SYSTEM_SUB_CLOCK_FREQUENCY 1048578 // Subsystem clock

/**
 * Enumeration of all timer modules in system
 */
typedef enum System_TimerID_enum
{
  SYSTEM_TIMER0,
  SYSTEM_TIMER1,
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
  SYSTEM_EVENT_TIMER1_COMPAREMATCH,
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
 * Provides the maximum counter value for the given timer
 *
 * \return Maximum value of timer with given ID
 */
static inline unsigned long int
System_TimerGetMaxValue(
    System_TimerID  timer
    )
{
  return 65535;
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
  // Initialize copy of timer control register
  unsigned int TACTL_copy = 0;

  switch (clockSource)
  {
    case SYSTEM_TIMER_CLKSOURCE_SUB:
      break;

    case SYSTEM_TIMER_CLKSOURCE_SUB_PRE2:
      TACTL_copy |= (ID0);
      break;

    case SYSTEM_TIMER_CLKSOURCE_SUB_PRE4:
      TACTL_copy |= (ID1);
      break;

    case SYSTEM_TIMER_CLKSOURCE_SUB_PRE8:
      TACTL_copy |= (ID1) | (ID0);
      break;

    case SYSTEM_TIMER_CLKSOURCE_OFF:
      break;

    default:
      return FALSE;
      break;
  };

  unsigned int TACTL_MC_copy = 0;
  switch (timer)
  {
    case SYSTEM_TIMER0:
      // Stop timer when configuring clock source
      TACTL_MC_copy = TA0CTL & ((MC1) | (MC0));
      TA0CTL &= ~((MC1) | (MC0));

      // Run off submaster clock
      TA0CTL &= ~((TASSEL1) | (TASSEL0));
      TA0CTL |= (TASSEL1);

      // Set input clock frequency divider
      TA0CTL &= ~((ID1) | (ID0));
      TA0CTL |= TACTL_copy;

      // Reset divider logic
      TA0CTL |= TACLR;

      // Restart timer
      TA0CTL |= TACTL_MC_copy;
      break;

    case SYSTEM_TIMER1:
      // Stop timer when configuring clock source
      TACTL_MC_copy = TA1CTL & ((MC1) | (MC0));
      TA1CTL &= ~((MC1) | (MC0));

      // Run off submaster clock
      TA1CTL &= ~((TASSEL1) | (TASSEL0));
      TA1CTL |= (TASSEL1);

      // Set input clock frequency divider
      TA1CTL &= ~((ID1) | (ID0));
      TA1CTL |= TACTL_copy;

      // Reset divider logic
      TA1CTL |= TACLR;

      // Restart timer
      TA1CTL |= TACTL_MC_copy;
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
  switch (timer)
  {
    case SYSTEM_TIMER0:
      TA0CCR0 = compareValue;
      TA0CCTL0 &= ~(CAP);
      break;
    
    case SYSTEM_TIMER1:
      TA1CCR0 = compareValue;
      TA1CCTL0 &= ~(CAP);
      break;

    default:
      return FALSE;
      break;
  };

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
  switch (timer)
  {
    case SYSTEM_TIMER0:
      TA0CCTL0 &= ~((OUTMOD2) | (OUTMOD1) | (OUTMOD0) | (OUT));
      break;

    case SYSTEM_TIMER1:
      TA1CCTL0 &= ~((OUTMOD2) | (OUTMOD1) | (OUTMOD0) | (OUT));
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
  switch (timer)
  {
    case SYSTEM_TIMER0:
      TA0CTL &= ~((MC1) | (MC0));
      TA0CTL |= (MC0);
      break;

    case SYSTEM_TIMER1:
      TA1CTL &= ~((MC1) | (MC0));
      TA1CTL |= (MC0);
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
  unsigned int TACTL_MC_copy = 0;

  switch (event)
  {
    case SYSTEM_EVENT_TIMER0_COMPAREMATCH:
      // Stop timer when configuring clock source
      TACTL_MC_copy = TA0CTL & ((MC1) | (MC0));
      TA0CTL &= ~((MC1) | (MC0));

      TA0CCTL0 |= (CCIE);

      // Restart timer
      TA0CTL |= TACTL_MC_copy;
      break;
    
    case SYSTEM_EVENT_TIMER1_COMPAREMATCH:
      // Stop timer when configuring clock source
      TACTL_MC_copy = TA1CTL & ((MC1) | (MC0));
      TA1CTL &= ~((MC1) | (MC0));

      TA1CCTL0 |= (CCIE);

      // Restart timer
      TA1CTL |= TACTL_MC_copy;
      break;
    
    default:
      return FALSE;
      break;
  };

  // Set global interrupt enable
  __eint();

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
  unsigned int TACTL_MC_copy = 0;

  switch (event)
  {
    case SYSTEM_EVENT_TIMER0_COMPAREMATCH:
      // Stop timer when configuring clock source
      TACTL_MC_copy = TA0CTL & ((MC1) | (MC0));
      TA0CTL &= ~((MC1) | (MC0));

      TA0CCTL0 &= ~(CCIE);

      // Restart timer
      TA0CTL |= TACTL_MC_copy;
      break;
    
    case SYSTEM_EVENT_TIMER1_COMPAREMATCH:
      // Stop timer when configuring clock source
      TACTL_MC_copy = TA1CTL & ((MC1) | (MC0));
      TA1CTL &= ~((MC1) | (MC0));

      TA1CCTL0 &= ~(CCIE);

      // Restart timer
      TA1CTL |= TACTL_MC_copy;
      break;
    
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
    System_TimerID  timerID
    )
{
  switch (timerID)
  {
    case SYSTEM_TIMER0: return SYSTEM_EVENT_TIMER0_COMPAREMATCH; break;
    case SYSTEM_TIMER1: return SYSTEM_EVENT_TIMER1_COMPAREMATCH; break;
    
    default:
      return SYSTEM_EVENT_INVALID;
      break;
  };
}

#endif /* TARGET_SYSTEM */

#ifndef TARGET_SYSTEM
#define TARGET_SYSTEM

#include <stdint.h>

#define TRUE 1
#define FALSE 0

/**
 * Enumeration of all timer modules in system
 */
typedef enum System_TimerID_enum
{
  SYSTEM_TIMER0,
  SYSTEM_TIMER1,
  SYSTEM_TIMER2,
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
 * Enumeration of timer waveform generation modes
 */
typedef enum System_TimerWaveGenMode_enum
{
  SYSTEM_TIMER_WAVEGEN_MODE_CTC /**< Clear timer on compare-match */
} System_TimerWaveGenMode;

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

/**
 * Sets the timer waveform generation mode
 *
 * \return Nonzero if the configuration was successful, zero otherwise
 */
uint8_t
System_TimerSetWaveGenMode(
    System_TimerWaveGenMode
    );

/**
 * Registers a callback function to call when a given event occurs
 */
void
System_RegisterCallback(
    System_EventCallback  callback,     /**< Pointer to callback function to register */
    System_EventType      event         /**< Type of event to register the callback for */
    );

/**
 * Gets a callback registered with the given event, if any
 */
System_EventCallback
System_GetCallback(
    System_EventType  event /**< Event to get callback function for */
    );

/**
 * Enables interrupts for a given event
 */
uint8_t
System_EnableEvent(
    System_EventType  event /**< Type of event to enable interrupts for */
    );

/**
 * Disables interrupts for a given event
 */
uint8_t
System_DisableEvent(
    System_EventType  event /** Type of event to disable interrupts for */
    );

/**
 * Provides the callback event type for the given timer
 */
System_EventType
System_GetTimerCallbackEvent(
    System_TimerID  timerID
    );

/**
 * Records that the specified event occurred
 */
void
System_SetEvent(
    System_EventType  event /**< Type of event to record */
    );

/**
 * Gets the next event that was last recorded
 */
System_EventType
System_PopEvent();

// Test accessors (not for production use)

System_TimerClockSource
System_TimerGetClockSource();

uint8_t
System_TimerGetCompareValue();

System_TimerCompareOutputMode
System_TimerGetCompareOutputMode();

System_TimerWaveGenMode
System_TimerGetWaveGenMode();

uint8_t
System_GetEvent(
    System_EventType
    );

System_EventCallback
System_GetEventCallback(
    System_EventType
    );

// Test manipulators (not for production use)

void
System_SetCoreClockFrequency(
    uint32_t
    );

#endif /* TARGET_SYSTEM */

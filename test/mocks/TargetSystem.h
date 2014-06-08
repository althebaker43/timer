#ifndef TARGET_SYSTEM
#define TARGET_SYSTEM

#define TRUE 1
#define FALSE 0

/**
 * Total number of timer modules in the system
 */
#define SYSTEM_NUM_TIMERS 3

/**
 * Frequency of main core clock
 */
#define SYSTEM_CORE_CLOCK_FREQ 1000000

/**
 * Frequency of I/O clock
 */
#define SYSTEM_IO_CLOCK_FREQ 1000000

/**
 * Number of prescaler options for timers
 */
#define SYSTEM_NUM_TIMER_PRESCALERS 5

/**
 * Enumeration of different clock sources for timer 0
 */
enum System_TimerClockSources
{
  TIMER_CLOCK_SELECT_OFF,
  TIMER_CLOCK_SELECT_ON,
  TIMER_CLOCK_SELECT_ON_PRE8,
  TIMER_CLOCK_SELECT_ON_PRE64,
  TIMER_CLOCK_SELECT_ON_PRE256,
  TIMER_CLOCK_SELECT_ON_PRE1024
};

/**
 * Hardware prescaler options available for timer
 */
unsigned int System_TimerHWPrescalers [SYSTEM_NUM_TIMER_PRESCALERS];

/**
 * Sets the clock source for a timer
 */
void System_TimerSelectClock(int);

/**
 * Sets the timer output compare value
 */
void System_TimerSetOutputCompare(char);

#endif /* TARGET_SYSTEM */

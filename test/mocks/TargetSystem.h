#define TRUE 1
#define FALSE 0

#define SYSTEM_NUM_TIMERS 3

/**
 * Enumeration of different timer clock sources
 */
enum System_TimerClockSources
{
  TIMER_CLOCK_SELECT_OFF,
  TIMER_CLOCK_SELECT_ON
};

/**
 * Sets the clock source for a timer
 */
void System_TimerSelectClock(int);

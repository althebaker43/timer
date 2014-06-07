/**
 * Timer context structure typedef
 */
typedef struct TimerInstance_struct TimerInstance;

/**
 * Enumeration of all possible timer states
 */
typedef enum TimerStatus_enum
{
  TIMER_STATUS_INVALID,
  TIMER_STATUS_STOPPED,
  TIMER_STATUS_RUNNING
} TimerStatus;

/**
 * Initializes the timer driver
 *
 * This only needs to be called once after system reset
 */
void InitTimers();

/**
 * Allocates a new timer context (if possible)
 *
 * \return Pointer to new context, or NULL if not created
 */
TimerInstance* CreateTimer();

/**
 * Destroys a given timer context
 */
void DestroyTimer(TimerInstance**);

/**
 * Destroys all timers in use
 *
 * \note All existing TimerInstance pointers are invalidated by this function
 */
void DestroyAllTimers();

/**
 * Provides the given timer's status
 *
 * \return Status of the given timer
 */
TimerStatus GetTimerStatus(TimerInstance*);

/**
 * Starts the given timer, if not already running
 */
void StartTimer(TimerInstance*);

/**
 * Stops the given timer, if not already stopped
 */
void StopTimer(TimerInstance*);

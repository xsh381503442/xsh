#ifndef __DRV_RTC_TIMER_H__
#define __DRV_RTC_TIMER_H__


#include "app_error.h"
#include "app_util.h"
#include "compiler_abstraction.h"
#include "nordic_common.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>




#define DRV_RTC_TIMER_CLOCK_FREQ            32768                     /**< Clock frequency of the RTC timer used to implement the app timer module. */
#define DRV_RTC_TIMER_MIN_TIMEOUT_TICKS     5                         /**< Minimum value of the timeout_ticks parameter of app_timer_start(). */

#ifdef RTX
#define DRV_RTC_TIMER_NODE_SIZE             40                        /**< Size of app_timer.timer_node_t (used to allocate data). */
#else
#define DRV_RTC_TIMER_NODE_SIZE             32                        /**< Size of app_timer.timer_node_t (used to allocate data). */
#endif // RTX

#define DRV_RTC_TIMER_SCHED_EVENT_DATA_SIZE sizeof(drv_rtc_timer_event_t) /**< Size of event data when scheduler is used. */

/**@brief Convert milliseconds to timer ticks.
 *
 * This macro uses 64-bit integer arithmetic, but as long as the macro parameters are
 *       constants (i.e. defines), the computation will be done by the preprocessor.
 *
 * @param[in]  MS          Milliseconds.
 *
 * @return     Number of timer ticks.
 */



#define DRV_RTC_TIMER_TICKS_1HZ(MS)                                \
            ((uint32_t)ROUNDED_DIV(                        \
            (MS) * (uint64_t)DRV_RTC_TIMER_CLOCK_FREQ,         \
            1000 * (APP_TIMER_CONFIG_RTC_FREQUENCY + 1))) + 4 //加上4个TICK保证定时器在1秒产生一次中断

#define DRV_RTC_TIMER_TICKS_2HZ(MS)                                \
            ((uint32_t)ROUNDED_DIV(                        \
            (MS) * (uint64_t)DRV_RTC_TIMER_CLOCK_FREQ,         \
            1000 * (APP_TIMER_CONFIG_RTC_FREQUENCY + 1))) + 2 //加上2个TICK保证定时器在500.01微秒产生一次中断
						
#define DRV_RTC_TIMER_TICKS_4HZ(MS)                                \
            ((uint32_t)ROUNDED_DIV(                        \
            (MS) * (uint64_t)DRV_RTC_TIMER_CLOCK_FREQ,         \
            1000 * (APP_TIMER_CONFIG_RTC_FREQUENCY + 1))) + 1 //加上1个TICK保证定时器在250微秒产生一次中断						
						
#define DRV_RTC_TIMER_TICKS_8HZ(MS)                                \
            ((uint32_t)ROUNDED_DIV(                        \
            (MS) * (uint64_t)DRV_RTC_TIMER_CLOCK_FREQ,         \
            1000 * (APP_TIMER_CONFIG_RTC_FREQUENCY + 1))) + 1 //加上1个TICK保证定时器在125.02微秒产生一次中断

//#define DRV_RTC_TIMER_TICKS(MS)                                \
//            ((uint32_t)ROUNDED_DIV(                        \
//            (MS) * (uint64_t)DRV_RTC_TIMER_CLOCK_FREQ,         \
//            1000 * (APP_TIMER_CONFIG_RTC_FREQUENCY + 1))) 


typedef struct drv_rtc_timer_t { uint32_t data[CEIL_DIV(DRV_RTC_TIMER_NODE_SIZE, sizeof(uint32_t))]; } drv_rtc_timer_t;

/**@brief Timer ID type.
 * Never declare a variable of this type, but use the macro @ref APP_TIMER_DEF instead.*/
typedef drv_rtc_timer_t * drv_rtc_timer_id_t;


/**
 * @brief Create a timer identifier and statically allocate memory for the timer.
 *
 * @param timer_id Name of the timer identifier variable that will be used to control the timer.
 */
#define DRV_RTC_TIMER_DEF(timer_id)                                      \
    static drv_rtc_timer_t CONCAT_2(timer_id,_data) = { {0} };           \
    static const drv_rtc_timer_id_t timer_id = &CONCAT_2(timer_id,_data)

		
/**@brief Application time-out handler type. */
typedef void (*drv_rtc_timer_timeout_handler_t)(void * p_context);

/**@brief Structure passed to app_scheduler. */
typedef struct
{
    drv_rtc_timer_timeout_handler_t timeout_handler;
    void *                      p_context;
} drv_rtc_timer_event_t;

/**@brief Timer modes. */
typedef enum
{
    DRV_RTC_TIMER_MODE_SINGLE_SHOT,                 /**< The timer will expire only once. */
    DRV_RTC_TIMER_MODE_REPEATED                     /**< The timer will restart each time it expires. */
} drv_rtc_timer_mode_t;


/**@brief Function for initializing the timer module.
 *
 * @retval     NRF_SUCCESS               If the module was initialized successfully.
 */
ret_code_t drv_rtc_timer_init(void);

/**@brief Function for creating a timer instance.
 *
 * @param[in]  p_timer_id        Pointer to timer identifier.
 * @param[in]  mode              Timer mode.
 * @param[in]  timeout_handler   Function to be executed when the timer expires.
 *
 * @retval     NRF_SUCCESS               If the timer was successfully created.
 * @retval     NRF_ERROR_INVALID_PARAM   If a parameter was invalid.
 * @retval     NRF_ERROR_INVALID_STATE   If the application timer module has not been initialized or
 *                                       the timer is running.
 *
 * @note This function does the timer allocation in the caller's context. It is also not protected
 *       by a critical region. Therefore care must be taken not to call it from several interrupt
 *       levels simultaneously.
 * @note The function can be called again on the timer instance and will re-initialize the instance if
 *       the timer is not running.
 * @attention The FreeRTOS and RTX app_timer implementation does not allow app_timer_create to
 *       be called on the previously initialized instance.
 */
ret_code_t drv_rtc_timer_create(drv_rtc_timer_id_t const *      p_timer_id,
                            drv_rtc_timer_mode_t            mode,
                            drv_rtc_timer_timeout_handler_t timeout_handler);


/**@brief Function for starting a timer.
 *
 * @param[in]       timer_id      Timer identifier.
 * @param[in]       timeout_ticks Number of ticks (of RTC1, including prescaling) to time-out event
 *                                (minimum 5 ticks).
 * @param[in]       p_context     General purpose pointer. Will be passed to the time-out handler when
 *                                the timer expires.
 *
 * @retval     NRF_SUCCESS               If the timer was successfully started.
 * @retval     NRF_ERROR_INVALID_PARAM   If a parameter was invalid.
 * @retval     NRF_ERROR_INVALID_STATE   If the application timer module has not been initialized or the timer
 *                                       has not been created.
 * @retval     NRF_ERROR_NO_MEM          If the timer operations queue was full.
 *
 * @note The minimum timeout_ticks value is 5.
 * @note For multiple active timers, time-outs occurring in close proximity to each other (in the
 *       range of 1 to 3 ticks) will have a positive jitter of maximum 3 ticks.
 * @note When calling this method on a timer that is already running, the second start operation
 *       is ignored.
 */
ret_code_t drv_rtc_timer_start(drv_rtc_timer_id_t timer_id, uint32_t timeout_ticks, void * p_context);





/**@brief Function for stopping the specified timer.
 *
 * @param[in]  timer_id                  Timer identifier.
 *
 * @retval     NRF_SUCCESS               If the timer was successfully stopped.
 * @retval     NRF_ERROR_INVALID_PARAM   If a parameter was invalid.
 * @retval     NRF_ERROR_INVALID_STATE   If the application timer module has not been initialized or the timer
 *                                       has not been created.
 * @retval     NRF_ERROR_NO_MEM          If the timer operations queue was full.
 */
ret_code_t drv_rtc_timer_stop(drv_rtc_timer_id_t timer_id);

/**@brief Function for stopping all running timers.
 *
 * @retval     NRF_SUCCESS               If all timers were successfully stopped.
 * @retval     NRF_ERROR_INVALID_STATE   If the application timer module has not been initialized.
 * @retval     NRF_ERROR_NO_MEM          If the timer operations queue was full.
 */
ret_code_t drv_rtc_timer_stop_all(void);

/**@brief Function for returning the current value of the RTC1 counter.
 *
 * @return    Current value of the RTC1 counter.
 */
uint32_t drv_rtc_timer_cnt_get(void);

/**@brief Function for computing the difference between two RTC1 counter values.
 *
 * @param[in]  ticks_to       Value returned by app_timer_cnt_get().
 * @param[in]  ticks_from     Value returned by app_timer_cnt_get().
 *
 * @return    Number of ticks from ticks_from to ticks_to.
 */
uint32_t drv_rtc_timer_cnt_diff_compute(uint32_t   ticks_to,
                                    uint32_t   ticks_from);



/**@brief Function for getting the maximum observed operation queue utilization.
 *
 * Function for tuning the module and determining OP_QUEUE_SIZE value and thus module RAM usage.
 *
 * @note APP_TIMER_WITH_PROFILER must be enabled to use this functionality.
 *
 * @return Maximum number of events in queue observed so far.
 */
uint8_t drv_rtc_timer_op_queue_utilization_get(void);

/**
 * @brief Function for pausing RTC activity which drives app_timer.
 *
 * @note This function can be used for debugging purposes to ensure
 *       that application is halted when entering a breakpoint.
 */
void drv_rtc_timer_pause(void);

/**
 * @brief Function for resuming RTC activity which drives app_timer.
 *
 * @note This function can be used for debugging purposes to resume
 *       application activity.
 */
void drv_rtc_timer_resume(void);












#endif //__DRV_RTC_TIMER_H__

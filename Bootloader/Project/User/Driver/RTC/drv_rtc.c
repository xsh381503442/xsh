#include "drv_rtc.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "com_apollo2.h"
#include "SEGGER_RTT.h"

#include "am_mcu_apollo.h"
#include "am_util.h"
#include "am_util_string.h"

#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[DRV_RTC]:"

#if MOUDLE_LOG_ENABLED == 1 && RTT_LOG_ENABLED == 1
#define LOG_PRINTF(...)                      SEGGER_RTT_printf(0, ##__VA_ARGS__) 
#define LOG_HEXDUMP(p_data, len)             rtt_log_hexdump(p_data, len) 
#else
#define LOG_PRINTF(...) 
#define LOG_HEXDUMP(p_data, len)             
#endif

#if MOUDLE_DEBUG_ENABLED == 1 && RTT_LOG_ENABLED == 1
#define DEBUG_PRINTF(...)                    SEGGER_RTT_printf(0, ##__VA_ARGS__)
#define DEBUG_HEXDUMP(p_data, len)           rtt_log_hexdump(p_data, len)
#else
#define DEBUG_PRINTF(...)   
#define DEBUG_HEXDUMP(p_data, len) 
#endif
am_hal_rtc_time_t g_rtc_time;

char *pcWeekday[] =
{
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "Invalid day"
};

char *pcMonth[] =
{
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December",
    "Invalid month"
};

static int mthToIndex(char *pcMon)
{
    int idx;
    for (idx = 0; idx < 12; idx++)
    {
        if ( am_util_string_strnicmp(pcMonth[idx], pcMon, 3) == 0 )
        {
            return idx;
        }
    }
    return 12;
}
static int toVal(char *pcAsciiStr)
{
	int iRetVal = 0;
	iRetVal += pcAsciiStr[1] - '0';
	iRetVal += pcAsciiStr[0] == ' ' ? 0 : (pcAsciiStr[0] - '0') * 10;
	return iRetVal;
}




void drv_rtc_time_debug(void)
{
	am_hal_rtc_time_get(&g_rtc_time);

	DEBUG_PRINTF(MOUDLE_NAME"It is now ");
	DEBUG_PRINTF("%d : ", g_rtc_time.ui32Hour);
	DEBUG_PRINTF("%02d : ", g_rtc_time.ui32Minute);
	DEBUG_PRINTF("%02d.", g_rtc_time.ui32Second);
	DEBUG_PRINTF("%02d ", g_rtc_time.ui32Hundredths);
	DEBUG_PRINTF(pcWeekday[g_rtc_time.ui32Weekday]);
	DEBUG_PRINTF(" ");
	DEBUG_PRINTF(pcMonth[g_rtc_time.ui32Month]);
	DEBUG_PRINTF(" ");
	DEBUG_PRINTF("%d, ", g_rtc_time.ui32DayOfMonth);
	DEBUG_PRINTF("20%02d\r\n", g_rtc_time.ui32Year);
}




void drv_rtc_init(bool reset)
{
	DEBUG_PRINTF(MOUDLE_NAME"drv_rtc_init\n");

	am_hal_rtc_time_get(&g_rtc_time);
	
	if(reset)
	{
	
		g_rtc_time.ui32Hour = toVal(&__TIME__[0]);
		g_rtc_time.ui32Minute = toVal(&__TIME__[3]);
		g_rtc_time.ui32Second = toVal(&__TIME__[6]);
		g_rtc_time.ui32Hundredths = 00;
		g_rtc_time.ui32Weekday = am_util_time_computeDayofWeek(2000 + toVal(&__DATE__[9]), mthToIndex(&__DATE__[0]) + 1, toVal(&__DATE__[4]) );
		g_rtc_time.ui32DayOfMonth = toVal(&__DATE__[4]);
		g_rtc_time.ui32Month = mthToIndex(&__DATE__[0]);
		g_rtc_time.ui32Year = toVal(&__DATE__[9]);
		g_rtc_time.ui32Century = 20;
		g_rtc_time.ui32Hundredths = 0;
		DEBUG_PRINTF(MOUDLE_NAME"This application was built on %s at %s.\n", __DATE__, __TIME__);
		am_hal_rtc_time_set(&g_rtc_time);	
	}

	
	drv_rtc_time_debug();
	

}









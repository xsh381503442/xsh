
#ifndef __DEFINE_HEADER
#define __DEFINE_HEADER



#define _APPLICATIONBOARD_V11_
//#define _DEVELOPMENTBOARD_V22_
#define INCLUDE_BMA250API

#ifndef NULL
 #ifdef __cplusplus              // EC++
   #define NULL   0
 #else
   #define NULL   ((void *) 0)
 #endif
#endif

//typedef char						S8;
typedef unsigned char				U8;
typedef short						S16;
typedef unsigned short				U16;
typedef int							S32;
typedef unsigned int				U32;
typedef long long					S64;
typedef unsigned long long			U64;
typedef unsigned char				BIT;
typedef unsigned int				BOOL;
typedef double						F32;

#define CCLK		60000000		/**< CPU-Clock */
#define PCLK		60000000		/**< Peripheral Clock */

//#define ON			1						/**< Define for "ON" */
//#define OFF			0						/**< Define for "OFF" */
#define TRUE		1						/**< Define for "TRUE" */
#define FALSE		0						/**< Define for "FALSE" */
#define ENABLE	    1						/**< Define for "ENABLE" */
#define DISABLE	    0						/**< Define for "DISABLE" */
#define LOW			0						/**< Define for "Low" */
#define HIGH		1						/**< Define for "High" */
#define INPUT		0						/**< Define for "Input" */
#define OUTPUT	    1						/**< Define for "Output" */

#define UART0		0						/**< Define for "UART0" */
#define UART1		1					  /**< Define for "UART1" */
#define USB			2						/**< Define for "USB" */
#define SPI0		3						/**< Define for "SPI0" */
#define SPI1		4						/**< Define for "SPI1" */
#define I2C0		5						/**< Define for "I2C0" */
#define I2C1		6						/**< Define for "I2C1" */
#define C_Null_U8X  				0
#define C_Zero_U8X  			(U8)0
#define C_One_U8X               (U8)1
#define C_Two_U8X               (U8)2
#define C_Three_U8X             (U8)3
#define C_Four_U8X              (U8)4
#define C_Five_U8X              (U8)5
#define C_Six_U8X               (U8)6
#define C_Seven_U8X             (U8)7
#define C_Eight_U8X             (U8)8
#define C_Nine_U8X              (U8)9
#define C_Ten_U8X               (U8)10
#define C_Eleven_U8X            (U8)11
#define C_Twelve_U8X            (U8)12
#define C_Sixteen_U8X           (U8)16
#define C_TwentyFour_U8X        (U8)24
#define C_ThirtyTwo_U8X         (U8)32
#define C_Hundred_U8X           (U8)100
#define C_OneTwentySeven_U8X    (U8)127
#define C_TwoFiftyFive_U8X      (U8)255
#define C_TwoFiftySix_U16X      (U16)256
/* Return type is True */
#define C_Successful_S8X			(S8)0
/* return type is False */
#define C_Unsuccessful_S8X			(S8)-1
#if 0
enum FUNCTION_RETURN_VALUES
{
    OK = 0,											/**< no Error occured */
    ERROR=-1,										/**< general error occured */
	PARAMETER_OUT_OF_RANGE=-2,	/**< one or more parameter out of range */
	MISSING_PARAMETER=-3,				/**< one or more parameter is missing */
	NO_DATA_AVAILABLE=-4,				/**< no data available */
	UNKNOWN_INSTRUCTION=-5,			/**< unknown instruction */
	UNKNOWN_SENSOR=-6,					/**< unknown sensor */
	WRONG_SENSOR=-7							/**< wrong sensor */
};
#endif

typedef enum
{
	E_False,
	E_True
} te_Boolean;




#endif /*  __DEFINE_HEADER */


#define		BSP_GPIO_UWB_WAKEUP		47
#define		BSP_GPIO_UWB_SOS		44
#define		BSP_GPIO_UWB_BUSY		24

#define		UWB_BUFFER_SIZE     60

extern  uint8_t g_UWB_Tx_Buffer[UWB_BUFFER_SIZE];
extern uint8_t g_UWB_Rx_Buffer[UWB_BUFFER_SIZE];
void drv_uwb_init(void);
void drv_uwb_uninit(void);
void SetPeriod( uint8_t period );
void StartLocating( void );
void StartBoot( void );
void SendData( uint16_t CO_Data, uint8_t  heartrate );







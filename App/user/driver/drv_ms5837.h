#ifndef DRV_MS5837_H
#define	DRV_MS5837_H

#define	DRV_MS5837_I2C_ADDR			0x76

#define	DRV_MS5837_COMMAND_RESET	0x1E
#define	DRV_MS5837_COMMAND_D1_256	0x40
#define	DRV_MS5837_COMMAND_D1_512	0x42
#define	DRV_MS5837_COMMAND_D1_1024	0x44
#define	DRV_MS5837_COMMAND_D1_2048	0x46
#define	DRV_MS5837_COMMAND_D1_4096	0x48
#define	DRV_MS5837_COMMAND_D1_8192	0x4A
#define	DRV_MS5837_COMMAND_D2_256	0x50
#define	DRV_MS5837_COMMAND_D2_512	0x52
#define	DRV_MS5837_COMMAND_D2_1024	0x54
#define	DRV_MS5837_COMMAND_D2_2048	0x56
#define	DRV_MS5837_COMMAND_D2_4096	0x58
#define	DRV_MS5837_COMMAND_D2_8192	0x5A
#define	DRV_MS5837_COMMAND_ADC		0x00
#define	DRV_MS5837_COMMAND_PROM_C0	0xA0
#define	DRV_MS5837_COMMAND_PROM_C1	0xA2
#define	DRV_MS5837_COMMAND_PROM_C2	0xA4
#define	DRV_MS5837_COMMAND_PROM_C3	0xA6
#define	DRV_MS5837_COMMAND_PROM_C4	0xA8
#define	DRV_MS5837_COMMAND_PROM_C5	0xAA
#define	DRV_MS5837_COMMAND_PROM_C6	0xAC

static void drv_ms5837_enable(void);
static void drv_ms5837_disable(void);
static void drv_ms5837_write(uint8_t ui8Command);
static uint32_t drv_ms5837_read(uint8_t ui8Command,uint8_t ui8Number);
static void drv_ms5837_CalcPT(float *pPres,float *pTemp,uint32_t d1,uint32_t d2);
static void drv_ms5837_CalcPT2nd(float *pPres,float *pTemp,uint32_t d1,uint32_t d2);

extern void drv_ms5837_data_get(float *pPres,float *pAlt,float *pTemp);
extern float drv_ms5837_rel_altitude(float Press, float Ref_P);
extern int32_t drv_ms5837_abs_altitude(uint32_t Press);
extern float drv_ms5837_Pressure2altitude(float p);

extern void ambient_value_get(float *pPres,float *pAlt,float *pTemp);
extern float algo_altitude_to_press(float altit, float Ref_P);

#endif

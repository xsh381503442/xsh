#ifndef _ALGO_IMU_CALIBRATION_H_
#if defined WATCH_IMU_CALIBRATION

#define _ALGO_IMU_CALIBRATION_H_

#include <stdint.h>

#define ACC_CAL_SUM 10 //校准点个数
#define ACC_SAMPLE_SUM 7 //静止判断采样点最小值
#define GRYO_CAL_SUM 5
#define CAL_QUIT 0 
#define CAL_ING 1 
#define MATRIX_ORDER 9
//#define MATRIX_ORDER 7 //磁力计测试

typedef struct
{
    float acc_x;
    float acc_y;
    float acc_z;
    uint8_t captured; 
}imu_acc_data;

typedef struct
{
    float x_acc;
    float y_acc;
    float z_acc;
    float x_gyro;
    float y_gyro;
    float z_gyro;
}imu_6d_data;

typedef struct
{
    uint8_t cal_flag;
    uint8_t staticFlg;
    uint32_t static_count;
}data_flag;

typedef struct
{
    float factor_and_vertical[3][3];
    float acc_zero_offset[3];
	//float gryo_zero[3];
	uint8_t flag ;
}acc_matrix_parameter;

typedef struct
{
    
	float gryo_zero[3];
	uint8_t flag ;
}gyro_matrix_parameter;


extern void acc_calibration_entrance(void);
extern void gyro_calibration_entrance(void);
extern void get_static_data(void);
extern void get_static_gryo_data(void);
extern float variance_data(float *data,uint16_t len);
extern void bubble_sort(float *ac, int n);
extern void calculate_acc_parameter(void);
extern void acc_matrix_operation(void);
extern void acc_matrix_inversion(float matrix_acc[9][9]);
extern float get_det(float acc_BBT[9][9], uint8_t n);
extern void get_matrix_surplus(float acc_BBT[9][9], int n, float ans[9][9]);
extern void acc_matrix_multiply(float acc_matrix_inver[9][9],float acc_matrix_B[9][ACC_CAL_SUM]);
extern void get_acc_parameter(void);
extern void pass_back_parameter(void);
extern int8_t find_position(float *acc_data);
extern uint8_t find_min(float *data,uint8_t length);
extern void acc_parameter_reset(uint8_t flag);
extern void gyro_parameter_reset(uint8_t flag);
extern void parameter_reset(uint8_t flag);
extern void save_acc_calibration_paremeter(uint8_t flag);
extern void read_acc_calibration_paremeter(void);
extern void save_gyro_calibration_paremeter(uint8_t flag);
extern void read_gyro_calibration_paremeter(void);
static void pc_uart_command_hardware_test_acc_data(uint8_t *buf, uint8_t len);	
extern void pc_uart_cal_back(uint8_t flag);
//extern void pc_uart_gyro_cal_back(uint8_t flag);
extern int getNext(int i, int m, int n);
extern int getPre(int i, int m, int n);
extern void movedata(double *mtx, int i, int m, int n);
extern void transpose(double *mtx, int m, int n);
extern void LUP_solve_inverse(double A[MATRIX_ORDER*MATRIX_ORDER]);
extern void LUP_Descomposition(double A[MATRIX_ORDER*MATRIX_ORDER], double L[MATRIX_ORDER*MATRIX_ORDER], double U[MATRIX_ORDER*MATRIX_ORDER], int P[MATRIX_ORDER]);
extern  void LUP_Solve(double L[MATRIX_ORDER*MATRIX_ORDER], double U[MATRIX_ORDER*MATRIX_ORDER], int P[MATRIX_ORDER], double b[MATRIX_ORDER],double*x);

extern void get_acc_use_parameter(float raw_acc[3]);//使用加速度校准参数
extern void get_gyro_use_parameter(float raw_gyro[3]);//使用加速度校准参数
#endif


#endif

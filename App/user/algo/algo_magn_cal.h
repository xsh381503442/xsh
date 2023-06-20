#ifndef _ALGO_MAGN_CAL_
#define _ALGO_MAGN_CAL_
#include "drv_lis3mdl.h"
#include <float.h>
#include "Watch_config.h"

#define  MAGN_CAL_POINT_MIN 49
#define  MAGN_CAL_PARA_MIN  63
#define  MAGN_CONVER_COFF  1.0/2281.0 //里程为12gauss时，磁力计的灵敏度为2281LSB/gauss
#define  MAGN_CAL_STATE_NUM 3


	



//#define  MAGN_CAL_PARA_DIS 2.5f 
//#define  MAGN_CAL_PARA_AREA 4.5f
//#define  MAGN_CAL_PARA_VOL 6.0f

//#define  MAGN_CAL_PARA_INTERVAL_AREA 7.5f

#define  MAGN_CAL_PARA_DIS 2.5f 
#define  MAGN_CAL_PARA_AREA 4.0f
#define  MAGN_CAL_PARA_VOL 5.0f
#define  MAGN_CAL_PARA_INTERVAL_AREA 6.2f


#define  MAGN_CAL_POINT_LACK  0
#define  MAGN_CAL_DIS_SMALL 0
#define  MAGN_CAL_ANGLE_SMALL 0


#define  MAGN_CAL_DIS_SHORT   0
#define  MAGN_CAL_RADIUS_SAMLL 0
#define  MAGN_CAL_DATA_CON 0


#define  MAGN_CAL_POINT_ENOUGH 1
#define  MAGN_CAL_PARA_ENOUGH  1
#define  MAGN_CAL_DIS_ENOUGH   1
#define  MAGN_CAL_DATA_FINE 1


 #define  MAGN_MATRIX_ORDER 7









#define	 ACC_STD_SUM	7


#ifdef MAGN_CAL_DOUBLE

#define MAGN_CAL_SMAX		DBL_MAX
#define MAGN_CAL_SMIN		DBL_MIN

#else

#define MAGN_CAL_SMAX		FLT_MAX
#define MAGN_CAL_SMIN		FLT_MIN

#endif



typedef struct
{
float x;
float y;
float z;


}magn_cal_data;


typedef struct
{
    float acc_x[ACC_STD_SUM];
    float acc_y[ACC_STD_SUM];
    float acc_z[ACC_STD_SUM];
	float acc_c[ACC_STD_SUM];
	uint8_t static_flag,static_count;
	
    
}acc_data;
typedef struct
{
 double hi_bias[3];     
 double sf_matrix[3][3];       
 uint8_t flag ;
}magn_matrix_parameter;

typedef struct
{
	int row,column;
	
	double *data;//用来存放矩阵的元素
}matrix_data;

extern void magn_dev_para_init(void);
extern void magn_para_init(void);
extern void magn_para_reset(void);
extern float magn_angle_and_cal(uint8_t magn_cal_flag);
extern void save_magn_calibration_paremeter(void);
extern void read_magn_calibration_paremeter(void);
extern uint8_t magn_cal_auto(float*magn_data,magn_matrix_parameter*para);
extern uint8_t magn_cal_flag_get(void);
extern uint8_t judge_value(magn_cal_data *data,uint8_t num);

static void lis3mdl_reg_write(uint8_t reg,uint8_t value);
extern void drv_lis3mdl_bubble_get(float pitch, float roll);
extern uint8_t get_four_point(magn_cal_data*point_data,uint8_t num,uint8_t flag,magn_cal_data four_data[]);
extern uint8_t fit_ellipsoid(magn_cal_data*cal_ellipsold,uint8_t cal_num);

extern int get_next(int i, int m, int n);
extern int get_pre(int i, int m, int n);
extern void move_data(double *mtx, int i, int m, int n);
extern void transpose_m(double *mtx, int m, int n);
extern void magn_solve_inverse(double *A,double *mtx,uint8_t order_num);
extern void magn_descomposition(double *A, double *L, double *U, int *P,uint8_t order);
extern void magn_solve(double *L, double *U, int *P, double *b,double*x,uint8_t order);
extern void evalue_judge(double*para_poly,double*para_x);
extern void evector_judge(matrix_data *A_raw, matrix_data *eigenVector, matrix_data *eigenValue,uint8_t flag);



#endif

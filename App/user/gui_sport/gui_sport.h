#ifndef _GUI_SPORT_H__
#define _GUI_SPORT_H__

#include <stdint.h>
#include "drv_lcd.h"
#include "task_display.h"
#include "task_key.h"

extern volatile ScreenState_t Pause_Save_Index;		//��ͣ���汣�棬���ڷ��ز�ͬ��������ͣ����ʱ������
extern volatile ScreenState_t Save_Detail_Index;	//������汣�棬���ڷ��ز�ͬ�������������ʱ������
#if defined WATCH_SIM_NUMBER
	#define SPORT_DETAIL_DATA_UNITS_Y_AXIS		172
#else
	#define SPORT_DETAIL_DATA_UNITS_Y_AXIS		160
#endif

//#if defined WATCH_SIM_SPORT

#define SPORT_DATA_DISPLAY_SUM 4 			//������ʾ����
#define SPORT_DATA_DISPLAY_SET_MAX 32 		//������ʾ������������ 			

#define SPORT_REMIND_SET_MAX 32 		//�˶����������������� 


#define SPORT_DATA_PREVIEW 0
#define SPORT_DATA_SHOW    1
#define VICE_SCREEN_DATA_SHOW    2

#define GUI_SPORT_READY_INDEX_MAX 3

#define GUI_SPORT_READY_INDEX_DISPLAY 2
#define GUI_SPORT_READY_INDEX_START 0
#define GUI_SPORT_READY_INDEX_SET 1


#define GUI_VICE_SPORT_DETAIL_1 0
#define GUI_VICE_SPORT_DETAIL_2 1


/*������ʾѡ��*/
#define SPORT_DATA_DISPALY_TOTAL_DISTANCE			0	//����
#define SPORT_DATA_DISPALY_TOTAL_TIME 				1	//ʱ��
#define SPORT_DATA_DISPALY_AVE_PACE					2	//ƽ������
#define SPORT_DATA_DISPALY_REALTIME_PACE			3	//ʵʱ����
#define SPORT_DATA_DISPALY_TOTAL_STEP		        4	//�ܲ���
#define SPORT_DATA_DISPALY_TOTAL_CLIMB		        5	//�ۼ�����
#define SPORT_DATA_DISPALY_REALTIME_HEART			6	//����
#define SPORT_DATA_DISPALY_AVE_SPEED				7	//ƽ���ٶ�
#define SPORT_DATA_DISPALY_REALTIME_SPEED			8	//ʵʱ�ٶ�
#define SPORT_DATA_DISPALY_ALTITUDE			        9	//���θ߶�
#define SPORT_DATA_DISPALY_SWIM_LAPS				10	//��Ӿ����
#define SPORT_DATA_DISPALY_PREVIOUS_SWOLF		    11	//��һ��swolf
#define SPORT_DATA_DISPALY_AVE_SWIM_FREQUENCY		12	//��ˮ����(��ƽ����Ƶ)
#define SPORT_DATA_DISPALY_AVE_SWIM_PACE			13	//��Ӿ��ƽ������
#define SPORT_DATA_DISPALY_VER_AVE_SPEED			14	//��ɽ�Ĵ�ֱ�ٶ�


#define SPORT_DATA_DISPALY_OPTIMUM_PACE				15	//�������
#define SPORT_DATA_DISPALY_AVE_HEART				16	//ƽ������
#define SPORT_DATA_DISPALY_REALTIME_FREQUENCY		17	//ʵʱ��Ƶ
#define SPORT_DATA_DISPALY_AVE_FREQUENCY			18	//ƽ����Ƶ
#define SPORT_DATA_DISPALY_CALORIE					19	//��·��ֵ
#define SPORT_DATA_DISPALY_LAST_PACE				20	//��һȦ����
#define SPORT_DATA_DISPALY_LAST_TIME				21	//��һȦʱ��
#define SPORT_DATA_DISPALY_OPTIMUM_SPEED			22	//����ٶ�
#define SPORT_DATA_DISPALY_REALTIME_CADENCE			23	//ʵʱ̤Ƶ
#define SPORT_DATA_DISPALY_AVE_CADENCE				24	//ƽ��̤Ƶ
#define SPORT_DATA_DISPALY_TOTAL_STROKES			25	//�ܻ���
#define SPORT_DATA_DISPALY_AVE_STROKES				26	//ƽ������
#define SPORT_DATA_DISPALY_AVE_SWOLF				27	//ƽ��swolf
//#define SPORT_DATA_DISPALY_AVE_SWIM_FREQUENCY		28	//ƽ����Ƶ


#define SPORT_REMIND_SET_HAERT						0	//���ʸ澯
#define SPORT_REMIND_SET_HRRECOVERY					1	//���ʻָ���
#define SPORT_REMIND_SET_PACE						2	//��������
#define SPORT_REMIND_SET_DISTANCE					3	//��������
#define SPORT_REMIND_SET_AUTOCIRCLE					4	//�Զ���Ȧ
#define SPORT_REMIND_SET_SPEED						5	//�ٶ�����
#define SPORT_REMIND_SET_ALTITUDE					6	//�߶�����(������ϸ�ڴ���)
#define SPORT_REMIND_SET_SWIMPOOL					7	//Ӿ�س���
#define SPORT_REMIND_SET_TIME						8	//ʱ������
#define SPORT_REMIND_SET_GOALCIRCLE					9	//��Ȧ����

/*GODδ��*/
#define SPORT_REMIND_SET_GOALKCAL					10	//ȼ֬Ŀ��
#define SPORT_REMIND_SET_INDOORRUNLENG				11	//�����ܲ���


typedef struct {
	bool enable;
	int8_t tens;
	int8_t ones;
} _remind_heartrate;

typedef struct {
	bool enable;
	int8_t min;
	int8_t sec;
}_remind_pace;

typedef struct {
	bool enable;
	int8_t km;
	int8_t hm;
}_remind_distance;

typedef struct {
	bool enable;
	int8_t hundreds;
	int8_t tens;
	int8_t ones;
}_remind_goalkcal;

typedef struct {
	bool enable;
	int8_t thousand;
	int8_t hundreds;
	int8_t tens;
	int8_t ones;
}_remind_altitude;






/*
�˶����ý���
*/
extern void gui_sport_pause_time(void);
extern void gui_sport_pause_acttime(void);
extern void gui_sport_cancel_options(uint8_t index);
extern void gui_sport_detail_track_paint(uint8_t backgroundcolor);

extern void gui_sport_data_init(void);



extern void gui_sport_heart_1_paint(uint8_t heart,uint8_t type);
extern void gui_sport_realtime_paint(uint8_t type);

extern void gui_sport_pause_hint_paint(void);


//�˶�׼������
extern void gui_sport_ready_paint(void);
//������ʾ�����
extern void gui_sport_data_display_ave_pace_paint(uint8_t state,uint16_t pace);
extern void gui_sport_data_display_optimum_pace_paint(uint8_t state,uint16_t pace);
extern void gui_sport_data_display_last_pace_paint(uint8_t state,uint16_t pace);
extern void gui_sport_data_display_realtime_heart_paint(uint8_t state,uint8_t heart);
extern void gui_sport_data_display_ave_heart_paint(uint8_t state,uint8_t heart);
extern void gui_sport_data_display_realtime_frequency_paint(uint8_t state,uint16_t	frequency);
extern void gui_sport_data_display_ave_frequency_paint(uint8_t state,uint16_t frequency);
extern void gui_sport_data_display_calorie_paint(uint8_t state,uint32_t calorie);
extern void gui_sport_data_display_last_time_paint(uint8_t state,uint32_t time);
extern void gui_sport_data_display_ave_speed_paint(uint8_t state,uint16_t speed);
extern void gui_sport_data_display_ave_swim_pace_paint(uint8_t state,uint16_t pace);
extern void gui_sport_data_display_swim_laps_paint(uint8_t state,uint16_t laps);
extern void gui_sport_data_display_total_strokes_paint(uint8_t state,uint32_t strokes);
extern void gui_sport_data_display_ave_strokes_paint(uint8_t state,uint32_t strokes);
extern void gui_sport_data_display_ave_swlof_paint(uint8_t state,uint16_t swlof);
extern void gui_sport_data_display_ave_swim_frequency_paint(uint8_t state,uint16_t frequency);
extern void gui_sport_data_display_realtime_speed_paint(uint8_t state,uint16_t speed);

extern void gui_sport_data_display_optimum_speed_paint(uint8_t state,uint16_t speed);
extern void gui_sport_data_display_realtime_cadence_paint(uint8_t state,uint32_t cadence);
extern void gui_sport_data_display_ave_cadence_paint(uint8_t state,uint32_t cadence);

extern void gui_sport_data_bar_display_paint(void);//������ʾ����ֽ���

extern void gui_sport_data_display_set_paint(void);
extern void gui_sport_remind_set_paint(void);

extern void gui_sport_remind_set_heart_paint(void);
extern void gui_sport_remind_set_pace_paint(void);
extern void gui_sport_remind_set_distance_paint(void);
extern void gui_sport_remind_set_autocircle_paint(void);
extern void gui_sport_remind_set_goalkcal_paint(void);
extern void gui_sport_remind_set_time_paint(void);
extern void gui_sport_remind_set_speed_paint(void);
extern void gui_sport_remind_set_swimpool_paint(void);
extern void gui_sport_remind_set_goalcircle_paint(void);
extern void gui_sport_remind_set_indoorleng_paint(void);
extern void gui_sport_remind_set_altitude_paint(void);

extern void gui_sport_pause_hint_btn_evt(uint32_t Key_Value);
extern void gui_sport_ready_btn_evt(uint32_t Key_Value);
extern void gui_sport_data_display_btn_evt(uint32_t Key_Value);



extern void gui_sport_data_display_preview_btn_evt(uint32_t Key_Value);


extern void gui_sport_data_display_set_btn_evt(uint32_t Key_Value);
extern void gui_sport_remind_set_btn_evt(uint32_t Key_Value);

extern void gui_sport_remind_set_heart_btn_evt(uint32_t Key_Value);
extern void gui_sport_remind_set_pace_btn_evt(uint32_t Key_Value);
extern void gui_sport_remind_set_distance_btn_evt(uint32_t Key_Value);
extern void gui_sport_remind_set_autocircle_btn_evt(uint32_t Key_Value);
extern void gui_sport_remind_set_goalkcal_btn_evt(uint32_t Key_Value);
extern void gui_sport_remind_set_time_btn_evt(uint32_t Key_Value);
extern void gui_sport_remind_set_speed_btn_evt(uint32_t Key_Value);

extern void gui_sport_remind_set_swimpool_btn_evt(uint32_t Key_Value);
extern void gui_sport_remind_set_goalcircle_btn_evt(uint32_t Key_Value);
extern void gui_sport_remind_set_indoorleng_btn_evt(uint32_t Key_Value);


//#endif

#if defined WATCH_SIM_SPORT || defined WATCH_COM_SPORT

#define SPORT_DATA_DISPALY_INDEX_UP					0	//������ʾ�ϲ�����,��Բ��������������ʾ������
#define SPORT_DATA_DISPALY_INDEX_MID				1	//������ʾ�в�����,��gps��������ʾ������
#define SPORT_DATA_DISPALY_INDEX_DOWN				2	//������ʾ�²����ã�������ʾ������
#define SPORT_DATA_DISPALY_INDEX_UP_CIRCLE			3	//������ʾ�ϲ�����Բ���������˶�����ʾ��
#define SPORT_DATA_DISPALY_INDEX_MID_GPS			4	//������ʾ�в�,��gps���˶�����ʾ��
#define SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT	    5	//������ʾ�²����ã�����ʾ���˶�����ʾ��
#define SPORT_DATA_DISPALY_INDEX_NEW_LACTATE	    6	//��UI��������ʾ4������

extern void gui_sport_data_display_total_time_paint(uint8_t state,uint32_t time);
extern void gui_sport_data_display_total_distance_paint(uint8_t state,uint32_t distance);
extern void gui_sport_data_display_realtime_pace_paint(uint8_t state,uint16_t pace);
extern void gui_sport_data_display_realtime_speed_paint(uint8_t state,uint16_t speed);

#endif 

#if defined WATCH_COM_SPORT
/*extern void gui_sport_heart_1_paint(uint8_t heart);
extern void gui_sport_realtime_1_paint(uint8_t type);
extern void gui_sport_pause_hint_paint(void);
extern void gui_sport_pause_time(void);
extern void gui_sport_pause_acttime(void);
extern void gui_sport_cancel_options(uint8_t index);
extern void gui_sport_detail_track_paint(uint8_t backgroundcolor);
extern void gui_sport_pause_hint_btn_evt(uint32_t Key_Value);*/

#endif 
typedef struct {
	uint32_t remaining_distance;    //ѭ������ʣ�����
	uint32_t sport_distance;        //�˶��ܾ���
	uint32_t nearest_distance;      //ƫ��켣����
	uint8_t  whether_in_track;      //�Ƿ��ڹ켣�ϱ��
	uint8_t  sport_traceback_flag;  //ѭ�����������л���ǣ�����ʱ����ʣ����뽻��10����ʾ��1��ʾʱ�䣬0��ʾʣ�����
	uint8_t	 is_traceback_flag;		//�Ƿ���ѭ��������ǣ�1��ʾ��ѭ������
}TraceBackStr;

extern TraceBackStr TRACE_BACK_DATA;


enum
{
	SPORT_STATUS_READY = 0U,	//׼��״̬
	SPORT_STATUS_START,			//��ʼ״̬
	SPORT_STATUS_PAUSE,			//��ͣ״̬
};

enum
{
	SPORT_PAGE_UP = 0U,			//�Ϸ���ʾ
	SPORT_PAGE_DOWN,			//�·���ʾ
};

enum
{
	SPORT_GPS_LEFT1 = 0U,		//GPSͼ�꿿��1
	SPORT_GPS_LEFT2,			//GPSͼ�꿿��2
	SPORT_GPS_LEFT3,			//GPSͼ�꿿��3
	SPORT_GPS_UP,				//GPSͼ�꿿��
	SPORT_GPS_RIGHT_UP_4,
	SPORT_GPS_RIGHT_UP,
	
	SPORT_GPS_RIGHT_UP_6,

	
	SPORT_GPS_RIGHT_UP_COD
};

//enum
//{
//	SPORT_FEEL_EASY = 0U,		//����
//	SPORT_FEEL_ABITIRED,		//�е����
//	SPOTR_FEEL_TIRED,	        //����
//};


enum
{   
    FeelEase = 0U,           //����,������
    FeelExtremelyEasy = 1U, // ��������
    FeelVeryEasy = 2U,      // ������
	FeelEasy = 3U,	        //����
	FeelAbitTired = 4U,		//�е����
	FeelTired = 5U,	        //����
	FeelVeryTired = 6U,     //�ǳ�����
    FeelExtremelyTired = 7U,//�������
    FeelExhausted = 8U,     //��ƣ����

};

//���ʻָ�����ز���
extern uint8_t sport_end_flag ;  //�Ƿ��˶�����ѡ���־
extern uint8_t sport_end_hrvalue;   //�˶�����ʱ����ֵ
extern uint8_t sport_min_hrvalue ;   //�˶�һ���Ӻ�����ֵ
extern uint32_t sport_end_time ;    //�˶�����?


extern ScreenState_t Get_Curr_Sport_Index(void);
extern ScreenState_t Set_Sport_Index( ScreenState_t ScreenSta );
extern void Set_Select_Sport_Index(void);
extern ScreenState_t Set_Curr_Sport_Index( ScreenState_t ScreenSta);
extern void CreateSportDisplay(void);

extern void gui_sport_detail_heartratezone_paint(uint8_t backgroundcolor);


extern void gui_sport_select_paint(ScreenState_t state);

extern void gui_sport_btn_evt(uint32_t Key_Value);

extern void gui_sport_index_circle_paint(uint8_t index, uint8_t nums, uint8_t type);

extern void gui_sport_index_circle_vert_paint(uint8_t index, uint8_t nums);

extern void gui_sport_arrows(uint16_t angle,uint16_t y,uint16_t x, uint8_t color);
extern void gui_sport_compass(uint16_t angle);
extern void gui_sport_realtime_battery(uint8_t type);
extern void gui_sport_page_prompt(uint8_t type);

extern void set_gps_flicker(uint8_t flicker);
extern uint8_t get_gps_flicker(void);
extern void gui_sport_gps_status(uint8_t direction);
extern void gui_sport_motor_prompt(void);
extern void SportContinueLaterDisplay(void);

extern void SportContinueLaterFlagSet(void);
extern void SportContinueLaterFlagClr(void);
extern void gui_sport_every_1_distance(uint8_t type, uint32_t distance);
typedef struct {
	uint8_t x_axis;						//Destination X coordinate
	uint8_t y_axis;						//Destination Y coordinate				
} Sport_index_axis;



typedef struct {
	uint16_t start_angle;						//Destination X coordinate
	uint16_t end_angle;						//Destination Y coordinate				
} sport_angle_range;

extern uint8_t g_sport_status;

extern ScreenState_t sports_menu_index_record(void);

extern void gui_sport_cancel_paint(void);
extern void gui_sport_feel_paint(void);
extern void gui_sport_heart_recovery_paint(void);
extern void gui_sport_saving_paint(uint8_t endangle);
extern void gui_sport_saved_paint(void);

extern void gui_swich_msg(void);
extern void gui_sport_index_down(ScreenState_t FirstIndex,ScreenState_t LastIndex);
extern void gui_sport_index_up(ScreenState_t FirstIndex,ScreenState_t LastIndex);

extern void gui_sport_cancel_init(void);
extern void gui_sport_cancel_btn_evt(uint32_t Key_Value);
extern void gui_sport_feel_btn_evt(uint32_t Key_Value);
extern void gui_sport_heart_recovery_btn_evt(uint32_t Key_Value);
extern void gui_sport_feel_init(void);
extern uint8_t SportIndexRecAdjust(void);

extern void gui_run_track_trackback_paint2(void);
extern void gui_sport_remaining_distance_paint(uint32_t distance);
extern void gui_sport_sail_round_time_paint(uint8_t x,uint8_t y,uint8_t forecolor);
extern void gui_sport_traceback_offtrace_paint(uint32_t distance);
extern void gui_sport_traceback_heartrate_paint(uint32_t distance, uint8_t heart);

extern void gui_sport_track_trackback_paint(uint32_t distance); 
extern void gui_sport_traceback_time(void);
extern void gui_sport_track_trackback_paint2(uint32_t distance);

extern void gui_sport_dottrack_nums(uint32_t dot_nums);
extern void gui_sport_dot_prompt(void);
extern void sport_dottrack(void);
extern void gui_sport_dot_draw_prompt(void);
extern void gui_sport_start_init(void);
#if defined WATCH_SPORT_EVENT_SCHEDULE
extern void init_sport_event_data_str(uint8_t type,uint8_t m_status);
extern uint32_t g_tool_or_sport_goto_screen;//0����ǰ��ʼ״̬>0��ʾ���ĸ����������Ƽ������˶�
#endif
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
extern void sport_cloud_start_notify(void);
extern void gui_notify_cloud_close_time_paint(void);
extern void gui_notify_cloud_close_time_btn_evt(uint32_t Key_Value);
extern void set_is_sport_event_cloud_valid(bool status);
extern bool get_is_sport_event_cloud_valid(void);
#endif
  
extern void gui_lactate_distance_paint(uint8_t state,uint32_t distance);
extern void gui_lactate_time_paint(uint8_t state,uint32_t time);
extern void gui_lactate_pace_paint(uint8_t state,uint16_t pace);
extern void gui_lactate_bpm_paint(uint8_t state,uint16_t bpm);

extern void gui_sport_data_display_preview_paint(uint8_t flag);

extern void gui_sport_data_four_bar_display_paint(uint8_t bar_order,uint8_t sport_item,uint8_t flag,uint8_t size_flag);
extern void gui_sport_data_show_str(uint8_t sport_item,char *str);

extern void gui_sport_heart_paint(uint8_t flag);
extern void sport_ready_menu( char* menu1, char* menu2, char* menu3);



extern void gui_sport_remind_set_altitude_init(void);
extern void gui_sport_remind_set_altitude_btn_evt(uint32_t Key_Value);
extern void gui_app_sport_data_show_str(uint8_t type,char *str);
extern void gui_vice_screen_data_paint(uint8_t flag);
extern void gui_vice_screen_hint_paint(void);
extern void gui_vice_pause_hint_paint(void);
extern void gui_sport_pause_and_data_paint(uint8_t type,uint8_t index);
extern void gui_vice_sport_detail_paint(uint8_t sport_type);
extern void gui_vice_sport_run_detail_paint(void);
extern void gui_vice_sport_walk_detail_paint(void);
extern void gui_vice_sport_ride_detail_paint(void);
extern void gui_vice_sport_climb_detail_paint(void);
extern void gui_vice_run_save_detail_1_paint(uint8_t backgroundcolor);
extern void gui_vice_run_save_detail_2_paint(uint8_t backgroundcolor);
extern void gui_vice_sport_walk_detail_1_paint(uint8_t backgroundcolor);
extern void gui_vice_sport_walk_detail_2_paint(uint8_t backgroundcolor);
extern void gui_vice_sport_cycling_detail_1_paint(uint8_t backgroundcolor);
extern void gui_vice_sport_cycling_detail_2_paint(uint8_t backgroundcolor);
extern void gui_vice_climbing_save_detail_1_paint(uint8_t backgroundcolor);
extern void gui_vice_climbing_save_detail_2_paint(uint8_t backgroundcolor);

extern void gui_vice_sport_detail_btn_evt(uint32_t evt);

extern void gui_sport_location_ok_data_paint(uint8_t type,uint8_t index);

#endif




#include "gui_tool_config.h"
#include "gui_tool_gps.h"

#include "drv_lcd.h"
#include "drv_key.h"

#include "task_tool.h"
#include "task_timer.h"

#include "gui.h"
#include "gui_tool.h"

#include "algo_gauss.h"

#include "font_config.h"

#include "com_data.h"
#include "gui_run.h"
#include "drv_battery.h"
#include "gui_sport.h"
#include "drv_extFlash.h"

#include "task_sport.h"
#include "gui_sport_crosscountry_hike.h"
#include "gui_sport_cycling.h"
#include "gui_sport_walk.h"
#include "gui_sport_marathon.h"
#include "gui_crosscountry.h"
#include "gui_climbing.h"
#include "gui_swimming.h"


#if DEBUG_ENABLED == 1 && GUI_TOOL_GPS_LOG_ENABLED == 1
	#define GUI_TOOL_GPS_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_TOOL_GPS_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_TOOL_GPS_WRITESTRING(...)
	#define GUI_TOOL_GPS_PRINTF(...)		        
#endif


extern _drv_bat_event   g_bat_evt;

uint8_t ENTER_NAVIGATION;
uint8_t ENTER_MOTION_TRAIL;
uint8_t LOADINGFLAG;

static ScreenState_t SCREEN_STATE_S;
//���Խӿ��ã���ʼ
#if 1

//com_sport.c�е�ShareTrack[SHARED_TRACK_NUM_MAX]������ע�͵����������
ShareTrackStr ShareTrack[SHARED_TRACK_NUM_MAX] = {
																										0,
																										0
																									/*{.Name = "���������",
																									 .diatance = 1000000,
																									 .Activity_Type = ACT_RUN},
																									{.Name = "ɳ���ܲ�",
																									 .diatance = 800000,
																									 .Activity_Type = ACT_CLIMB}*/
																								 };    //����켣����ͷ�洢���飬���洢����

static int8_t ShareTrackDataValidNum = 0;    //����켣��Ч������Ŀ���˳�����켣������Ҫ����
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
static uint8_t m_share_track_back_discreen = GOTO_SHARE_TRACK_BY_NONE;//0:һ���˶� 1��GPS������淵���Ƽ�����  2��ͽ��ԽҰ�����Ƽ�������� 3��ԽҰ�ܽ����Ƽ����
void set_share_track_back_discreen(uint8_t status)
{
	m_share_track_back_discreen = status;
}
uint8_t get_share_track_back_discreen(void)
{
	return m_share_track_back_discreen;
}
#endif
//��flash��ȡ����켣����ͷ�ṹ�壬error����negative��else����0
int8_t ShareTrackDataGetFlash(uint32_t address_s, ShareTrackStr *share_track_data)
{
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
	if(address_s >= SHARE_TRACK_START_ADDR && ((address_s - SHARE_TRACK_START_ADDR) % ONE_SHARED_TRACK_SIZE == 0))
#else
	if(address_s != SHARE_TRACK_START_ADDR && address_s != (SHARE_TRACK_START_ADDR + ONE_SHARED_TRACK_SIZE))
	{
			return -1;
	}
#endif
	if(NULL == share_track_data)    //��ָ��
		return -2;
	
	dev_extFlash_enable();

  dev_extFlash_read(address_s, (uint8_t*)share_track_data, sizeof(ShareTrackStr));
	
  dev_extFlash_disable();
	
	return 0;
}


//�����洢��flash�еĹ���켣����240k���ɹ�����0��ʧ�ܷ��ظ���
int8_t ShareTrackDataEraseFlash(uint32_t address_s)
{
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
	if(address_s >= SHARE_TRACK_START_ADDR && ((address_s - SHARE_TRACK_START_ADDR) % ONE_SHARED_TRACK_SIZE == 0))
#else
	if(address_s != SHARE_TRACK_START_ADDR && address_s != (SHARE_TRACK_START_ADDR + ONE_SHARED_TRACK_SIZE))
	{
			return -1;
	}
#endif

    dev_extFlash_enable();

    dev_extFlash_erase(address_s, ONE_SHARED_TRACK_SIZE);

    dev_extFlash_disable();	
	
		return 0;
}

//�ж϶����������Ƿ���Ч����ЧΪ1����ЧΪ0
bool IsShareTrackDataValid(ShareTrackStr *share_track_data)
{
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION && defined WATCH_SPORT_EVENT_SCHEDULE
	if(is_sport_type_cloud_navigation_valid(share_track_data->Activity_Type) == false)
	{
		return false;
	}
	if(g_start_search_cloud_status == 1)
	{//ԽҰ�ܷ����� ���ݵ�ʱ�˶������Ƽ����� �����Ľ��뷽ʽ������
		if(share_track_data->Activity_Type != ACT_CROSSCOUNTRY)
		{//���Ͳ���
			return false;
		}
	}
	else if(g_start_search_cloud_status == 2)
	{//ͽ��ԽҰ ������ ���ݵ�ʱ�˶������Ƽ����� �����Ľ��뷽ʽ������
		if(share_track_data->Activity_Type != ACT_HIKING)
		{//���Ͳ���
			return false;
		}
	}


	if(is_sport_type_cloud_navigation_valid(share_track_data->Activity_Type) == true
		 && (share_track_data->gps_end_address > share_track_data->gps_start_address)
		 && share_track_data->Number != 0xFF
		 && share_track_data->new_sign_value == SPORT_NEW_CLOUD_DISTINGUISH_SIGN_VALUE)
	{
		return true;
	}
	else
	{
		return false;
	}
#else
	//��Ч���������ж�
	if(NULL == share_track_data  ||
		share_track_data->gps_end_address == share_track_data->gps_start_address ||
		0xFF == share_track_data->Number)
	{
		return 0;
	}
	else
		return 1;
#endif
}


//��ȡ��Ч������Ŀ���ɹ�������Ч������Ŀ��ʧ�ܷ��ظ���
int8_t ShareTrackDataNUMTraverse(void)
{
	uint8_t valid_num = 0;
	ShareTrackStr tmp_share_track_data;
	
	for(int i = 0; i < SHARED_TRACK_NUM_MAX; i++)
	{
		memset(&tmp_share_track_data,0,sizeof(ShareTrackStr));
		ShareTrackDataGetFlash(SHARE_TRACK_START_ADDR + i * ONE_SHARED_TRACK_SIZE, &tmp_share_track_data);
		if(IsShareTrackDataValid(&tmp_share_track_data))
			valid_num++;
	}
	GUI_TOOL_GPS_PRINTF("[com_sport]:ShareTrackDataTraverse  num=%d\r\n",valid_num);
	return valid_num;
}


//��ȡ��Ч����
int8_t ShareTrackDataTraverse(uint32_t address_s, ShareTrackStr *sport_data_rec, uint8_t num)
{
	uint8_t n = 0;
	ShareTrackStr tmp_share_track_data;
	
	if(address_s != SHARE_TRACK_START_ADDR) //��ʼ��ַ����
	{
			return -1;
	}
	
	if(NULL == sport_data_rec)    //��ָ��
		return -2;
	
	for(int i = 0; i < num; i++)
	{
		memset(&tmp_share_track_data,0,sizeof(ShareTrackStr));
		ShareTrackDataGetFlash(address_s + i * ONE_SHARED_TRACK_SIZE, &tmp_share_track_data);
		
		if(IsShareTrackDataValid(&tmp_share_track_data))
		{
			//��ʱע�͵������ڵ��ԡ�
			memcpy(&sport_data_rec[n++], &tmp_share_track_data, sizeof(ShareTrackStr));
		}
	}
	GUI_TOOL_GPS_PRINTF("[com_sport]:ShareTrackDataTraverse  num=%d\r\n",n);
	return n;
}



#endif
//���Խӿ��ã�����




static ActivityDataStr SPORT_DATA[2] = {0};
static uint8_t NUM_M = 2;                    //һ�ζ���������������ʾ
//static uint8_t VALID_SPORT_NUM = 0;
static uint8_t SPORTS_DATA_TOTAL_NUM = 0;
//static uint32_t FIRST_DATA_ADDRESS;       //��һ�����ݵ�ַ

//��flash��ȡ�˶����ݣ��������ݽṹ�����ڱ�ʾ���ؽ��
typedef struct {
	uint32_t address0;
	uint32_t address1;
	uint8_t number;
}result_s;
 
result_s READ_SPORTS_DATA_RESULT;

//��ȡ�˶����ݽṹ��
uint8_t ActivityDataGetFlash(uint32_t address_s, ActivityDataStr *actdata)
{
    int i;

    if(NULL == actdata)  //��ָ��
        return 1;

    //��ʼ��ַ���� ����������������
		for(i = 0;i < MAX_RECORD_DAY;i++)
		{
			if((address_s >= (DAY_MAIN_DATA_START_ADDRESS + DAY_MAIN_DATA_SIZE * i + DAY_COMMON_DATA_SIZE + 0*ACTIVITY_DAY_DATA_SIZE))
				 && address_s <= (DAY_MAIN_DATA_START_ADDRESS + DAY_MAIN_DATA_SIZE * i + DAY_COMMON_DATA_SIZE + DAY_ACTIVITY_MAX_NUM*ACTIVITY_DAY_DATA_SIZE))
			{//�ҵ�ʱ
				break;
			}
		}
		if(i == MAX_RECORD_DAY)
		{//��δ�ҵ�ʱ
			return 2;
		}

    dev_extFlash_enable();

    dev_extFlash_read(address_s, (uint8_t*)actdata, ACTIVITY_DAY_DATA_SIZE);
	
    dev_extFlash_disable();

    return 0;
}

/*����˵��:�����жϲ���Flash�ж�,��ʱδ��*/
#if 0
uint8_t ActivityDataEraseFlash(uint32_t address_s, uint32_t size)
{
    //int i, j;
    uint32_t tmp;

    //��ʼ��ַ����
		if((address_s < DAY_MAIN_DATA_START_ADDRESS + ACTIVITY_DATA_START_ADDRESS) 
			|| (address_s > (0xD000+DAY_COMMON_DATA_SIZE+(DAY_ACTIVITY_MAX_NUM-1)*sizeof(ActivityDataStr))))
			                                                                                          
        return 2;

    //��ʼ��ַ����
    tmp = (address_s - DAY_MAIN_DATA_START_ADDRESS) / DAY_MAIN_DATA_SIZE ;
    switch(tmp)
    {
        case 0:
            if(((address_s - (DAY_MAIN_DATA_START_ADDRESS + 0 * DAY_MAIN_DATA_SIZE + ACTIVITY_DATA_START_ADDRESS)) % ACTIVITY_DAY_DATA_SIZE))
                return 3;
            break;
        case 1:
            if(((address_s - (DAY_MAIN_DATA_START_ADDRESS + 1 * DAY_MAIN_DATA_SIZE + ACTIVITY_DATA_START_ADDRESS)) % ACTIVITY_DAY_DATA_SIZE))
                return 4;
            break;
        case 2:
            if(((address_s - (DAY_MAIN_DATA_START_ADDRESS + 2 * DAY_MAIN_DATA_SIZE + ACTIVITY_DATA_START_ADDRESS)) % ACTIVITY_DAY_DATA_SIZE))
                return 5;
            break;
        case 3:
            if(((address_s - (DAY_MAIN_DATA_START_ADDRESS + 3 * DAY_MAIN_DATA_SIZE + ACTIVITY_DATA_START_ADDRESS)) % ACTIVITY_DAY_DATA_SIZE))
                return 6;
            break;
        case 4:
            if(((address_s - (DAY_MAIN_DATA_START_ADDRESS + 4 * DAY_MAIN_DATA_SIZE + ACTIVITY_DATA_START_ADDRESS)) % ACTIVITY_DAY_DATA_SIZE))
                return 7;
            break;
        case 5:
            if(((address_s - (DAY_MAIN_DATA_START_ADDRESS + 5 * DAY_MAIN_DATA_SIZE + ACTIVITY_DATA_START_ADDRESS)) % ACTIVITY_DAY_DATA_SIZE))
                return 8;
            break;
        case 6:
            if(((address_s - (DAY_MAIN_DATA_START_ADDRESS + 6 * DAY_MAIN_DATA_SIZE + ACTIVITY_DATA_START_ADDRESS)) % ACTIVITY_DAY_DATA_SIZE))
                return 9;
            break;
        default:
            break;
    }

    dev_extFlash_enable();

    dev_extFlash_erase(address_s, size);

    dev_extFlash_disable();

    return 0;

}
#endif


//�ж��˶������Ƿ�Ϊ��Ч����,valid 1, else 0.
bool IsDataValid(ActivityDataStr *actdata)

{
    if(NULL == actdata)
        return 0;

    if((actdata->Activity_Type >= ACT_RUN) && (actdata->Activity_Type < ACT_TRIATHLON_SWIM) &&
        (actdata->GPS_Start_Address != actdata->GPS_Stop_Address))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


//��������˶����ݣ��ж���Ч������Ŀ����������һ��������ʼ��ַ
uint32_t ActivityDataNUMTraverse(uint32_t address_s)
{
    int i = 0, j = 0;
		
		uint32_t activitynum = 0;
		uint32_t tmp = 0;
		uint32_t tmp_address = 0;
		ActivityDataStr sport_data_rec;
	  rtc_time_t tmp_rtc_time = {0};
		uint32_t address_re;

		uint32_t daynum = (address_s - DAY_MAIN_DATA_START_ADDRESS) / DAY_MAIN_DATA_SIZE;   //�жϴ�����һ��

		//flag = (address_s - DAY_MAIN_DATA_START_ADDRESS) % DAY_MAIN_DATA_SIZE;   //�жϵ�ַ�Ƿ�Ϊÿ�����ݴ洢����ʼ��ַ 

		tmp = address_s - DAY_MAIN_DATA_START_ADDRESS - daynum * DAY_MAIN_DATA_SIZE;
		if(tmp >= ACTIVITY_DATA_START_ADDRESS)      ////�ж�Ϊĳһ��ĵڼ����˶�����
			activitynum = (tmp - ACTIVITY_DATA_START_ADDRESS) / ACTIVITY_DAY_DATA_SIZE;
		else
			activitynum = 0;
		
		SPORTS_DATA_TOTAL_NUM = 0;//ÿ�������¹���ͳ��
    for(i = daynum; i < MAX_RECORD_DAY; i++)
    {
        for(j = activitynum; j < MAX_NUM_ACTIVITY_DATA; j++)
        {			
						tmp_address = i * DAY_MAIN_DATA_SIZE + DAY_MAIN_DATA_START_ADDRESS + ACTIVITY_DATA_START_ADDRESS + j * ACTIVITY_DAY_DATA_SIZE;
						
            ActivityDataGetFlash(tmp_address, &sport_data_rec);
            if(IsDataValid(&sport_data_rec))
            {	
							SPORTS_DATA_TOTAL_NUM++; //�˶���������+1
							
              if(IsActivityDataValid(&sport_data_rec))
							{
								tmp_rtc_time = sport_data_rec.Act_Start_Time;    //��¼�����ʱ��
								address_re = tmp_address;                        //��¼�����ʱ���Ӧ�ĵ�ַ
							}
            }
        }
				
				activitynum = 0;
    }
		
		return address_re;
}



//��������˶����ݣ�����result_s�ṹ��
result_s ActivityDataTraverse(uint32_t address_s, ActivityDataStr *sport_data_rec)
{
    int i = 0, j = 0, n = 0;
		result_s tmp_struct;
		tmp_struct.number = 0;
		uint32_t tmp_address = 0;
		uint32_t activitynum = 0;
		uint32_t tmp = 0;
	
		uint32_t daynum = (address_s - DAY_MAIN_DATA_START_ADDRESS) / DAY_MAIN_DATA_SIZE;   //�жϴ�����һ��
	
		//flag = (address_s - DAY_MAIN_DATA_START_ADDRESS) % DAY_MAIN_DATA_SIZE;   //�жϵ�ַ�Ƿ�Ϊÿ�����ݴ洢����ʼ��ַ 

		tmp = address_s - DAY_MAIN_DATA_START_ADDRESS - daynum * DAY_MAIN_DATA_SIZE;
		if(tmp >= ACTIVITY_DATA_START_ADDRESS)      ////�ж�Ϊĳһ��ĵڼ����˶�����
			activitynum = (tmp - ACTIVITY_DATA_START_ADDRESS) / ACTIVITY_DAY_DATA_SIZE;
		else
			activitynum = 0;
	  

    for(i = daynum; i < MAX_RECORD_DAY; i++)
    {
        for(j = activitynum; j < MAX_NUM_ACTIVITY_DATA; j++)
        {			
						tmp_address = i * DAY_MAIN_DATA_SIZE + DAY_MAIN_DATA_START_ADDRESS + ACTIVITY_DATA_START_ADDRESS + j * ACTIVITY_DAY_DATA_SIZE;
						
            ActivityDataGetFlash(tmp_address, &sport_data_rec[NUM_M - n - 1]);
            if(IsDataValid(&sport_data_rec[NUM_M - n - 1]))
            {								
							n++;
							tmp_struct.number = n;
							if(1 == n) 
							{
								tmp_struct.address0 = tmp_address;
							}
							else
							{
								tmp_struct.address1 = tmp_address;
							}

            }

            if(n == NUM_M)   //��������������
						{
                goto RESULT;
						}
        }
				
				if(j == MAX_NUM_ACTIVITY_DATA)        //����
				{
					activitynum = 0;
				}
    }
		
RESULT:

    return tmp_struct;
}



//��������˶����ݣ�����result_s�ṹ��
result_s ActivityDataReverseTraverse(uint32_t address_s, ActivityDataStr *sport_data_rec)
{
    int8_t i = 0, j = 0, n = 0;
		result_s tmp_struct;
		tmp_struct.number = 0;
		uint32_t tmp_address = 0;
		uint32_t activitynum = 0;
		uint32_t tmp = 0, flag = 0;
		
		uint32_t daynum = (address_s - DAY_MAIN_DATA_START_ADDRESS) / DAY_MAIN_DATA_SIZE;    //��һ��
		if(daynum > 6)
			goto RESULT;

		flag = (address_s - DAY_MAIN_DATA_START_ADDRESS) % DAY_MAIN_DATA_SIZE;   //�жϵ�ַ�Ƿ�Ϊÿ�����ݴ洢����ʼ��ַ 
	
		if(0 == daynum && 0 == flag)   //��һ�����ʼ��ַ
			goto RESULT;

		tmp = address_s - DAY_MAIN_DATA_START_ADDRESS - daynum * DAY_MAIN_DATA_SIZE;
		
		if(tmp >= ACTIVITY_DATA_START_ADDRESS)       //�ж�Ϊĳһ��ĵڼ����˶�����
		{
			activitynum = (tmp - ACTIVITY_DATA_START_ADDRESS) / ACTIVITY_DAY_DATA_SIZE;
		}
		else
			activitynum = 0;
		
    for(i = daynum; i >= 0; i--)
    {
        for(j = activitynum ; j >= 0; j--)
        {			
						tmp_address = i * DAY_MAIN_DATA_SIZE + DAY_MAIN_DATA_START_ADDRESS + ACTIVITY_DATA_START_ADDRESS + j * ACTIVITY_DAY_DATA_SIZE;
						
            ActivityDataGetFlash(tmp_address, &sport_data_rec[n]);
            if(IsDataValid(&sport_data_rec[n]))
            {
							n++;
							tmp_struct.number = n;
							if(1 == n)
							{
								tmp_struct.address1 = tmp_address;
							}
							else
							{
								tmp_struct.address0 = tmp_address;
							}
							
            }

            if(n == NUM_M)   //��������������
						{
                goto RESULT;
						}
        }
				
				if(j == -1)        //����
				{
					activitynum = DAY_ACTIVITY_MAX_NUM - 1;
				}
    }
		
RESULT:
    return tmp_struct;
}






extern _drv_bat_event g_bat_evt;

typedef enum
{
	Projection = 0U,//ͶӰ�ִ�
	MotionTrail = 1U,//�˶��켣
	//Navigation = 2U//�Ƽ�����
}GPS_DETAIL_OPS;

const char* cGpsDetailStrs[] = 
{
	"ͶӰ�ִ�",
	"�˶��켣",
	//"�Ƽ�����"
};

const char* eGpsDetailStrs[] = 
{
	"projection zoning",
	"motion trail",
	//"navigation from cloud"
};


const char** GpsDetailStrs[] = {cGpsDetailStrs,eGpsDetailStrs};


static GPS_DETAIL_OPS gps_detail_index = Projection; //gps���ý���ѡ������

typedef enum
{
	Degree3 = 0U,
	Degree6 = 1U
}PROJECTION_OPS;

const char* cProjectionStrs[] = 
{
	"���ȴ�",
	"���ȴ�"
};

const char* eProjectionStrs[] = 
{
	"3degree strap",
	"6degree strap"
};

static PROJECTION_OPS projection_index = Degree3; //ͶӰ�ִ���������



const char** ProjectionStrs[] = {cProjectionStrs,eProjectionStrs};

static uint8_t navigation_index = 0;    //�Ƽ���������

uint8_t get_navigation_index(void)
{
	return navigation_index;
}

typedef enum
{
	Begin = 0U,
	Look = 1U,
	Delete = 2U
}NAVIGATION_OPS;

const char* cNavigationStrs[] = 
{
	"��ʼ����",
	"�鿴�켣",
	"ɾ��"
};

const char* eNavigationStrs[] = 
{
	"start navigation",
	"view the record",
	"delete"
};

const char** NavigationStrs[] = {cNavigationStrs,eNavigationStrs};


static NAVIGATION_OPS navigation_ops_index = Begin; //ͶӰ�ִ���������


typedef enum
{
	No = 0U,
	Yes = 1U
}DELETE_OPS;

const char* cDeleteStrs[] = 
{
	"��",
	"��"
};

const char* eDeleteStrs[] = 
{
	"no",
	"yes"
};

const char** DeleteStrs[] = {cDeleteStrs,eDeleteStrs};


static DELETE_OPS delete_ops_index = No; //�Ƽ�����ɾ����������


typedef enum
{
	StartNavigation = 0U,
	ViewTrail = 1U
}MOTION_TRAIL_OPS;

const char* cStartNavigationStrs[] = 
{
	"��ʼ�켣",
	"�鿴�켣"
};

const char* eStartNavigationStrs[] = 
{
	"start navigation",
	"view trail"
};

const char** StartNavigationStrs[] = {cStartNavigationStrs, eStartNavigationStrs};


//MOTION_TRAIL_OPS motion_trail_index = StartNavigation;   //�˶��켣�����������

MOTION_TRAIL_OPS motion_trail_index = ViewTrail;   //�˶��켣�����������

const char* sports_type_str_c[] = {"�ܲ�",
																	"����", 
																	"������", 
																	"��Ӿ", 
																	"������", 
																	"ԽҰ��", 
																	"ͽ��ԽҰ", 
																	"��ɽ", 
																	"����",
																	"��������"};

const char* sports_type_str_e[] = {"Run",
																	"FitnessWalk", 
																	"Marathon", 
																	"Swimming", 
																	"IndoorRun", 
																	"CrosscountryRun", 
																	"CrosscountryHike", 
																	"Climbing", 
																	"Cycling",
																	"Triathlon"};
																	
const char** SportsTypeStrs[] = {sports_type_str_c, sports_type_str_e};



static uint32_t sport_data_index = 0;    //�洢�˶���������





void gui_tool_gps_paint(GPSMode *gps)
{
   
   LCD_SetBackgroundColor(LCD_BLACK);
	SetWord_t word = {0};
	uint8_t i,gps_n = 0,bds_n = 0;
	char str[20];
	uint8_t a,b,c;
	
	//LCD_SetRectangle(84, 126, 40, 160, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	//LCD_SetPicture(12,LCD_CENTER_JUSTIFIED,LCD_RED,LCD_NONE,&img_tools_lists[5]);
   
     gui_sport_gps_status(SPORT_GPS_UP);
	
	
			//GPS + BEIDOU
	//�������������ź�ǿ��
	if(gps->possl_gpsnum == 0)
	{
		gps_n = 0;
	}
	else if(gps->possl_gpsnum < 3)
	{
		gps_n = 1;
	}
	else if(gps->possl_gpsnum < 5)
	{
		gps_n = 2;
	}
	else if(gps->possl_gpsnum < 7)
	{
		gps_n = 3;
	}
	else if(gps->possl_gpsnum < 9)
	{
		gps_n = 4;
	}
	else
	{
		gps_n = 5;
	}
	
	if(gps->possl_bdnum == 0)
	{
		if(gps->status == 'A')
		{
			//�����λ�ɹ������ޱ�������ʱ���ֶ����һ�ű�������
			bds_n = 1;
		}
		else
		{
			bds_n = 0;
		}
	}
	else if(gps->possl_bdnum < 3)
	{
		bds_n = 1;
	}
	else if(gps->possl_bdnum < 5)
	{
		bds_n = 2;
	}
	else if(gps->possl_bdnum < 7)
	{
		bds_n = 3;
	}
	else if(gps->possl_bdnum < 9)
	{
		bds_n = 4;
	}
	else
	{
		bds_n = 5;
	}
	
	//GPS
	word.x_axis = 60-14;
	word.y_axis = 48;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_RED, LCD_NONE, &img_tool_gps_flag);
	
	//BDS
	word.y_axis += img_tool_gps_flag.width + 28;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_CYAN, LCD_NONE, &img_tool_bds_flag);
	
	
	word.x_axis += img_tool_bds_flag.height + 6 + 25;
	word.y_axis = 50;
	
	//GPS�ź�ǿ��
	for(i = 0;i < 5; i++)
	{
		//LCD_SetRectangle(word.x_axis, 22, word.y_axis, 8, LCD_WHITE, 1, 1, LCD_FILL_DISABLE);
		
		if(i < gps_n)
		{
			LCD_SetRectangle(word.x_axis - (i+1)*5 , (i+1)*5, word.y_axis, 8, LCD_RED, 1, 1, LCD_FILL_ENABLE);
		}
		else
		{
            
			LCD_SetRectangle(word.x_axis - (i+1)*5 , (i+1)*5, word.y_axis, 8, LCD_GRAY, 1, 1, LCD_FILL_ENABLE);
         }
		word.y_axis += 8 + 4;
	}
	
	word.y_axis += 24;
	//BDS�ź�ǿ��
	for(i = 0;i < 5; i++)
	{
		

		if(i < bds_n)
		{
			LCD_SetRectangle(word.x_axis - (i+1)*5 , (i+1)*5, word.y_axis, 8, LCD_CYAN, 1, 1, LCD_FILL_ENABLE);
		}
		else
		{
            
			LCD_SetRectangle(word.x_axis - (i+1)*5 , (i+1)*5, word.y_axis, 8, LCD_GRAY, 1, 1, LCD_FILL_ENABLE);
         }
		word.y_axis += 8 + 4;
	}
	//����
	word.x_axis = 128;
	word.y_axis = 72;
	word.size = LCD_FONT_16_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	if(gps->ewhemi == 'W')
	{
		LCD_SetString("W",&word);
	}
	else
	{
		LCD_SetString("E",&word);
	}
	if(gps->longitude != 0)
	{
		a = (uint8_t)(gps->longitude/GPS_SCALE_FLOAT);
		b = (uint8_t)((gps->longitude/GPS_SCALE_FLOAT-a)*60.f);
		c = (uint8_t)(((gps->longitude/GPS_SCALE_FLOAT-a)*60.f-b)*60.f);
		word.y_axis += (16);
		memset(str,0x0,20);
		sprintf(str,"%d %d'%d\"",a,b,c);
		LCD_SetString(str,&word);
		
		memset(str,0x0,20);
		sprintf(str,"%d",a);
		word.x_axis += 1;
		word.y_axis += (8*strlen(str)+1);
		LCD_SetPicture(word.x_axis, word.y_axis, LCD_WHITE, LCD_NONE, &Img_Degree_6X6);
		word.x_axis -= 1;
	}
	else
	{
		LCD_SetRectangle(word.x_axis + 13, 1, word.y_axis+16, 80, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	}
	
	//γ��
	word.x_axis += 20;
	word.y_axis = 72;
	if(gps->nshemi == 'S')
	{
		LCD_SetString("S",&word);
	}
	else
	{
		LCD_SetString("N",&word);
	}
	if(gps->latitude != 0)
	{
		a = (uint8_t)(gps->latitude/GPS_SCALE_FLOAT);
		b = (uint8_t)((gps->latitude/GPS_SCALE_FLOAT-a)*60.f);
		c = (uint8_t)(((gps->latitude/GPS_SCALE_FLOAT-a)*60.f-b)*60.f);
		word.y_axis += (16);
		memset(str,0x0,20);
		sprintf(str,"%d %d'%d\"",a,b,c);
		LCD_SetString(str,&word);
		
		memset(str,0x0,20);
		sprintf(str,"%d",a);
		word.x_axis += 1;
		word.y_axis += (8*strlen(str)+1);
		LCD_SetPicture(word.x_axis, word.y_axis, LCD_WHITE, LCD_NONE, &Img_Degree_6X6);
		word.x_axis -= 1;
	}
	else
	{
		LCD_SetRectangle(word.x_axis + 13,1,word.y_axis+16,80,LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	}

	//��˹X����
	ST_GPSXY_POINT st_guass = {0};
	ST_GPS_POINT st_gps = {0};
	uint8_t st_num = 0;
	
	st_gps.sgp_lat = gps->latitude/GPS_SCALE_FLOAT;
	st_gps.sgp_lon = gps->longitude/GPS_SCALE_FLOAT;
	
	//��˹�㷨��ʼ��
	algo_gauss_init(SetValue.GPS_Gauss_Type, st_gps.sgp_lon);
	//����
	st_num = algo_gauss_number_get();
	//��˹����
	st_guass = algo_gauss_point_get(st_gps);
	
	word.x_axis += 20;
	word.y_axis = 72;
	LCD_SetString("X",&word);
	
	if(gps->longitude != 0)
	{
		word.y_axis += (16);
		memset(str,0x0,20);
		#if defined (SPRINTF_FLOAT_TO_INT)
		sprintf(str,"%d.%03d",(int32_t)st_guass.x,(uint32_t)(fabs(st_guass.x)*1000)%1000);
		#else
		sprintf(str,"%.3f",st_guass.x);
		#endif
		LCD_SetString(str,&word);
	}
	else
	{
		LCD_SetRectangle(word.x_axis + 13, 1, word.y_axis+16, 80, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	}
	
	//��˹Y����
	word.x_axis += 20;
	word.y_axis = 72;
	LCD_SetString("Y",&word);
	
	if(gps->longitude != 0)
	{
		word.y_axis += (16);
		memset(str,0x0,20);
		#if defined (SPRINTF_FLOAT_TO_INT)
		sprintf(str,"%d%d.%03d",st_num,(int32_t)(st_guass.y+500000),(uint32_t)(fabs(st_guass.y+500000)*1000)%1000);
		#else
		sprintf(str,"%d%.3f",st_num,st_guass.y+500000);
		#endif
		LCD_SetString(str,&word);
	}
	else
	{
		LCD_SetRectangle(word.x_axis + 13, 1, word.y_axis+16, 80, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	}
}




void gui_tool_gps_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch(Key_Value)
	{
		case KEY_OK:
			{
				//����GPS��ϸ����
				ScreenState = DISPLAY_SCREEN_GPS_DETAIL;
				gps_detail_index = Projection;
				msg.cmd = MSG_DISPLAY_SCREEN;
				//msg.value = Get_Curr_Tool_Index();
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
	
		case KEY_BACK:
			//���ع��߲˵�����
			tool_task_close();
			ScreenState = DISPLAY_SCREEN_TOOL;
			msg.cmd = MSG_DISPLAY_SCREEN;
			msg.value = Get_Curr_Tool_Index();
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}

}


void gui_gps_detail_options_paint(uint8_t index)
{
    SetWord_t word = {0};
    
	word.x_axis = 120 - 24/2;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_24_SIZE;
    word.forecolor = LCD_CYAN;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString((char *)GpsDetailStrs[SetValue.Language][index],&word);

   
    word.x_axis = 120 + 24/2 + 36 + 24;
    word.forecolor = LCD_GRAY;
	word.size = LCD_FONT_16_SIZE;
    if (index == Projection)
    {
		LCD_SetString((char *)GpsDetailStrs[SetValue.Language][index + 1],&word);
    }
	else
		{
          
		  LCD_SetString((char *)GpsDetailStrs[SetValue.Language][index - 1],&word);


	   }
}


void gui_gps_detail_paint(void)
{
	SetWord_t word = {0};
   //����������Ϊ��ɫ
	LCD_SetBackgroundColor(LCD_BLACK);

	//λ�õ���ͼ��
	LCD_SetPicture(120 - 24/2 - 36 - 24 - 16/2 - img_tools_lists[0].height/2,120-24-img_tools_lists[0].width,LCD_RED,LCD_NONE,&img_tools_lists[5]);
	

	
	
		word.x_axis = 120 - 24/2 - 36 - 24 - 16;
		word.y_axis = 102;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
	 LCD_SetString((char *)ToolStrs[SetValue.Language][5],&word);
	
	
	LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	
	LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);

	gui_gps_detail_options_paint(gps_detail_index);


	
	//���Ƽ�����
	LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
}
/*��ʼ���켣*/
void init_cloud_navigation(uint32_t goto_type)
{
#if defined WATCH_SPORT_EVENT_SCHEDULE
		if(goto_type == GOTO_SHARE_TRACK_BY_COUNTRYRUN)
		{//��ԽҰ�ܽ�����Ƽ�����
			g_start_search_cloud_status = 1;//�Ƽ�����������ʽ0 ���˶����� 1�˶���������
		}
		else if(goto_type == GOTO_SHARE_TRACK_BY_HIKING)
		{
			g_start_search_cloud_status = 2;//�Ƽ�����������ʽ0 ���˶����� 1�˶���������
		}
		else
		{//GPS���߽�����Ƽ�����
			g_start_search_cloud_status = 0;//�Ƽ�����������ʽ0 ���˶����� 1�˶��������� 2GPS���߽�����Ƽ�����
		}
	
#endif
	navigation_index = 0;
	ShareTrackDataValidNum = ShareTrackDataNUMTraverse();
	if(ShareTrackDataValidNum < 0)
	{
		ShareTrackDataValidNum = 0;
	}

	if(ShareTrackDataTraverse(SHARE_TRACK_START_ADDR, ShareTrack, SHARED_TRACK_NUM_MAX) >= 0)
	{
		ScreenState = DISPLAY_SCREEN_CLOUD_NAVIGATION;
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
		//��ת���� ����
		if(goto_type == GOTO_SHARE_TRACK_BY_COUNTRYRUN)
		{
			set_share_track_back_discreen(GOTO_SHARE_TRACK_BY_COUNTRYRUN);//���ý����Ƽ�������ʽ
		}
		else if(goto_type == GOTO_SHARE_TRACK_BY_HIKING)
		{
			set_share_track_back_discreen(GOTO_SHARE_TRACK_BY_HIKING);//���ý����Ƽ�������ʽ
		}
		else
		{
			set_share_track_back_discreen(GOTO_SHARE_TRACK_BY_GPS);//���ý����Ƽ�������ʽ
		}
#endif
	}
}
void gui_gps_detail_btn_evt(uint32_t Key_Value)
{
	uint32_t tmp_addr;
	
	switch( Key_Value )
	{
		case KEY_OK:  
					if(gps_detail_index == Projection)
					{
						ScreenState = DISPLAY_SCREEN_PROJECTION_ZONING;
						if(SetValue.GPS_Gauss_Type == 6)
						{
							projection_index = Degree6;
						}
						else
						{
							projection_index = Degree3;
						}
					}
				/*	else if(gps_detail_index == Navigation)
					{
				g_tool_or_sport_goto_screen = DISPLAY_SCREEN_GPS_DETAIL;
						init_cloud_navigation(0);
					}*/
					else if(gps_detail_index == MotionTrail)
					{	
							SPORTS_DATA_TOTAL_NUM = 0;
							sport_data_index = 0;
				
						  tmp_addr = ActivityDataNUMTraverse(DAY_MAIN_DATA_START_ADDRESS);

							READ_SPORTS_DATA_RESULT = ActivityDataReverseTraverse(tmp_addr, SPORT_DATA);
	
							ScreenState = DISPLAY_SCREEN_MOTION_TRAIL;
					}
          gui_swich_msg();
			break;
		case KEY_UP:
			if((gps_detail_index > Projection) && (gps_detail_index <= MotionTrail))
			{
				gps_detail_index--; 
			}
			else
			{
				gps_detail_index = MotionTrail;
			}
			gui_swich_msg();
			break;
		case KEY_DOWN:
			if (gps_detail_index < MotionTrail)
			{
				gps_detail_index++;
			}
			else
			{
				gps_detail_index = Projection;
			}
			gui_swich_msg();
			break;

		case KEY_BACK:
				 ScreenState = DISPLAY_SCREEN_GPS;
				 gui_swich_msg();
			break;
		default:
			break;
	}	
}

static void gui_projection_options_paint(uint8_t index)
{
    SetWord_t word = {0};
    
    word.x_axis = 120 - 24/2;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_24_SIZE;
    word.forecolor = LCD_CYAN;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString((char *)ProjectionStrs[SetValue.Language][index],&word);

    word.x_axis = 120 + 24/2 + 36 + 24;
    word.forecolor = LCD_GRAY;
	word.size = LCD_FONT_16_SIZE;
    if (index == Degree3)
    {
        LCD_SetString((char *)ProjectionStrs[SetValue.Language][index + 1],&word);
    }
	else
	{
         
	  LCD_SetString((char *)ProjectionStrs[SetValue.Language][index - 1],&word);

   }

}


void gui_projection_paint(void)
{
	SetWord_t word = {0};
    LCD_SetBackgroundColor(LCD_BLACK);
    word.x_axis = 120 - 24/2 - 36 - 24 - 16;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetString("ͶӰ�ִ�",&word);
	LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);

	
	gui_projection_options_paint(projection_index);

	//LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
	
	LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
}

void gui_projection_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:  
			if(projection_index == Degree3)
			{
				//����Ϊ3�ȴ�
				SetValue.GPS_Gauss_Type = 3;
			}
			else if(projection_index == Degree6)
			{
				//����Ϊ6�ȴ�
				SetValue.GPS_Gauss_Type = 6;
			}
			ScreenState = DISPLAY_SCREEN_GPS;
			gui_swich_msg();
			break;
		case KEY_UP:
			if (projection_index > Degree3)
			{
				projection_index--;
			}
			else
			{	
				projection_index = Degree6;
			}
			gui_swich_msg();
			break;
		case KEY_DOWN:
			if (projection_index < Degree6)
			{
				projection_index++;
			}
			else
			{	
				projection_index = Degree3;
			}
			gui_swich_msg();
			break; 
		case KEY_BACK:
			ScreenState = DISPLAY_SCREEN_GPS_DETAIL;
			gui_swich_msg();
			break;
		default:
			break;
	}	

}





static void gui_cloud_navigation_options_paint(uint8_t index)    //������
{
    SetWord_t word = {0};
    char tmpstr[18] = {0};
		
		if(0 == ShareTrackDataValidNum)
		{   
			word.x_axis = 110;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_24_SIZE;
			word.forecolor = LCD_BLACK;
			word.bckgrndcolor = LCD_NONE;
			LCD_SetString("���˶�����",&word);
		}
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
		else if(ShareTrackDataValidNum > 0)
		{
			//Name
			word.x_axis = 98;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_24_SIZE;
			word.forecolor = LCD_BLACK;
			word.bckgrndcolor = LCD_NONE;
			memset(tmpstr, 0, sizeof(tmpstr) / sizeof(tmpstr[0]));
			memcpy(tmpstr, ShareTrack[index].Name, 16);
			LCD_SetString(tmpstr, &word);
			
			//Activity_Type
			word.x_axis = 132;
			word.y_axis = 60;
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_BLACK;
			word.bckgrndcolor = LCD_NONE;
			LCD_SetString((char *)SportsTypeStrs[SetValue.Language][(ShareTrack[index].Activity_Type & 0x0F) - 1], &word);
			
			//distance
			word.x_axis = 132;
			word.y_axis += 70;
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_BLACK;
			word.bckgrndcolor = LCD_NONE;
			memset(tmpstr, 0, sizeof(tmpstr) / sizeof(tmpstr[0]));
			sprintf(tmpstr, "%.2fkm", ShareTrack[index].diatance / 1000.f);    //App����������mΪ��λ
			LCD_SetString(tmpstr, &word);
			
			if(ShareTrackDataValidNum == 2)
			{
				word.x_axis = 183; 
				word.y_axis = LCD_CENTER_JUSTIFIED;
				word.size = LCD_FONT_16_SIZE;
				memset(tmpstr, 0, sizeof(tmpstr) / sizeof(tmpstr[0]));
				if(index == 0)
				{
					memcpy(tmpstr, ShareTrack[1].Name, 16);
				}
				else if(index == 1)
				{
					memcpy(tmpstr, ShareTrack[0].Name, 16);
				}
				LCD_SetString(tmpstr, &word);
			}
			else if(ShareTrackDataValidNum > 2)
			{
				if(index <= ShareTrackDataValidNum - 2)
				{
					word.x_axis = 183; 
					word.y_axis = LCD_CENTER_JUSTIFIED;
					word.size = LCD_FONT_16_SIZE;
					memset(tmpstr, 0, sizeof(tmpstr) / sizeof(tmpstr[0]));
					memcpy(tmpstr, ShareTrack[index + 1].Name, 16);
					LCD_SetString(tmpstr, &word);
				}
				else if(index == ShareTrackDataValidNum - 1)
				{
					word.x_axis = 183; 
					word.y_axis = LCD_CENTER_JUSTIFIED;
					word.size = LCD_FONT_16_SIZE;
					memset(tmpstr, 0, sizeof(tmpstr) / sizeof(tmpstr[0]));
					memcpy(tmpstr, ShareTrack[0].Name, 16);
					LCD_SetString(tmpstr, &word);
				}
			}
		}
#else
		else if(1 == ShareTrackDataValidNum)
		{
			//Name
			word.x_axis = 98;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_24_SIZE;
			word.forecolor = LCD_BLACK;
			word.bckgrndcolor = LCD_NONE;
			memset(tmpstr, 0, sizeof(tmpstr) / sizeof(tmpstr[0]));
			memcpy(tmpstr, ShareTrack[index].Name, 16);
			LCD_SetString(tmpstr, &word);
			
			//Activity_Type
			word.x_axis = 132;
			word.y_axis = 60;
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_BLACK;
			word.bckgrndcolor = LCD_NONE;
			LCD_SetString((char *)SportsTypeStrs[SetValue.Language][(ShareTrack[index].Activity_Type & 0x0F) - 1], &word);
			
			//distance
			word.x_axis = 132;
			word.y_axis += 70;
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_BLACK;
			word.bckgrndcolor = LCD_NONE;
			memset(tmpstr, 0, sizeof(tmpstr) / sizeof(tmpstr[0]));
			sprintf(tmpstr, "%.2fkm", ShareTrack[index].diatance / 1000.f);    //App����������mΪ��λ
			LCD_SetString(tmpstr, &word);

		}
		else
		{
			//Name
			word.x_axis = 98;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_24_SIZE;
			word.forecolor = LCD_BLACK;
			word.bckgrndcolor = LCD_NONE;
			memset(tmpstr, 0, sizeof(tmpstr) / sizeof(tmpstr[0]));
			memcpy(tmpstr, ShareTrack[index].Name, 16);
			LCD_SetString(tmpstr, &word);
			
			//Activity_Type
			word.x_axis = 132;
			word.y_axis = 60;
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_BLACK;
			word.bckgrndcolor = LCD_NONE;
			LCD_SetString((char *)SportsTypeStrs[SetValue.Language][(ShareTrack[index].Activity_Type & 0x0F) - 1], &word);
			
			//distance
			word.x_axis = 132;
			word.y_axis += 70;
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_BLACK;
			word.bckgrndcolor = LCD_NONE;
			memset(tmpstr, 0, sizeof(tmpstr) / sizeof(tmpstr[0]));
			sprintf(tmpstr, "%.2fkm", ShareTrack[index].diatance / 1000.f);    //App����������mΪ��λ
			LCD_SetString(tmpstr, &word);
			
			if(index < ShareTrackDataValidNum - 1 && ShareTrackDataValidNum > 0)
			{
				word.x_axis = 183; 
				word.y_axis = LCD_CENTER_JUSTIFIED;
				word.size = LCD_FONT_16_SIZE;
				memset(tmpstr, 0, sizeof(tmpstr) / sizeof(tmpstr[0]));
				memcpy(tmpstr, ShareTrack[index + 1].Name, 16);
				LCD_SetString(tmpstr, &word);
			}
		}
#endif
    
}

void gui_cloud_navigation_paint(void)
{
	SetWord_t word = {0};
	
	LCD_SetBackgroundColor(LCD_BLACK);
	
	word.x_axis = 40;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetString("�Ƽ�����",&word);
	
	LCD_SetRectangle(80,160,0,LCD_LINE_CNT_MAX,LCD_WHITE,0,0,LCD_FILL_ENABLE);
	
	LCD_SetLine( 80, 0, 80, 239, LCD_BLACK, 1);

	gui_cloud_navigation_options_paint(navigation_index);

	LCD_SetLine( 160, 0, 160, 239, LCD_BLACK, 1);
	
	LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Up_12X8);

	if(navigation_index < ShareTrackDataValidNum - 1 && ShareTrackDataValidNum > 0)
		LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Down_12X8);

}


uint32_t get_navigation_gps_startaddress(void)
{
    return ShareTrack[navigation_index].gps_start_address;
}

uint32_t get_navigation_gps_endaddress(void)
{
    return ShareTrack[navigation_index].gps_end_address;
}

uint32_t get_track_point_startaddress(void)
{
    return ShareTrack[navigation_index].point_start_address;
}

uint32_t get_track_point_endaddress(void)
{
    return ShareTrack[navigation_index].point_end_address;
}


void gui_cloud_navigation_btn_item (void)
{
	navigation_ops_index = Begin;
	if(ShareTrackDataValidNum != 0)
	 {
		 ScreenState = DISPLAY_SCREEN_NAVIGATION_OPS;
		 gui_swich_msg();
	 }

}

void gui_cloud_navigation_load(void)
{
    if(ShareTrackDataValidNum != 0)
    {   
        LOADINGFLAG =1;       //�����б�־
        ScreenState = DISPLAY_SCREEN_TRACK_LOAD;      
        gui_swich_msg();
        Send_Timer_Cmd(CMD_NAVIGATION_LOAD);
        
    }   
}

void gui_cloud_navigation_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK: 
				gui_cloud_navigation_load();
			break;
		case KEY_UP:
         if (navigation_index > 0)
         {
           navigation_index-- ;
					 
					 gui_swich_msg(); 
         }
			break;
		case KEY_DOWN:
         if (ShareTrackDataValidNum > 0 && navigation_index < (ShareTrackDataValidNum - 1))
         {
           navigation_index++ ;
					 
					 gui_swich_msg();
         }
				 break; 
		case KEY_BACK:
		{
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
			ShareTrackDataValidNum = 0;   //�˳�ʱ��Ч����������
			
			if(g_tool_or_sport_goto_screen == DISPLAY_SCREEN_CROSSCOUNTRY)
			{
				ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY;
				init_sport_event_data_str(ACT_CROSSCOUNTRY,true);
			}
			else if(g_tool_or_sport_goto_screen == DISPLAY_SCREEN_CROSSCOUNTRY_HIKE)
			{
				ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE;
				init_sport_event_data_str(ACT_HIKING,true);
			}
			else
			{
				ScreenState = DISPLAY_SCREEN_GPS_DETAIL;
			}
			set_share_track_back_discreen(GOTO_SHARE_TRACK_BY_NONE);//���뷽ʽ����
#else
				 ShareTrackDataValidNum = 0;   //�˳�ʱ��Ч����������
				 ScreenState = DISPLAY_SCREEN_GPS_DETAIL;
#endif
				 gui_swich_msg();
		}
			break;
		default:
			break;
	}	

}

void gui_navigation_ops_paint(void)
{

}

void gui_navigation_ops_btn_evt(uint32_t Key_Value)
{

}

static void gui_navigation_item_ops_paint(uint8_t index)
{
    SetWord_t word = {0};
    
    word.x_axis = 110;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_24_SIZE;
    word.forecolor = LCD_BLACK;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString((char *)NavigationStrs[SetValue.Language][index],&word);

    word.x_axis = 183; 
		word.size = LCD_FONT_16_SIZE;
    if (index != Delete)
    {
        LCD_SetString((char *)NavigationStrs[SetValue.Language][index + 1],&word);
    }

}

void gui_navigation_item_paint(void)
{
	SetWord_t word = {0};
	char tmpstr[18] = {0};
	
	if(navigation_ops_index == Begin)
	{		
		LCD_SetBackgroundColor(LCD_BLACK);
	
		word.x_axis = 40;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		memset(tmpstr, 0, sizeof(tmpstr) / sizeof(tmpstr[0]));
		memcpy(tmpstr, ShareTrack[navigation_index].Name, 16);
		LCD_DisplayGTString(tmpstr,&word);
	}
	else
	{
		LCD_SetBackgroundColor(LCD_WHITE);
	
		word.x_axis = 40;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_NONE;
		LCD_SetString((char *)NavigationStrs[SetValue.Language][navigation_ops_index - 1],&word);	
		
	}

	LCD_SetRectangle(80,160,0,LCD_LINE_CNT_MAX,LCD_WHITE,0,0,LCD_FILL_ENABLE);

	LCD_SetLine( 80, 0, 80, 239, LCD_BLACK, 1);
	
	gui_navigation_item_ops_paint(navigation_ops_index);

	LCD_SetLine( 160, 0, 160, 239, LCD_BLACK, 1);

	LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Up_12X8);	
	
	LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Down_12X8);
}

void gui_navigation_item_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};

	switch( Key_Value )
	{
		case KEY_OK:  
		{
					if(navigation_ops_index == Begin)
					{
						if(0 == IS_SPORT_CONTINUE_LATER) 
						{
							ENTER_NAVIGATION = 1;    //���붨λ�����ǣ�������ǿ�ʼ������������˶�ready�����������������˶�ready���水���ؼ����ɷ��ع��߶�λ���档
							//ScreenState = DISPLAY_SCREEN_PROJECTION_ZONING;
//							SEGGER_RTT_printf(0, "ShareTrack[%d].Activity_Type = 0x%x\n", navigation_index, ShareTrack[navigation_index].Activity_Type);
							switch(ShareTrack[navigation_index].Activity_Type)
							{
								case ACT_RUN:
								ReadySport(DISPLAY_SCREEN_RUN);  
#if defined WATCH_SIM_SPORT
#elif defined WATCH_COM_SPORT
								 ScreenState = DISPLAY_SCREEN_RUN;
								 gui_sport_run_init();
#else

								ScreenState = DISPLAY_SCREEN_RUN_READY;
#endif
								break;
								case ACT_WALK:
								 ReadySport(DISPLAY_SCREEN_WALK);     
								 ScreenState = DISPLAY_SCREEN_WALK;
								 gui_sport_walk_init();
								break;
								case ACT_MARATHON:
								ReadySport(DISPLAY_SCREEN_MARATHON);   
							 	ScreenState = DISPLAY_SCREEN_MARATHON;
								gui_sport_marathon_init();
								break;
								case ACT_SWIMMING:
								ReadySport(DISPLAY_SCREEN_SWIMMING); 
#if defined WATCH_SIM_SPORT
#elif  defined WATCH_COM_SPORT
							ScreenState = DISPLAY_SCREEN_SWIMMING;
							gui_sport_swim_init();

#else

								ScreenState = DISPLAY_SCREEN_SWIM_READY;
#endif
								break;
								case ACT_INDOORRUN:
//								 ReadySport();    �����˶���ʱû��ʵ�� 
//								 ScreenState = ;
								break;
								case ACT_CROSSCOUNTRY:
								ReadySport(DISPLAY_SCREEN_CROSSCOUNTRY); 
#if defined WATCH_COM_SPORT
								ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY;
								gui_sport_crosscountry_init();
#else
								ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_READY;
#endif
								break;
								case ACT_HIKING:
								ReadySport(DISPLAY_SCREEN_CROSSCOUNTRY_HIKE);    //ͽ��ԽҰ
								ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE;
								gui_sport_crosscountry_hike_init();
								break;
								case ACT_CLIMB:
								ReadySport(DISPLAY_SCREEN_CLIMBING);
#if defined WATCH_COM_SPORT
								ScreenState = DISPLAY_SCREEN_CLIMBING;
								gui_sport_climbing_init();
#else

								ScreenState = DISPLAY_SCREEN_CLIMBING_READY;
#endif
								break;
								case ACT_CYCLING:
								ReadySport(DISPLAY_SCREEN_CYCLING);    
								ScreenState = DISPLAY_SCREEN_CYCLING;
								gui_sport_cycling_init();
								break;
							
								default:
								//
								break;
							}
								//׼��״̬
							g_sport_status = SPORT_STATUS_READY;
							memset(&TRACE_BACK_DATA, 0, sizeof(TraceBackStr));
							TRACE_BACK_DATA.sport_distance = ShareTrack[navigation_index].diatance *100;
              TRACE_BACK_DATA.sport_traceback_flag = 1;
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
					set_share_track_back_discreen(GOTO_SHARE_TRACK_BY_GPS);
#else
							msg.cmd = MSG_DISPLAY_SCREEN;
							xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
#endif
						}
						else
						{
							ScreenState = DISPLAY_SCREEN_GPS_TRAIL_REMIND;
							SCREEN_STATE_S = DISPLAY_SCREEN_NAVIGATION_OPS;
							gui_swich_msg();
						}

					}
					else if(navigation_ops_index == Look)
					{
						ScreenState = DISPLAY_SCREEN_CLOUD_NAVIGATION_TRAIL;
					}
					else if(navigation_ops_index == Delete)
					{
						ScreenState = DISPLAY_SCREEN_CLOUD_NAVIGATION_DEL;
					}
          gui_swich_msg();
				}
			break;
		case KEY_UP:
			if((navigation_ops_index > Begin) && (navigation_ops_index <= Delete))
			{
				navigation_ops_index--;
			}
			else
			{
				navigation_ops_index = Delete;
			}
			gui_swich_msg();
			break;
		case KEY_DOWN:
			if(navigation_ops_index < Delete)
			{
				navigation_ops_index++;
			}
			else
			{
				navigation_ops_index = Begin;
			}
			gui_swich_msg();
			break;
		case KEY_BACK:
				 ENTER_NAVIGATION = 0;    //�������
				 LOADINGFLAG = 0;
                 InitLoadTrack();
                 memset(&TRACE_BACK_DATA, 0, sizeof(TraceBackStr));
				 ScreenState = DISPLAY_SCREEN_CLOUD_NAVIGATION;
				 gui_swich_msg();
			break;
		default:
			break;
	}	

}

static void gui_realtime_battery_trail_paint(uint8_t x, uint8_t percent)
{
	SetWord_t word = {0};
	char str[20] = {0};
	
	if(percent > 100)
	{
		percent = 100;
	}

    am_hal_rtc_time_get(&RTC_time);
    
	//�ٷֱ�
	word.x_axis = x;
	
	memset(str,0,sizeof(str));
	sprintf(str,"%02d:%02d", RTC_time.ui32Hour,RTC_time.ui32Minute);
	
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - strlen(str)*8 - 4 - Img_Battery_26X12.width) / 2;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetString(str,&word);
	
	//����
	word.x_axis += 4;
	word.y_axis += (strlen(str)*8 + 4);
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_Battery_26X12);
	
	//���ʣ�����
	LCD_SetRectangle(word.x_axis + 2, 8, word.y_axis + 2, percent * (Img_Battery_26X12.width - 5) / 100, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
}


//�켣
static void gui_sport_c_1_track(uint16_t angle)
{
	//���ж��Ƿ���ѭ������
	DrawLoadALLTrack();//�鿴�켣
	
	//ָ���ͷ
	//gui_sport_arrows(angle, LCD_AQUA);
}


void gui_trail_distance_paint(uint32_t distance)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
	//���ͼ��
	word.x_axis = 20;
	word.y_axis = 86;  //LCD_CENTER_JUSTIFIED
	LCD_SetPicture(word.x_axis,word.y_axis,LCD_NONE,LCD_NONE,&Img_Distance_16X16);
	
	//�������
	word.y_axis += (Img_Distance_16X16.width + 4);
	word.size = LCD_FONT_16_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_NONE;
	word.kerning = 1;
	
	memset(str,0,sizeof(str));
 #if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%02dkm",distance/1000,distance/10%100);
	#else
    sprintf(str,"%.2fkm",distance / 1000.f);
	#endif
	LCD_SetString(str,&word);
}



void gui_navigation_trail_paint(void)
{
//	SetWord_t word = {0};

	LCD_SetBackgroundColor(LCD_BLACK);
  gui_sport_c_1_track(0);

	gui_trail_distance_paint(ShareTrack[navigation_index].diatance);   
		
	gui_realtime_battery_trail_paint(205, g_bat_evt.level_percent);
}


void gui_navigation_trail_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:  
			break;
		case KEY_UP:
			break;
		case KEY_DOWN:
				 break; 
		case KEY_BACK:
				 ScreenState = DISPLAY_SCREEN_NAVIGATION_OPS;
				 gui_swich_msg();
			break;
		default:
			break;
	}	

}

static void gui_navigation_del_caption_paint(void)
{
  SetWord_t word = {0};
  
  word.x_axis = 48;
  word.y_axis = LCD_CENTER_JUSTIFIED;
  word.size = LCD_FONT_24_SIZE;
  word.forecolor = LCD_BLACK;
  word.bckgrndcolor = LCD_NONE;
  LCD_SetString("ɾ��",&word);
  word.y_axis = 120 + 24 +2;

  LCD_SetString("?",&word);
}

static void gui_navigation_del_options_paint(bool index)
{
  SetWord_t word = {0};
  
  word.x_axis = 133;
  word.y_axis = LCD_CENTER_JUSTIFIED;
  word.size = LCD_FONT_24_SIZE;
  word.forecolor = LCD_WHITE;
  word.bckgrndcolor = LCD_NONE;
  LCD_SetString((char *)DeleteStrs[SetValue.Language][index],&word);

  word.x_axis = 183;
  LCD_SetString((char *)DeleteStrs[SetValue.Language][!index],&word);
}


void gui_navigation_del_paint(void)
{
  SetWord_t word = {0};

    //���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_WHITE);

  gui_navigation_del_caption_paint();
  
  LCD_SetRectangle(120,120,0,LCD_LINE_CNT_MAX,LCD_BLACK,0,0,LCD_FILL_ENABLE);

  gui_navigation_del_options_paint(delete_ops_index);

  word.x_axis = 170;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

  LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_WHEAT, LCD_NONE, &Img_Pointing_Down_12X8);
}

void gui_navigation_del_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK: 
				 if(delete_ops_index == No)
				 {
					 ScreenState = DISPLAY_SCREEN_NAVIGATION_OPS;
				 }
				 else if(delete_ops_index == Yes)
				 {
					 ShareTrackDataValidNum = ShareTrackDataNUMTraverse();
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
					if(ShareTrackDataValidNum > 0)
					{//���ݱ�ע��ȷ�����ĸ�����
						uint32_t m_address = 0;
						uint8_t m_section_num = 0;
						
						m_address = ShareTrack[navigation_index].point_start_address - sizeof(ShareTrackStr);
						if(m_address >= SHARE_TRACK_START_ADDR)
						{
							m_section_num = (m_address - SHARE_TRACK_START_ADDR)/ ONE_SHARED_TRACK_SIZE;
							
							dev_extFlash_enable();
							dev_extFlash_erase(SHARE_TRACK_START_ADDR + ONE_SHARED_TRACK_SIZE * (m_section_num), ONE_SHARED_TRACK_SIZE);
							dev_extFlash_disable();	
							navigation_index = 0;
							ShareTrackDataValidNum = ShareTrackDataNUMTraverse();
							ShareTrackDataTraverse(SHARE_TRACK_START_ADDR, ShareTrack, SHARED_TRACK_NUM_MAX);
							InitLoadTrack();
						}
						ScreenState = DISPLAY_SCREEN_CLOUD_NAVIGATION;
					}
#else
					 if(ShareTrackDataValidNum == SHARED_TRACK_NUM_MAX)
					 {
						//���Խӿڣ���ʱע�͵���
						 ShareTrackDataEraseFlash(SHARE_TRACK_START_ADDR + ONE_SHARED_TRACK_SIZE * navigation_index);
						 ShareTrackDataValidNum--;
					 }
					 else if(ShareTrackDataValidNum == 1)
					 {
						 ShareTrackDataEraseFlash(SHARE_TRACK_START_ADDR);
						 ShareTrackDataEraseFlash(SHARE_TRACK_START_ADDR + ONE_SHARED_TRACK_SIZE);
						 ShareTrackDataValidNum--;
					 }
					 navigation_index = 0;
					 
					 if(ShareTrackDataValidNum < 0)
					 {
					 	ShareTrackDataValidNum = 0;
//				 		SEGGER_RTT_printf(0, "ShareTrackDataValidNum is less then 0 ...\n");
					 }
					 ShareTrackDataTraverse(SHARE_TRACK_START_ADDR, ShareTrack, SHARED_TRACK_NUM_MAX);
           InitLoadTrack();
					 ScreenState = DISPLAY_SCREEN_CLOUD_NAVIGATION;
#endif
					 //
				 }
				 gui_swich_msg();
			break;
		case KEY_UP:
				 if(delete_ops_index == No)
				 {
				   delete_ops_index = Yes;
				 }
				 else
				 {
					 delete_ops_index--;				 
				 }
				 gui_swich_msg();
			break;
		case KEY_DOWN:
				 if(delete_ops_index == Yes)
				 {
				   delete_ops_index = No;
				 }
				 else
				 {
					 delete_ops_index++;				 
				 }
				 gui_swich_msg();
				 break; 
		case KEY_BACK:
				 
			break;
		default:
			break;
	}	
}


static void gui_sports_record_data_paint(uint8_t activity_t)
{
		SetWord_t word = {0};
		char tmp_str[128] = {0};
		char tmp_dis[64] = {0};
    
 //   word.x_axis = 110;
  //  word.y_axis = 30;
	word.x_axis = 120 - 12 - 16 - 13 + 13/2 - img_sports_record[0].height/2;
   word.y_axis = 45;
    word.size = LCD_FONT_24_SIZE;
    word.forecolor = LCD_CYAN;
    word.bckgrndcolor = LCD_NONE;
		
		switch(activity_t)
		{
			case ACT_RUN:
			LCD_SetPicture(word.x_axis, word.y_axis, LCD_PERU, LCD_NONE, &img_sports_record[0]);
			  #if defined (SPRINTF_FLOAT_TO_INT)
			  sprintf(tmp_dis,"%d.%02dkm",SPORT_DATA[0].ActivityDetails.RunningDetail.Distance/100000,SPORT_DATA[0].ActivityDetails.RunningDetail.Distance/1000%100);
			  #else
			  sprintf(tmp_dis, "����:%.2fkm", SPORT_DATA[0].ActivityDetails.RunningDetail.Distance/100000.f);
			  #endif
			break;
			case ACT_WALK:
				//LCD_SetPicture(word.x_axis, word.y_axis, LCD_AUQAMARIN, LCD_NONE, &Img_Sport[4]);
				 #if defined (SPRINTF_FLOAT_TO_INT)
			  sprintf(tmp_dis,"%d.%02dkm",SPORT_DATA[0].ActivityDetails.RunningDetail.Distance/100000,SPORT_DATA[0].ActivityDetails.RunningDetail.Distance/1000%100);
			  #else
			  	sprintf(tmp_dis, "����:%.2fkm", SPORT_DATA[0].ActivityDetails.RunningDetail.Distance/100000.f);
			  #endif
			break;
			case ACT_MARATHON:
				//LCD_SetPicture(word.x_axis, word.y_axis, LCD_AUQAMARIN, LCD_NONE, &Img_Sport[1]);
				 #if defined (SPRINTF_FLOAT_TO_INT)
			  sprintf(tmp_dis,"%d.%02dkm",SPORT_DATA[0].ActivityDetails.RunningDetail.Distance/100000,SPORT_DATA[0].ActivityDetails.RunningDetail.Distance/1000%100);
			  #else
			  	sprintf(tmp_dis, "����:%.2fkm", SPORT_DATA[0].ActivityDetails.RunningDetail.Distance/100000.f);
			  #endif
			break;
			case ACT_SWIMMING:
				LCD_SetPicture(word.x_axis, word.y_axis, LCD_PERU, LCD_NONE, &img_sports_record[3]);
				#if defined (SPRINTF_FLOAT_TO_INT)
			  sprintf(tmp_dis,"%d.%02dkm",SPORT_DATA[0].ActivityDetails.SwimmingDetail.Distance/100,SPORT_DATA[0].ActivityDetails.SwimmingDetail.Distance%100);
			  #else
			  sprintf(tmp_dis, "����:%.2fkm", SPORT_DATA[0].ActivityDetails.SwimmingDetail.Distance/100.f);
			  #endif
			break;
			case ACT_INDOORRUN:
				//LCD_SetPicture(word.x_axis, word.y_axis, LCD_AUQAMARIN, LCD_NONE, &Img_Sport[3]);
			  //sprintf(tmp_dis, "%.2fkm", SPORT_DATA[0].ActivityDetails.RunningDetail.Distance/100000.f);
			break;
			case ACT_CROSSCOUNTRY:
				LCD_SetPicture(word.x_axis, word.y_axis, LCD_PERU, LCD_NONE, &img_sports_record[5]);
				 #if defined (SPRINTF_FLOAT_TO_INT)
			  sprintf(tmp_dis,"%d.%02dkm",SPORT_DATA[0].ActivityDetails.CrosscountryDetail.Distance/100000,SPORT_DATA[0].ActivityDetails.CrosscountryDetail.Distance/1000%100);
			  #else
			  sprintf(tmp_dis, "����:%.2fkm", SPORT_DATA[0].ActivityDetails.CrosscountryDetail.Distance/100000.f);
			  #endif
			break;
			case ACT_HIKING:
				LCD_SetPicture(word.x_axis, word.y_axis, LCD_PERU, LCD_NONE, &img_sports_record[4]);
				 #if defined (SPRINTF_FLOAT_TO_INT)
			  sprintf(tmp_dis,"%d.%02dkm",SPORT_DATA[0].ActivityDetails.CrosscountryDetail.Distance/100000,SPORT_DATA[0].ActivityDetails.CrosscountryDetail.Distance/1000%100);
			  #else
			  sprintf(tmp_dis, "����:%.2fkm", SPORT_DATA[0].ActivityDetails.CrosscountryDetail.Distance/100000.f);
			  #endif
			break;
			case ACT_CLIMB:
				LCD_SetPicture(word.x_axis, word.y_axis, LCD_PERU, LCD_NONE, &img_sports_record[2]);
				 #if defined (SPRINTF_FLOAT_TO_INT)
			  sprintf(tmp_dis,"%d.%02dkm",SPORT_DATA[0].ActivityDetails.ClimbingDetail.Distance/100000,SPORT_DATA[0].ActivityDetails.ClimbingDetail.Distance/1000%100);
			  #else
			  sprintf(tmp_dis, "����:%.2fkm", SPORT_DATA[0].ActivityDetails.ClimbingDetail.Distance/100000.f);
			  #endif
			break;
			case ACT_CYCLING:
				LCD_SetPicture(word.x_axis, word.y_axis, LCD_PERU, LCD_NONE, &img_sports_record[1]);
				 #if defined (SPRINTF_FLOAT_TO_INT)
			  sprintf(tmp_dis,"%d.%02dkm",SPORT_DATA[0].ActivityDetails.CyclingDetail.Distance/100000,SPORT_DATA[0].ActivityDetails.CyclingDetail.Distance/1000%100);
			  #else
			  sprintf(tmp_dis, "����:%.2fkm", SPORT_DATA[0].ActivityDetails.CyclingDetail.Distance/100000.f);
			  #endif
			break;
			
			default:
			break;
		}
		
		//Date����
		//word.x_axis = 98;
		// word.y_axis = 70;
		word.x_axis = 120 - 12 - 16 - 13;
	    word.y_axis = 45+ img_sports_record[0].width + 8;
	    word.size = LCD_FONT_13_SIZE;
	    word.forecolor = LCD_CYAN;
	    word.bckgrndcolor = LCD_BLACK;
	    word.kerning = 0;	
		memset(tmp_str, 0, sizeof(tmp_str) / sizeof(tmp_str[0]));
		sprintf(tmp_str, "%02d-%02d-%02d %02d:%02d", SPORT_DATA[0].Act_Start_Time.Year,
			SPORT_DATA[0].Act_Start_Time.Month, SPORT_DATA[0].Act_Start_Time.Day,
			SPORT_DATA[0].Act_Start_Time.Hour, SPORT_DATA[0].Act_Start_Time.Minute);
		LCD_SetNumber(tmp_str, &word);


      //���� 
		 word.x_axis = 120 - 12;
         word.y_axis = 45;
         word.size = LCD_FONT_24_SIZE;
         word.forecolor = LCD_CYAN;
          word.bckgrndcolor = LCD_BLACK;
		LCD_SetString(tmp_dis, &word);
		
		//ʱ���� Act_Stop_Time - Act_Start_Time
		//word.x_axis += 132;
		
		word.x_axis = 120 + 12 + 16;
       word.y_axis = 45;
      word.size = LCD_FONT_24_SIZE;
       word.forecolor = LCD_CYAN;
      word.bckgrndcolor = LCD_BLACK;
		memset(tmp_str, 0, sizeof(tmp_str) / sizeof(tmp_str[0]));
		sprintf(tmp_str, "ʱ��:%02d:%02d:%02d", (SPORT_DATA[0].ActTime / 60 / 60),
		(SPORT_DATA[0].ActTime / 60 % 60),
		(SPORT_DATA[0].ActTime % 60));
		LCD_SetString(tmp_str, &word);
		
		
	//	LCD_SetRectangle(word.x_axis + 24 + 8 -1,2,LCD_LINE_CNT_MAX/2 - 60,120,LCD_WHITE,0,0,LCD_FILL_ENABLE);

}

																	
static void gui_sports_data_detail_options_paint(void)
{
	char tmp_str[128] = {0};
	SetWord_t word = {0};
	
	if(SPORTS_DATA_TOTAL_NUM==0)  //���˶�����
	{   
	
	//LCD_SetBackgroundColor(LCD_BLACK);
	//word.x_axis = 24;
	//word.y_axis = LCD_CENTER_JUSTIFIED;
	//word.size = LCD_FONT_16_SIZE;
	//word.forecolor = LCD_WHITE;
	//word.bckgrndcolor = LCD_NONE;
	//LCD_SetString("�˶��켣",&word);

    
	word.x_axis = 120 - 24/2;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_24_SIZE;
    word.forecolor = LCD_WHITE;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString("���˶�����",&word);
	}
	else if( SPORTS_DATA_TOTAL_NUM==1)  //ֻ��һ���˶�����
	{
		gui_sports_record_data_paint(SPORT_DATA[0].Activity_Type);
	}
	else													//���������˶�����
	{
		gui_sports_record_data_paint(SPORT_DATA[0].Activity_Type);
		
		if(SPORTS_DATA_TOTAL_NUM != 0 && sport_data_index != SPORTS_DATA_TOTAL_NUM - 1)
		{
			
			word.x_axis = 180 + 8 +8;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_LIGHTGRAY;
			word.bckgrndcolor = LCD_BLACK;
			word.kerning = 1;	
			memset(tmp_str, 0, sizeof(tmp_str) / sizeof(tmp_str[0]));
			sprintf(tmp_str, "%02d-%02d-%02d", SPORT_DATA[1].Act_Start_Time.Year,
			SPORT_DATA[1].Act_Start_Time.Month, SPORT_DATA[1].Act_Start_Time.Day);
			LCD_SetString(tmp_str, &word);
		}
	}	
}

uint32_t get_motion_trail_startaddress(void)
{
    return SPORT_DATA[0].GPS_Start_Address;
}

uint32_t get_motion_trail_endaddress(void)
{
    return SPORT_DATA[0].GPS_Stop_Address;
}

void gui_track__trail_load_paint(void)
{
    SetWord_t word = {0};
     
    LCD_SetBackgroundColor(LCD_BLACK);

    word.x_axis = 112;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_16_SIZE;
    word.forecolor = LCD_WHITE;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString("������",&word);

}


void gui_track_load_btn_evt(uint32_t Key_Value)
{
    switch( Key_Value )
		{
				case KEY_BACK:
						if (gps_detail_index == MotionTrail)
						{
								ENTER_MOTION_TRAIL = 0;    //�������
								LOADINGFLAG = 0;           //�����б�־����
								InitLoadTrack();
								ScreenState = DISPLAY_SCREEN_MOTION_TRAIL;
								gui_swich_msg(); 

						}

					/*	else if(gps_detail_index == Navigation)
						{
								ENTER_NAVIGATION = 0;    //��־��λ
								LOADINGFLAG = 0;           //�����б�־����
								InitLoadTrack();
								ScreenState = DISPLAY_SCREEN_CLOUD_NAVIGATION;
								gui_swich_msg(); 
						}*/

						break;
				default:
						break;
		}   
   

}


void gui_motion_trail_btn_detail(void)
{
    if(SPORTS_DATA_TOTAL_NUM != 0)
    {
        ScreenState = DISPLAY_SCREEN_MOTION_TRAIL_DETAIL;      
        gui_swich_msg();
    }   
}

void gui_motion_trail_btn_load(void)
{
    if(SPORTS_DATA_TOTAL_NUM != 0)
    {   
        LOADINGFLAG =1;       //�����б�־
        ScreenState = DISPLAY_SCREEN_TRACK_LOAD;
		//motion_trail_index = StartNavigation;
			motion_trail_index = ViewTrail;
        gui_swich_msg();
        Send_Timer_Cmd(CMD_MOTION_TRAIL_LOAD);
        
    }   
}


void gui_motion_trail_paint(void)
{
	SetWord_t word = {0};
	
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//word.x_axis = 40;
	word.x_axis = 24;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetString("�˶��켣",&word);
	
	//LCD_SetRectangle(24 + 24 + 12 -1,2,LCD_LINE_CNT_MAX/2 - 60,120,LCD_WHITE,0,0,LCD_FILL_ENABLE);
		LCD_SetRectangle(70-8,2,120 - 80,160,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
		LCD_SetRectangle(179+8,2,120 - 80,160,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);

	gui_sports_data_detail_options_paint();


	if(sport_data_index > 0)  //���������ʱ�����ʾ���Ϸ�ͼ��
		//LCD_SetPicture(215, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);//204

	if(SPORTS_DATA_TOTAL_NUM != 0 && sport_data_index < SPORTS_DATA_TOTAL_NUM - 1)   //����1���ʱ�����ʾ���·�ͼ��
		LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
}


void gui_motion_trail_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
            gui_motion_trail_btn_load();
			break;
		case KEY_UP:
				 if(sport_data_index > 0)
				 {
						sport_data_index--;
					  READ_SPORTS_DATA_RESULT = ActivityDataTraverse(READ_SPORTS_DATA_RESULT.address1, SPORT_DATA);

					 gui_swich_msg();
				 }
			break;
		case KEY_DOWN:
					if(SPORTS_DATA_TOTAL_NUM != 0 && sport_data_index < (SPORTS_DATA_TOTAL_NUM - 1))
					{
						sport_data_index++;	
					  READ_SPORTS_DATA_RESULT = ActivityDataReverseTraverse(READ_SPORTS_DATA_RESULT.address0, SPORT_DATA);
						
						gui_swich_msg();
					}
			break; 
		case KEY_BACK:
				 SPORTS_DATA_TOTAL_NUM = 0; //�˳�����
				 sport_data_index = 0;
				 ScreenState = DISPLAY_SCREEN_GPS_DETAIL;
				 gui_swich_msg();
			break;
		default:
			break;
	}	

}

static void gui_motion_trail_detail_options_paint(uint8_t index)
{
		SetWord_t word = {0};
    
    word.x_axis = 110;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_24_SIZE;
    word.forecolor = LCD_BLACK;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString((char *)StartNavigationStrs[SetValue.Language][index],&word);
		
		if(index != ViewTrail)
		{
			word.x_axis = 183; 
			word.size = LCD_FONT_16_SIZE;
			LCD_SetString((char *)StartNavigationStrs[SetValue.Language][index + 1],&word);
		}

}

void gui_motion_trail_detail_paint(void)
{
	SetWord_t word = {0};
	
	LCD_SetBackgroundColor(LCD_BLACK);
	//char tmp_str[128] = {0};
			
		/*if(motion_trail_index == StartNavigation)
		{
			LCD_SetBackgroundColor(LCD_BLACK);
		
			word.x_axis = 40;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_NONE;
			sprintf(tmp_str, "%02d-%02d-%02d %02d:%02d", SPORT_DATA[0].Act_Start_Time.Year,
			SPORT_DATA[0].Act_Start_Time.Month, SPORT_DATA[0].Act_Start_Time.Day,
			SPORT_DATA[0].Act_Start_Time.Hour, SPORT_DATA[0].Act_Start_Time.Minute);
				
			LCD_SetString(tmp_str, &word);
		}
else
	//  {
	  	LCD_SetBackgroundColor(LCD_BLACK);
		
      	word.x_axis = 40;
	  	word.y_axis = LCD_CENTER_JUSTIFIED;
	  	word.size = LCD_FONT_16_SIZE;
	  	word.forecolor = LCD_BLACK;
	  	word.bckgrndcolor = LCD_NONE;
	  	LCD_SetString((char *)StartNavigationStrs[SetValue.Language][motion_trail_index - 1], &word);		
	//  }*/

	
	word.x_axis = 110;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_24_SIZE;
    word.forecolor = LCD_WHITE;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString((char *)StartNavigationStrs[SetValue.Language][motion_trail_index],&word);

	//gui_motion_trail_detail_options_paint(motion_trail_index);


	//LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Up_12X8);

	//LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Down_12X8);
}


void gui_motion_trail_detail_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
				 if(motion_trail_index == StartNavigation)
				 {
					 if(0 == IS_SPORT_CONTINUE_LATER) 
					 {
							ENTER_MOTION_TRAIL = 1;    //�����λ
							switch(SPORT_DATA[0].Activity_Type)
							{
								case ACT_RUN:
									ReadySport(DISPLAY_SCREEN_RUN);  
#if defined WATCH_SIM_SPORT
#elif defined WATCH_COM_SPORT
								 	ScreenState = DISPLAY_SCREEN_RUN;
								 	gui_sport_run_init();

#else

									ScreenState = DISPLAY_SCREEN_RUN_READY;
#endif
									break;
								case ACT_WALK:
									ReadySport(DISPLAY_SCREEN_WALK);	 
									ScreenState = DISPLAY_SCREEN_WALK;
									gui_sport_walk_init();
									break;
								case ACT_MARATHON:
									ReadySport(DISPLAY_SCREEN_MARATHON);   
									ScreenState = DISPLAY_SCREEN_MARATHON;
									gui_sport_marathon_init();
									break;
								case ACT_SWIMMING:
									ReadySport(DISPLAY_SCREEN_SWIMMING); 
#if defined WATCH_SIM_SPORT
#elif  defined WATCH_COM_SPORT
					ScreenState = DISPLAY_SCREEN_SWIMMING;
					gui_sport_swim_init();

#else

									ScreenState = DISPLAY_SCREEN_SWIM_READY;
#endif
									break;
								case ACT_INDOORRUN:
//									 ReadySport();    �����˶���ʱû��ʵ�� 
//									 ScreenState = ;
									break;
								case ACT_CROSSCOUNTRY:
									ReadySport(DISPLAY_SCREEN_CROSSCOUNTRY);
#if defined WATCH_COM_SPORT
#if defined WATCH_SPORT_EVENT_SCHEDULE
								init_sport_event_data_str(ACT_CROSSCOUNTRY,false);
#else
									ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY;
									gui_sport_crosscountry_init();
#endif

									#else
									ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_READY;
									#endif
									break;
								case ACT_HIKING:
								{
									ReadySport(DISPLAY_SCREEN_CROSSCOUNTRY_HIKE);    //ͽ��ԽҰ
									
					#if defined WATCH_SPORT_EVENT_SCHEDULE
									init_sport_event_data_str(ACT_HIKING,false);
					#else
									ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE;
									gui_sport_crosscountry_hike_init();
					#endif

								}
									break;
								case ACT_CLIMB:
									ReadySport(DISPLAY_SCREEN_CLIMBING);
#if defined WATCH_COM_SPORT
									ScreenState = DISPLAY_SCREEN_CLIMBING;
									gui_sport_climbing_init();
#else

									ScreenState = DISPLAY_SCREEN_CLIMBING_READY;
#endif
									break;
								case ACT_CYCLING:
									ReadySport(DISPLAY_SCREEN_CYCLING);    
									ScreenState = DISPLAY_SCREEN_CYCLING;
									gui_sport_cycling_init();
									break;
								
								default:
									//
									break;
							}
								//׼��״̬
							g_sport_status = SPORT_STATUS_READY;
							memset(&TRACE_BACK_DATA, 0, sizeof(TraceBackStr));
							TRACE_BACK_DATA.sport_distance = SPORT_DATA[0].ActivityDetails.CommonDetail.Distance;
                            TRACE_BACK_DATA.sport_traceback_flag = 1;
							msg.cmd = MSG_DISPLAY_SCREEN;
							xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				   }
					 else
					 {
							ScreenState = DISPLAY_SCREEN_GPS_TRAIL_REMIND;
						  SCREEN_STATE_S = DISPLAY_SCREEN_MOTION_TRAIL_DETAIL;
						  gui_swich_msg();
					 }
				 }
				 else if(motion_trail_index == ViewTrail)
				 {
					 ScreenState = DISPLAY_SCREEN_VIEW_MOTION_TRAIL;
				 }

				 gui_swich_msg();
			break;
		case KEY_UP:
				/* if(motion_trail_index > StartNavigation)
				 {
					 motion_trail_index--;				 
				 }
				 else
				 {
					motion_trail_index = ViewTrail;
				 }
				 gui_swich_msg();*/
			break;
		case KEY_DOWN:
			/*	 if(motion_trail_index < ViewTrail)
				 {
					 motion_trail_index++;
				 }
				 else
				 {
					motion_trail_index = StartNavigation;
				 }
				 gui_swich_msg();*/
			break; 
		case KEY_BACK:
				 ENTER_MOTION_TRAIL = 0;    //��־��λ
				 LOADINGFLAG = 0;
                 InitLoadTrack();
                 memset(&TRACE_BACK_DATA, 0, sizeof(TraceBackStr));
				 ScreenState = DISPLAY_SCREEN_MOTION_TRAIL;
				 gui_swich_msg();
			break;
		default:
			break;
	}	
}

static void gui_realtime_battery_paint_gps(uint8_t x, uint8_t percent)
{
	SetWord_t word = {0};
	char str[20] = {0};
	
	if(percent > 100)
	{
		percent = 100;
	}

    am_hal_rtc_time_get(&RTC_time);
    
	//�ٷֱ�
	word.x_axis = x;
	
	memset(str,0,sizeof(str));
	sprintf(str,"%02d:%02d", RTC_time.ui32Hour,RTC_time.ui32Minute);
	
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - strlen(str)*8 - 4 - Img_Battery_26X12.width) / 2;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetString(str,&word);
	
	//����
	word.x_axis += 4;
	word.y_axis += (strlen(str)*8 + 4);
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_Battery_26X12);
	
	//���ʣ�����
	LCD_SetRectangle(word.x_axis + 2, 8, word.y_axis + 2, percent * (Img_Battery_26X12.width - 5) / 100, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
}



static void gui_distance_paint_gps(uint32_t distance)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
	//���ͼ��
	word.x_axis = 40;
	word.y_axis = 60;
	LCD_SetPicture(word.x_axis,word.y_axis,LCD_NONE,LCD_NONE,&Img_Distance_16X16);
	
	//�������
	word.y_axis += (Img_Distance_16X16.width + 4);
	word.size = LCD_FONT_16_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_NONE;
	word.kerning = 1;
	
	memset(str,0,sizeof(str));
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%02dkm",distance/1000,distance/10%100);
	#else
    sprintf(str,"%0.2fkm",distance/100000.f);
	#endif
	LCD_SetString(str,&word);

}

void gui_view_motion_trail_paint(void)
{
//	SetWord_t word = {0};

	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
    gui_sport_c_1_track(0);

	gui_distance_paint_gps(SPORT_DATA[0].ActivityDetails.CommonDetail.Distance);
	
	gui_realtime_battery_paint_gps(205, g_bat_evt.level_percent);
	
	
}

void gui_view_motion_trail_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
			break;
		case KEY_UP:
			break;
		case KEY_DOWN:
			break; 
		case KEY_BACK:
				// ScreenState = DISPLAY_SCREEN_MOTION_TRAIL_DETAIL;

				ScreenState =  DISPLAY_SCREEN_MOTION_TRAIL;
				 gui_swich_msg();
			break;
		default:
			break;
	}	

}


void gui_gps_trail_remind_paint(void)
{
	SetWord_t word = {0};

	LCD_SetBackgroundColor(LCD_BLACK);

	word.x_axis = 100;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;

	LCD_SetString("��ǰ��δ��ɵ��˶�", &word);
	word.x_axis += 20;
	LCD_SetString("������˶����ٽ��뵼��", &word);
}


void gui_gps_trail_remind_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
				 ScreenState = SCREEN_STATE_S;
				 gui_swich_msg();
			break;
		case KEY_UP:
			break;
		case KEY_DOWN:
			break; 
		case KEY_BACK:
				 ScreenState = SCREEN_STATE_S;
				 gui_swich_msg();
			break;
		default:
			break;
	}	
}



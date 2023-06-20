#include "gui_notification.h"
#include "gui_home_config.h"
#include "gui_home.h"
#include "gui_tool.h"
#include "task_ble.h"
#include "task_tool.h"
#include "drv_extFlash.h"
#include "drv_lcd.h"
#include "SEGGER_RTT.h"
#include "img_home.h"
#include "task_display.h"
#include "drv_key.h"
#include <string.h>

#define GUI_NOTIFICATION_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && GUI_NOTIFICATION_LOG_ENABLED == 1
	#define GUI_NOTIFICATION_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_NOTIFICATION_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_NOTIFICATION_WRITESTRING(...)
	#define GUI_NOTIFICATION_PRINTF(...)		        
#endif

extern SetValueStr SetValue;
extern SWITCH_HOME_SCREEN_T switch_home_screen;
extern HOME_OR_MINOR_SCREEN_T home_or_minor_switch;
extern uint16_t s_switch_screen_count;
extern ScreenState_t back_monitor_screen;
#define APP_MSG_DISPLAY_NUM         APP_NOTIFY_MSG_NUM_MAX

static uint32_t m_extflash_address;
static int32_t m_msg_cnt = 0;
static _app_notification_msg m_app_notification_msg;
uint8_t notification_msg_recogn_num[6] = {0};//通知消息类型及其数量:电话，微信，QQ，短信,企业微信，钉钉的数量
_app_notification_msg notification_msg[3]={0};//主页显示3个或更少的消息

 uint8_t fore_color[6] ={LCD_FORESTGREEN,LCD_LIME,LCD_RED,LCD_DARKTURQUOISE,LCD_NAVY,LCD_CORNFLOWERBLUE};

uint16_t gui_notification_unread_msg_get(void)
{
	uint16_t i;
	uint8_t status[1] = {0};
	uint16_t num = 0;
	
	//未读信息数
	for(i = APP_NOTIFY_MSG_NUM_MAX; i > 0; i--)
	{
		dev_extFlash_enable();			
		dev_extFlash_read(NOTIFY_DATA_START_ADDRESS+(i*APP_NOTIFY_MSG_SIZE_MAX), status, sizeof(status));			
		dev_extFlash_disable();			
		if(status[0] == APP_NOTIFY_MSG_STATUS_UNREAD)
		{
			num++;
			if(num >= APP_MSG_DISPLAY_NUM)
			{
				break;
			}
		}
	}
	
	return num;
}


uint8_t gui_notification_unread_msg_recogn(void)
{

	
    uint8_t show_num = 0;
	uint16_t i,j;
	uint16_t num = 0;
	
 for(i = 0; i < 6;i++)
 	{

     	notification_msg_recogn_num[i] = 0;
    }
	 _app_notification_msg app_notification_msg;
	//未读信息数
	for(i = APP_NOTIFY_MSG_NUM_MAX; i > 0; i--)
	{
		dev_extFlash_enable();			
		dev_extFlash_read(NOTIFY_DATA_START_ADDRESS+(i*APP_NOTIFY_MSG_SIZE_MAX), (uint8_t *)&app_notification_msg, sizeof(_app_notification_msg));
		dev_extFlash_disable();			
		if(app_notification_msg.status== APP_NOTIFY_MSG_STATUS_UNREAD)
		{
			num++;
			if(app_notification_msg.type==APP_MSG_TYPE_CALL_IOS)
			 {
              app_notification_msg.type = APP_MSG_TYPE_CALL;
			  }
			else;
			/*各类型的数量*/
			switch(app_notification_msg.type)
				{
                case 1 :
			    case 5 :
                     notification_msg_recogn_num[0]++;
					break;
                case 2 :
                     notification_msg_recogn_num[1]++;
					break;
                case 3 :
                     notification_msg_recogn_num[2]++;
					break;
                case 4 :
                     notification_msg_recogn_num[3]++;
					break;
                case 6 :
                     notification_msg_recogn_num[4]++;
					break;
                case 7 :
                     notification_msg_recogn_num[5]++;
					 break;
			   }
			/*主页需要显示的最多3个*/
		    if(num==1)
		    	{
		    	memcpy(notification_msg,&app_notification_msg,sizeof(_app_notification_msg));
                 // notification_msg[0] = app_notification_msg;
                  show_num++;
			   }
			else
				{
				if(show_num <3)
					{
	                  for(j=0;j<show_num;j++)
	                  	{
	                      if(app_notification_msg.type == notification_msg[j].type)
	                      	{
	                          break;

						    }

					    }
	                 if(j==show_num)//前面无重复的
	                 	{
	                    
						memcpy(notification_msg + show_num,&app_notification_msg,sizeof(_app_notification_msg));
						//notification_msg[show_num] = app_notification_msg;
	                    show_num++;
					   }

					}
				else;
				  

			    }
		}
	}
	
	return show_num;
}

static void gui_notification_info_init(void)		//通知信息初始化
{
	
	GUI_NOTIFICATION_PRINTF("[GUI_NOTIFICATION]:gui_notification_info_init\n");
	
	uint8_t status[1] = {0};
	uint32_t i=0;
//	for(i=0;i<APP_NOTIFY_MSG_NUM_MAX;i++)
	for(i=APP_NOTIFY_MSG_NUM_MAX;i>0;i--)
	{
		dev_extFlash_enable();			
		dev_extFlash_read(NOTIFY_DATA_START_ADDRESS+i*APP_NOTIFY_MSG_SIZE_MAX, status, sizeof(status));			
		dev_extFlash_disable();			
		if(status[0] == APP_NOTIFY_MSG_STATUS_UNREAD)
		{
			break;
		}
	}	
	m_extflash_address = NOTIFY_DATA_START_ADDRESS + i*APP_NOTIFY_MSG_SIZE_MAX;
	GUI_NOTIFICATION_PRINTF("[GUI_NOTIFICATION]:m_extflash_address --> 0x%08X\n",m_extflash_address);

	dev_extFlash_enable();			
	dev_extFlash_read(m_extflash_address, (uint8_t *)&m_app_notification_msg, sizeof(m_app_notification_msg));			
	dev_extFlash_disable();		
}


void gui_notification_paint(void)					//通知信息界面：主界面
{
	uint8_t index = 0,line = 0;
	uint8_t offset = 0,length = 0;
	uint8_t num_display_character = 0;	
    uint8_t i,home_show_num = 0;
	LCD_SetBackgroundColor(LCD_BLACK);

	SetWord_t word = {0};
	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	if(SetValue.Language == L_CHINESE)
	{
		LCD_DisplayGTString("通知",&word);	//更改使用字库字体
	}
	else
	{
		LCD_DisplayGTString("NOTIFICATION",&word);	//更改使用字库字体
	}

	/*初始化显示信息*/

  for(i = 0; i < 4;i++)
   {
    memset(&notification_msg[i],0,sizeof(_app_notification_msg));

   }
   /*初始化消息数量*/

 for(i = 0; i < 6;i++)
 	{

     	notification_msg_recogn_num[i] = 0;
    }
	
	//获取未读信息数
	m_msg_cnt = gui_notification_unread_msg_get();

	word.x_axis = 28;
	word.y_axis = 154;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	char p_str[64];
	memset(p_str,0,sizeof(p_str));
	if (m_msg_cnt != 0)					//判断是否有未读信息 
	{
	//多少种不同类型的消息
	home_show_num = gui_notification_unread_msg_recogn();
    switch(home_show_num)
    	{

           case 1:
                 word.x_axis = 120 - img_notifi_msg[0].height/2;
	             word.y_axis = 26;
				 word.forecolor = LCD_WHITE;
				 word.bckgrndcolor = LCD_NONE;
				 word.size = LCD_FONT_16_SIZE;
	             word.kerning = 1;   
				 if(notification_msg[0].type>5)
				 	{
                     
					 word.forecolor = fore_color[notification_msg[0].type -2];
					 /*图标*/
					 LCD_SetPicture( word.x_axis,word.y_axis,word.forecolor,word.bckgrndcolor,&img_notifi_msg[notification_msg[0].type -2]);
					 
					 /*含义*/
					/* word.forecolor = LCD_WHITE;
					 word.x_axis = 120 - LCD_FONT_16_SIZE/2;
					 word.y_axis += img_notifi_msg[0].width + 8;
					 LCD_SetString((char *)NotifiAppStrs[SetValue.Language][notification_msg[0].type -2],&word);*/
				   }
				 else
				 	{
                     
					 word.forecolor = fore_color[notification_msg[0].type - 1];
					 
					 /*图标*/
					 LCD_SetPicture( word.x_axis,word.y_axis,word.forecolor,word.bckgrndcolor,&img_notifi_msg[notification_msg[0].type - 1]);
					 /*含义*/
					/* word.forecolor = LCD_WHITE;
					 word.x_axis = 120 - LCD_FONT_16_SIZE/2;
					 word.y_axis += img_notifi_msg[0].width + 8;
					 LCD_SetString((char *)NotifiAppStrs[SetValue.Language][notification_msg[0].type - 1],&word);*/
				   }

				   /*时间*/
				/*   word.x_axis = 120 - LCD_FONT_16_SIZE/2;
				   if(notification_msg[0].type==6)//企业微信
				   	{
				     word.y_axis +=4*16+10;
				   	}
				   else
				   	{
                     
				     word.y_axis +=2*16+8;

				    }
 	               if(notification_msg[0].date_time[0] != 0)
	               {
						memset(p_str,0,sizeof(p_str));
						sprintf(p_str,"%c%c:%c%c",notification_msg[0].date_time[9],notification_msg[0].date_time[10],
						notification_msg[0].date_time[11],notification_msg[0].date_time[12]);
						LCD_SetString(p_str,&word);	
	                  }	*/

				   //标题
                 word.x_axis = 120 - 2 - 16;
	             word.y_axis = 26 + img_notifi_msg[0].width + 8;
				 word.forecolor = LCD_WHITE;
				 word.bckgrndcolor = LCD_NONE;
				 word.size = LCD_FONT_16_SIZE;
	             word.kerning = 1;  
                 if(notification_msg[0].title[0] != 0)
		        {
				  while(notification_msg[0].title[index] != 0)					//推送信息数据循环判断逻辑 
					{
					if(notification_msg[0].title[index] & 0x80)				//判断编码是否为中文
						{
							//中文
							index += 2;
							length += 2;
							num_display_character++;
						}
					else
						{
							//英文字符
							index++;
							length++;
							num_display_character++;
						}
					}
					
					if (index<=12)			//当消息提醒主界面标题小于12个字符串时，显示全部标题
						{
							LCD_DisplayGTString((char *)notification_msg[0].title,&word);
							index = 0;
							length = 0;
							num_display_character = 0;
						}
					else 							//当消息提醒主界面标题大于12个字符串时，只显示12个字符
						{	
							memset(p_str,0,sizeof(p_str));
							memcpy(p_str,notification_msg[0].title,12);
							LCD_DisplayGTString(p_str,&word);
							index = 0;
							length = 0;
							num_display_character = 0;
						}
			      }
                  
				   //内容
				   word.x_axis = 120 + 2;
	               word.y_axis = 26 + img_notifi_msg[0].width + 8;
				   word.forecolor = LCD_WHITE;
				   word.bckgrndcolor = LCD_NONE;
				   word.size = LCD_FONT_16_SIZE;
	               word.kerning = 1; 


                if((notification_msg[0].subtitle[0] != 0) && (notification_msg[0].type == APP_MSG_TYPE_CALL || notification_msg[0].type == APP_MSG_TYPE_CALL_IOS))
				{
					
					LCD_DisplayGTString((char *)notification_msg[0].subtitle,&word);
				}	

				
				while(notification_msg[0].message[index] != 0)					//推送信息数据循环判断逻辑 
					{
						if(notification_msg[0].message[index] & 0x80)				//判断编码是否为中文
						{
							//中文
							index += 2;
							length += 2;
							num_display_character++;
						}
						else		
						{
							//英文字符
							index++;
							length++;
							num_display_character++;
						}
						if(((num_display_character%7) == 0) && (line==0))				//最多显示7个
						{
							memset(p_str,0,sizeof(p_str));
							memcpy(p_str,notification_msg[0].message+offset,length);
							offset = index;
							length = 0;
							LCD_DisplayGTString(p_str,&word);
							word.x_axis += 20;			
							line++;
						}
					}
					if(line==0)
						{
	                    memset(p_str,0,sizeof(p_str));
		            	memcpy(p_str,notification_msg[0].message,length);
			            LCD_DisplayGTString(p_str,&word);
					    }
					else;
	
				      //显示消息数量
				      
					word.x_axis = 120 - LCD_FONT_16_SIZE/2;
	                word.y_axis = 200;
				    memset(p_str,0,sizeof(p_str));
				    if(notification_msg[0].type>5)
				        {
						   sprintf(p_str,"+%d",notification_msg_recogn_num[notification_msg[0].type - 2]);
						}
					else
						{
                               
						 sprintf(p_str,"+%d",notification_msg_recogn_num[notification_msg[0].type - 1]);
					    }
					LCD_SetString(p_str,&word);	
	                                               			
		   	break;
 
        
			case 2:
                  /*第一类消息*/
				 word.x_axis = 120 - 28 -  img_notifi_msg[0].height;
                 word.y_axis = 26;
				 word.bckgrndcolor = LCD_NONE;
				 word.size = LCD_FONT_16_SIZE;
	             word.kerning = 1;   
                   if(notification_msg[0].type>5)
				 	{
                     
					 word.forecolor = fore_color[notification_msg[0].type -2];
					 /*图标*/
					 LCD_SetPicture( word.x_axis,word.y_axis,word.forecolor,word.bckgrndcolor,&img_notifi_msg[notification_msg[0].type -2]);
					 
					 /*含义*/
					/* word.forecolor = LCD_WHITE;
					 word.x_axis = 120 - 36 - LCD_FONT_16_SIZE;
					 word.y_axis +=  img_notifi_msg[0].width+ 4;
					 LCD_SetString((char *)NotifiAppStrs[SetValue.Language][notification_msg[0].type -2],&word);*/
				   }
				 else
				 	{
                     
					 word.forecolor = fore_color[notification_msg[0].type - 1];
					 
					 /*图标*/
					 LCD_SetPicture( word.x_axis,word.y_axis,word.forecolor,word.bckgrndcolor,&img_notifi_msg[notification_msg[0].type - 1]);
					 
					 /*含义*/
					/* word.forecolor = LCD_WHITE;
					 word.x_axis = 120 - 36 - LCD_FONT_16_SIZE;
					 word.y_axis +=  img_notifi_msg[0].width+ 4;
					 LCD_SetString((char *)NotifiAppStrs[SetValue.Language][notification_msg[0].type - 1],&word);*/
				   }

				   /*时间*/
				/*   word.x_axis = 120 - 36 - LCD_FONT_16_SIZE;
				   if(notification_msg[0].type==6)
				   	{
				     word.y_axis +=4*16 + 10;
				   	}
				   else
				   	{
                     
				     word.y_axis +=2*16 + 8;

				    }
 	               if(notification_msg[0].date_time[0] != 0)
	               {
						memset(p_str,0,sizeof(p_str));
						sprintf(p_str,"%c%c:%c%c",notification_msg[0].date_time[9],notification_msg[0].date_time[10],
						notification_msg[0].date_time[11],notification_msg[0].date_time[12]);
						LCD_SetString(p_str,&word);	
	                  }	*/
                 	   //标题
                 word.x_axis += img_notifi_msg[0].height/2  - 2 - 16;
	             word.y_axis  = 26 + img_notifi_msg[0].width + 8;
				 word.forecolor = LCD_WHITE;
				 word.bckgrndcolor = LCD_NONE;
				 word.size = LCD_FONT_16_SIZE;
	             word.kerning = 1;  
                 if(notification_msg[0].title[0] != 0)
		        {
				  while(notification_msg[0].title[index] != 0)					//推送信息数据循环判断逻辑 
					{
					if (notification_msg[0].title[index] & 0x80)				//判断编码是否为中文
						{
							//中文
							index += 2;
							length += 2;
							num_display_character++;
						}
					else
						{
							//英文字符
							index++;
							length++;
							num_display_character++;
						}
					}
					
					if (index<=12)			//当消息提醒主界面标题小于12个字符串时，显示全部标题
						{
							LCD_DisplayGTString((char *)notification_msg[0].title,&word);
							index = 0;
							length = 0;
							num_display_character = 0;
						}
					else 							//当消息提醒主界面标题大于12个字符串时，只显示12个字符
						{	
							memset(p_str,0,sizeof(p_str));
							memcpy(p_str,notification_msg[0].title,12);
							LCD_DisplayGTString(p_str,&word);
							index = 0;
							length = 0;
							num_display_character = 0;
						}
			      }
				/* else
                   	{
                     GUI_NOTIFICATION_PRINTF("title_0 is 0\r\n");

				  }*/
				   //内容
				   word.x_axis += 16 + 2 + 2;
	               word.y_axis  = 26 + img_notifi_msg[0].width + 8;
				   word.forecolor = LCD_WHITE;
				   word.bckgrndcolor = LCD_NONE;
				   word.size = LCD_FONT_16_SIZE;
	               word.kerning = 1; 


                if((notification_msg[0].subtitle[0] != 0) && (notification_msg[0].type == APP_MSG_TYPE_CALL || notification_msg[0].type == APP_MSG_TYPE_CALL_IOS))
				{
					
					LCD_DisplayGTString((char *)notification_msg[0].subtitle,&word);
				}	
				/*if(notification_msg[0].message[0] == 0)
					{
                  
                     GUI_NOTIFICATION_PRINTF("message_0 is 0\r\n");

				 
				   }*/
				while(notification_msg[0].message[index] != 0)					//推送信息数据循环判断逻辑 
					{
						if(notification_msg[0].message[index] & 0x80)				//判断编码是否为中文
						{
							//中文
							index += 2;
							length += 2;
							num_display_character++;
						}
						else		
						{
							//英文字符
							index++;
							length++;
							num_display_character++;
						}
						if(((num_display_character%7) == 0) && (line==0))				//最多显示7个
						{
							memset(p_str,0,sizeof(p_str));
							memcpy(p_str,notification_msg[0].message+offset,length);
							offset = index;
							length = 0;
							LCD_DisplayGTString(p_str,&word);
							word.x_axis += 20;			
							line++;
						}
					}
					if(line==0)
						{
	                    memset(p_str,0,sizeof(p_str));
		            	memcpy(p_str,notification_msg[0].message,length);
			            LCD_DisplayGTString(p_str,&word);
					    }
					else;
				      //显示消息数量
				      
					  word.x_axis = 120 - 28 -  img_notifi_msg[0].height + img_notifi_msg[0].height/2 - LCD_FONT_16_SIZE/2 ;
	                    word.y_axis = 200;
						
						memset(p_str,0,sizeof(p_str));
						 if(notification_msg[0].type>=5)
				           	{
						      sprintf(p_str,"+%d",notification_msg_recogn_num[notification_msg[0].type -2]);
						 	}
						 else
						 	{
                               
							   sprintf(p_str,"+%d",notification_msg_recogn_num[notification_msg[0].type - 1]);
						    }
						LCD_SetString(p_str,&word);	



				/*第二类消息*/
				 word.x_axis = 120 + 28 ;
                 word.y_axis = 26;
				 word.bckgrndcolor = LCD_NONE;
	             word.kerning = 1; 

				  if(notification_msg[1].type>5)
				 	{
                     
					 word.forecolor = fore_color[notification_msg[1].type - 2];
					 /*图标*/
					 LCD_SetPicture( word.x_axis,word.y_axis,word.forecolor,word.bckgrndcolor,&img_notifi_msg[notification_msg[1].type - 2]);
					 
					 /*含义*/
					/* word.forecolor = LCD_WHITE;
					 word.x_axis = 120 + 36 +  img_notifi_msg[0].height- LCD_FONT_16_SIZE;
					 word.y_axis +=  img_notifi_msg[0].width+ 4;
					 LCD_SetString((char *)NotifiAppStrs[SetValue.Language][notification_msg[1].type - 2],&word);*/
				   }
				 else
				 	{
                     
					 word.forecolor = fore_color[notification_msg[1].type - 1];
					 
					 /*图标*/
					 LCD_SetPicture( word.x_axis,word.y_axis,word.forecolor,word.bckgrndcolor,&img_notifi_msg[notification_msg[1].type - 1]);
					 
					 /*含义*/
					/* word.forecolor = LCD_WHITE;
					 word.x_axis = 120 + 36 +  img_notifi_msg[0].height- LCD_FONT_16_SIZE;
					 word.y_axis +=  img_notifi_msg[0].width+ 4;
					 LCD_SetString((char *)NotifiAppStrs[SetValue.Language][notification_msg[1].type - 1],&word);*/
				   }

				   /*时间*/
				/*   word.x_axis = 120 + 36 +  img_notifi_msg[0].height- LCD_FONT_16_SIZE;
				   if(notification_msg[1].type==6)
				   	{
				     word.y_axis +=4*16 + 10;
				   	}
				   else
				   	{
                     
				     word.y_axis +=2*16 + 8;

				    }
 	               if(notification_msg[1].date_time[0] != 0)
	               {
						memset(p_str,0,sizeof(p_str));
						sprintf(p_str,"%c%c:%c%c",notification_msg[1].date_time[9],notification_msg[1].date_time[10],
						notification_msg[1].date_time[11],notification_msg[1].date_time[12]);
						LCD_SetString(p_str,&word);	
	                  }	*/
                    	   //标题
                 word.x_axis += img_notifi_msg[0].height/2  - 2 - 16;
	             word.y_axis  = 26 + img_notifi_msg[0].width + 8;
				 word.forecolor = LCD_WHITE;
				 word.bckgrndcolor = LCD_NONE;
				 word.size = LCD_FONT_16_SIZE;
	             word.kerning = 1;  
                 if(notification_msg[1].title[0] != 0)
		        {
				  while(notification_msg[1].title[index] != 0)					//推送信息数据循环判断逻辑 
					{
					if (notification_msg[1].title[index] & 0x80)				//判断编码是否为中文
						{
							//中文
							index += 2;
							length += 2;
							num_display_character++;
						}
					else
						{
							//英文字符
							index++;
							length++;
							num_display_character++;
						}
					}
					
					if (index<=12)			//当消息提醒主界面标题小于12个字符串时，显示全部标题
						{
							LCD_DisplayGTString((char *)notification_msg[1].title,&word);
							index = 0;
							length = 0;
							num_display_character = 0;
						}
					else 							//当消息提醒主界面标题大于12个字符串时，只显示12个字符
						{	
							memset(p_str,0,sizeof(p_str));
							memcpy(p_str,notification_msg[1].title,12);
							LCD_DisplayGTString(p_str,&word);
							index = 0;
							length = 0;
							num_display_character = 0;
						}
			      }
                /*    else
                   	{
                     GUI_NOTIFICATION_PRINTF("title_1 is 0\r\n");

				  }*/
				   //内容
				   word.x_axis += 16 + 2 + 2;
	               word.y_axis  = 26 + img_notifi_msg[0].width + 8;
				   word.forecolor = LCD_WHITE;
				   word.bckgrndcolor = LCD_NONE;
				   word.size = LCD_FONT_16_SIZE;
	               word.kerning = 1; 


                if((notification_msg[1].subtitle[0] != 0) && (notification_msg[1].type == APP_MSG_TYPE_CALL || notification_msg[1].type == APP_MSG_TYPE_CALL_IOS))
				{
					
					LCD_DisplayGTString((char *)notification_msg[1].subtitle,&word);
				}	
				/*	if(notification_msg[1].message[0] == 0)
					{
                  
                     GUI_NOTIFICATION_PRINTF("message_1 is 0\r\n");

				 
				   }*/
				while(notification_msg[1].message[index] != 0)					//推送信息数据循环判断逻辑 
					{
						if(notification_msg[1].message[index] & 0x80)				//判断编码是否为中文
						{
							//中文
							index += 2;
							length += 2;
							num_display_character++;
						}
						else		
						{
							//英文字符
							index++;
							length++;
							num_display_character++;
						}
						if(((num_display_character%7) == 0) && (line==0))				//最多显示7个
						{
							memset(p_str,0,sizeof(p_str));
							memcpy(p_str,notification_msg[1].message+offset,length);
							offset = index;
							length = 0;
							LCD_DisplayGTString(p_str,&word);
							word.x_axis += 20;			
							line++;
						}
					}
					if(line==0)
						{
	                    memset(p_str,0,sizeof(p_str));
		            	memcpy(p_str,notification_msg[1].message,length);
			            LCD_DisplayGTString(p_str,&word);
					    }
					else;
				      //显示消息数量
				      
					  word.x_axis = 120 + 28 + img_notifi_msg[0].height/2 - LCD_FONT_16_SIZE/2;
	                    word.y_axis = 200;
						
						memset(p_str,0,sizeof(p_str));
						 if(notification_msg[1].type>5)
				           	{
						      sprintf(p_str,"+%d",notification_msg_recogn_num[notification_msg[1].type - 2]);
						 	}
						 else
						 	{
                               
							   sprintf(p_str,"+%d",notification_msg_recogn_num[notification_msg[1].type - 1]);
						    }
						LCD_SetString(p_str,&word);	

			break;


	        case 3:
                 /*第一类消息*/
				 word.x_axis = 120 -  img_notifi_msg[0].height/2 - 20 -  img_notifi_msg[0].height;
                 word.y_axis = 26;
				 word.bckgrndcolor = LCD_NONE;
				 word.size = LCD_FONT_16_SIZE;
	             word.kerning = 1;  
                   if(notification_msg[0].type>5)
				 	{
                     
					 word.forecolor = fore_color[notification_msg[0].type -2];
					 /*图标*/
					 LCD_SetPicture( word.x_axis,word.y_axis,word.forecolor,word.bckgrndcolor,&img_notifi_msg[notification_msg[0].type -2]);
					 
					 /*含义*/
				//	 word.forecolor = LCD_WHITE;
				//	 word.x_axis = 120 -  img_notifi_msg[0].height/2 - 24 - LCD_FONT_16_SIZE;
				//	 word.y_axis +=  img_notifi_msg[0].width+4;
				//	 LCD_SetString((char *)NotifiAppStrs[SetValue.Language][notification_msg[0].type -2],&word);
				   }
				 else
				 	{
                     
					 word.forecolor = fore_color[notification_msg[0].type - 1];
					 
					 /*图标*/
					 LCD_SetPicture( word.x_axis,word.y_axis,word.forecolor,word.bckgrndcolor,&img_notifi_msg[notification_msg[0].type - 1]);
					 
					 /*含义*/
				//	 word.forecolor = LCD_WHITE;
				//	 word.x_axis = 120 -  img_notifi_msg[0].height/2 - 24 - LCD_FONT_16_SIZE;
				//	 word.y_axis +=  img_notifi_msg[0].width+4;
				//	 LCD_SetString((char *)NotifiAppStrs[SetValue.Language][notification_msg[0].type - 1],&word);
				   }

				   /*时间*/
				/*   word.x_axis = 120 -  img_notifi_msg[0].height/2 - 24 - LCD_FONT_16_SIZE;
				   if(notification_msg[0].type==6)
				   	{
				     word.y_axis +=4*16+10;
				   	}
				   else
				   	{
                     
				     word.y_axis +=2*16+8;

				    }
 	               if(notification_msg[0].date_time[0] != 0)
	               {
						memset(p_str,0,sizeof(p_str));
						sprintf(p_str,"%c%c:%c%c",notification_msg[0].date_time[9],notification_msg[0].date_time[10],
						notification_msg[0].date_time[11],notification_msg[0].date_time[12]);
						LCD_SetString(p_str,&word);	
	                  }	*/
                       	   //标题
                 word.x_axis += img_notifi_msg[0].height/2  - 2 - 16;
	             word.y_axis  = 26 + img_notifi_msg[0].width + 8;
				 word.forecolor = LCD_WHITE;
				 word.bckgrndcolor = LCD_NONE;
				 word.size = LCD_FONT_16_SIZE;
	             word.kerning = 1;  
                 if(notification_msg[0].title[0] != 0)
		        {
				  while(notification_msg[0].title[index] != 0)					//推送信息数据循环判断逻辑 
					{
					if (notification_msg[0].title[index] & 0x80)				//判断编码是否为中文
						{
							//中文
							index += 2;
							length += 2;
							num_display_character++;
						}
					else
						{
							//英文字符
							index++;
							length++;
							num_display_character++;
						}
					}
					
					if (index<=12)			//当消息提醒主界面标题小于12个字符串时，显示全部标题
						{
							LCD_DisplayGTString((char *)notification_msg[0].title,&word);
							index = 0;
							length = 0;
							num_display_character = 0;
						}
					else 							//当消息提醒主界面标题大于12个字符串时，只显示12个字符
						{	
							memset(p_str,0,sizeof(p_str));
							memcpy(p_str,notification_msg[0].title,12);
							LCD_DisplayGTString(p_str,&word);
							index = 0;
							length = 0;
							num_display_character = 0;
						}
			      }
                   
				   //内容
				   word.x_axis += 16 + 2 + 2;
	               word.y_axis  = 26 + img_notifi_msg[0].width + 8;
				   word.forecolor = LCD_WHITE;
				   word.bckgrndcolor = LCD_NONE;
				   word.size = LCD_FONT_16_SIZE;
	               word.kerning = 1; 


                if((notification_msg[0].subtitle[0] != 0) && (notification_msg[0].type == APP_MSG_TYPE_CALL || notification_msg[0].type == APP_MSG_TYPE_CALL_IOS))
				{
					
					LCD_DisplayGTString((char *)notification_msg[0].subtitle,&word);
				}	
				while(notification_msg[0].message[index] != 0)					//推送信息数据循环判断逻辑 
					{
						if(notification_msg[0].message[index] & 0x80)				//判断编码是否为中文
						{
							//中文
							index += 2;
							length += 2;
							num_display_character++;
						}
						else		
						{
							//英文字符
							index++;
							length++;
							num_display_character++;
						}
						if(((num_display_character%7) == 0) && (line==0))				//最多显示7个
						{
							memset(p_str,0,sizeof(p_str));
							memcpy(p_str,notification_msg[0].message+offset,length);
							offset = index;
							length = 0;
							LCD_DisplayGTString(p_str,&word);
							word.x_axis += 20;			
							line++;
						}
					}
					if(line==0)
						{
	                    memset(p_str,0,sizeof(p_str));
		            	memcpy(p_str,notification_msg[0].message,length);
			            LCD_DisplayGTString(p_str,&word);
					    }
					else;
				      //显示消息数量
				      
					  word.x_axis = 120 -  img_notifi_msg[0].height/2 - 20 -  img_notifi_msg[0].height + img_notifi_msg[0].height/2 - LCD_FONT_16_SIZE/2;
	                    word.y_axis = 200;
						
						memset(p_str,0,sizeof(p_str));
						 if(notification_msg[0].type>5)
				           	{
						      sprintf(p_str,"+%d",notification_msg_recogn_num[notification_msg[0].type - 2]);
						 	}
						 else
						 	{
                               
							   sprintf(p_str,"+%d",notification_msg_recogn_num[notification_msg[0].type - 1]);
						    }
						LCD_SetString(p_str,&word);	
				 
				   /*第二类消息*/
				 word.x_axis = 120 -  img_notifi_msg[0].height/2;
                 word.y_axis = 26;
				 word.bckgrndcolor = LCD_NONE;
				 word.size = LCD_FONT_16_SIZE;
	             word.kerning = 1; 
                  if(notification_msg[1].type>5)
				 	{
                     
					 word.forecolor = fore_color[notification_msg[1].type - 2];
					 /*图标*/
					 LCD_SetPicture( word.x_axis,word.y_axis,word.forecolor,word.bckgrndcolor,&img_notifi_msg[notification_msg[1].type - 2]);
					 
					 /*含义*/
				//	 word.forecolor = LCD_WHITE;
				//	 word.x_axis = 120 - LCD_FONT_16_SIZE/2;
				//	 word.y_axis +=  img_notifi_msg[0].width + 4;
				//	 LCD_SetString((char *)NotifiAppStrs[SetValue.Language][notification_msg[1].type - 2],&word);
				   }
				 else
				 	{
                     
					 word.forecolor = fore_color[notification_msg[1].type - 1];
					 
					 /*图标*/
					 LCD_SetPicture( word.x_axis,word.y_axis,word.forecolor,word.bckgrndcolor,&img_notifi_msg[notification_msg[1].type - 1]);
					 
					 /*含义*/
				//	 word.forecolor = LCD_WHITE;
				//	 word.x_axis = 120 - LCD_FONT_16_SIZE/2;
				//	 word.y_axis +=  img_notifi_msg[0].width + 4;
				//	 LCD_SetString((char *)NotifiAppStrs[SetValue.Language][notification_msg[1].type - 1],&word);
				   }

				   /*时间*/
				/*   word.x_axis = 120 - LCD_FONT_16_SIZE/2;
				   if(notification_msg[1].type==6)
				   	{
				     word.y_axis +=4*16+10;
				   	}
				   else
				   	{
                     
				     word.y_axis +=2*16+8;

				    }
 	               if(notification_msg[1].date_time[0] != 0)
	               {
						memset(p_str,0,sizeof(p_str));
						sprintf(p_str,"%c%c:%c%c",notification_msg[1].date_time[9],notification_msg[1].date_time[10],
						notification_msg[1].date_time[11],notification_msg[1].date_time[12]);
						LCD_SetString(p_str,&word);	
	                  }	*/
                 	   //标题
                 word.x_axis = 120 - 2 - 16;
	             word.y_axis = 26 + img_notifi_msg[0].width + 8;
				 word.forecolor = LCD_WHITE;
				 word.bckgrndcolor = LCD_NONE;
				 word.size = LCD_FONT_16_SIZE;
	             word.kerning = 1;  
                 if(notification_msg[1].title[0] != 0)
		        {
				  while(notification_msg[1].title[index] != 0)					//推送信息数据循环判断逻辑 
					{
					if (notification_msg[1].title[index] & 0x80)				//判断编码是否为中文
						{
							//中文
							index += 2;
							length += 2;
							num_display_character++;
						}
					else
						{
							//英文字符
							index++;
							length++;
							num_display_character++;
						}
					}
					
					if (index<=12)			//当消息提醒主界面标题小于12个字符串时，显示全部标题
						{
							LCD_DisplayGTString((char *)notification_msg[1].title,&word);
							index = 0;
							length = 0;
							num_display_character = 0;
						}
					else 							//当消息提醒主界面标题大于12个字符串时，只显示12个字符
						{	
							memset(p_str,0,sizeof(p_str));
							memcpy(p_str,notification_msg[1].title,12);
							LCD_DisplayGTString(p_str,&word);
							index = 0;
							length = 0;
							num_display_character = 0;
						}
			      }
                   
				   //内容
				   word.x_axis = 120 + 2;
	               word.y_axis = 26 + img_notifi_msg[0].width + 8;
				   word.forecolor = LCD_WHITE;
				   word.bckgrndcolor = LCD_NONE;
				   word.size = LCD_FONT_16_SIZE;
	               word.kerning = 1; 


                if((notification_msg[1].subtitle[0] != 0) && (notification_msg[1].type == APP_MSG_TYPE_CALL || notification_msg[1].type == APP_MSG_TYPE_CALL_IOS))
				{
					
					LCD_DisplayGTString((char *)notification_msg[1].subtitle,&word);
				}	
				while(notification_msg[1].message[index] != 0)					//推送信息数据循环判断逻辑 
					{
						if(notification_msg[1].message[index] & 0x80)				//判断编码是否为中文
						{
							//中文
							index += 2;
							length += 2;
							num_display_character++;
						}
						else		
						{
							//英文字符
							index++;
							length++;
							num_display_character++;
						}
						if(((num_display_character%7) == 0) && (line==0))				//最多显示7个
						{
							memset(p_str,0,sizeof(p_str));
							memcpy(p_str,notification_msg[1].message+offset,length);
							offset = index;
							length = 0;
							LCD_DisplayGTString(p_str,&word);
							word.x_axis += 20;			
							line++;
						}
					}
					if(line==0)
						{
	                    memset(p_str,0,sizeof(p_str));
		            	memcpy(p_str,notification_msg[1].message,length);
			            LCD_DisplayGTString(p_str,&word);
					    }
					else;
				      //显示消息数量
				   
					  word.x_axis = 120 - LCD_FONT_16_SIZE/2;
	                    word.y_axis = 200;
						
						memset(p_str,0,sizeof(p_str));
						 if(notification_msg[1].type>5)
				           	{
						      sprintf(p_str,"+%d",notification_msg_recogn_num[notification_msg[1].type - 2]);
						 	}
						 else
						 	{
                               
							   sprintf(p_str,"+%d",notification_msg_recogn_num[notification_msg[1].type - 1]);
						    }
						LCD_SetString(p_str,&word);	
                  /*第三类消息*/
				 word.x_axis = 120 +  img_notifi_msg[0].height/2 + 20;
                 word.y_axis = 26;
				 word.bckgrndcolor = LCD_NONE;
				 word.size = LCD_FONT_16_SIZE;
	             word.kerning = 1; 

                   if(notification_msg[2].type>5)
				 	{
                     
					 word.forecolor = fore_color[notification_msg[2].type - 2];
					 /*图标*/
					 LCD_SetPicture( word.x_axis,word.y_axis,word.forecolor,word.bckgrndcolor,&img_notifi_msg[notification_msg[2].type - 2]);
					 
					 /*含义*/
					// word.forecolor = LCD_WHITE;
					// word.x_axis = 120 +  img_notifi_msg[0].height/2 + 24 +  img_notifi_msg[0].height- LCD_FONT_16_SIZE;
					// word.y_axis +=  img_notifi_msg[0].width+4;
					// LCD_SetString((char *)NotifiAppStrs[SetValue.Language][notification_msg[2].type -2],&word);
				   }
				 else
				 	{
                     
					 word.forecolor = fore_color[notification_msg[2].type - 1];
					 
					 /*图标*/
					 LCD_SetPicture( word.x_axis,word.y_axis,word.forecolor,word.bckgrndcolor,&img_notifi_msg[notification_msg[2].type - 1]);
					 
					 /*含义*/
				//	 word.forecolor = LCD_WHITE;
				//	 word.x_axis = 120 +  img_notifi_msg[0].height/2 + 24 +  img_notifi_msg[0].height- LCD_FONT_16_SIZE;
				//	 word.y_axis +=  img_notifi_msg[0].width + 4;
				//	 LCD_SetString((char *)NotifiAppStrs[SetValue.Language][notification_msg[2].type - 1],&word);
				   }

				   /*时间*/
				 /*  word.x_axis = 120 +  img_notifi_msg[0].height/2 + 24 +  img_notifi_msg[0].height - LCD_FONT_16_SIZE;
				   if(notification_msg[2].type==6)
				   	{
				     word.y_axis +=4*16+10;
				   	}
				   else
				   	{
                     
				     word.y_axis +=2*16+8;

				    }
 	               if(notification_msg[2].date_time[0] != 0)
	               {
						memset(p_str,0,sizeof(p_str));
						sprintf(p_str,"%c%c:%c%c",notification_msg[2].date_time[9],notification_msg[2].date_time[10],
						notification_msg[2].date_time[11],notification_msg[2].date_time[12]);
						LCD_SetString(p_str,&word);	
	                  }	*/
                        	   //标题
                 word.x_axis += img_notifi_msg[0].height/2  - 2 - 16;
	             word.y_axis  = 26 + img_notifi_msg[0].width + 8;
				 word.forecolor = LCD_WHITE;
				 word.bckgrndcolor = LCD_NONE;
				 word.size = LCD_FONT_16_SIZE;
	             word.kerning = 1;  
                 if(notification_msg[2].title[0] != 0)
		        {
				  while(notification_msg[2].title[index] != 0)					//推送信息数据循环判断逻辑 
					{
					if (notification_msg[2].title[index] & 0x80)				//判断编码是否为中文
						{
							//中文
							index += 2;
							length += 2;
							num_display_character++;
						}
					else
						{
							//英文字符
							index++;
							length++;
							num_display_character++;
						}
					}
					
					if (index<=12)			//当消息提醒主界面标题小于12个字符串时，显示全部标题
						{
							LCD_DisplayGTString((char *)notification_msg[2].title,&word);
							index = 0;
							length = 0;
							num_display_character = 0;
						}
					else 							//当消息提醒主界面标题大于12个字符串时，只显示12个字符
						{	
							memset(p_str,0,sizeof(p_str));
							memcpy(p_str,notification_msg[2].title,12);
							LCD_DisplayGTString(p_str,&word);
							index = 0;
							length = 0;
							num_display_character = 0;
						}
			      }
                   
				   //内容
				   word.x_axis += 16 + 2 + 2;
	               word.y_axis  = 26 + img_notifi_msg[0].width + 8;
				   word.forecolor = LCD_WHITE;
				   word.bckgrndcolor = LCD_NONE;
				   word.size = LCD_FONT_16_SIZE;
	               word.kerning = 1; 


                if((notification_msg[2].subtitle[0] != 0) && (notification_msg[2].type == APP_MSG_TYPE_CALL || notification_msg[2].type == APP_MSG_TYPE_CALL_IOS))
				{
					
					LCD_DisplayGTString((char *)notification_msg[2].subtitle,&word);
				}	
				while(notification_msg[2].message[index] != 0)					//推送信息数据循环判断逻辑 
					{
						if(notification_msg[2].message[index] & 0x80)				//判断编码是否为中文
						{
							//中文
							index += 2;
							length += 2;
							num_display_character++;
						}
						else		
						{
							//英文字符
							index++;
							length++;
							num_display_character++;
						}
						if(((num_display_character%7) == 0) && (line==0))				//最多显示7个
						{
							memset(p_str,0,sizeof(p_str));
							memcpy(p_str,notification_msg[2].message+offset,length);
							offset = index;
							length = 0;
							LCD_DisplayGTString(p_str,&word);
							word.x_axis += 20;			
							line++;
						}
					}
					if(line==0)
						{
	                    memset(p_str,0,sizeof(p_str));
		            	memcpy(p_str,notification_msg[2].message,length);
			            LCD_DisplayGTString(p_str,&word);
					    }
					else;
				      //显示消息数量
				     
					  word.x_axis = 120 +  img_notifi_msg[0].height/2 + 20 +  img_notifi_msg[0].height/2 - LCD_FONT_16_SIZE/2;
	                    word.y_axis = 200;
						
						memset(p_str,0,sizeof(p_str));
						 if(notification_msg[2].type>=5)
				           	{
						      sprintf(p_str,"+%d",notification_msg_recogn_num[notification_msg[2].type - 2]);
						 	}
						 else
						 	{
                               
							   sprintf(p_str,"+%d",notification_msg_recogn_num[notification_msg[2].type - 1]);
						    }
						LCD_SetString(p_str,&word);	
				
		    break;


	      

          default:
		  	break;
		  	
   }
    
	
	//gui_notification_info_init();
	


 }
 else						//当没有未读消息时
 {
	SetWord_t word = {0};
	word.x_axis = 108;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	if(SetValue.Language == L_CHINESE)
	{
		LCD_DisplayGTString("没有更多消息",&word);	//更改使用字库字体
	}
	else
	{
		LCD_DisplayGTString("NO MESSAGE",&word);	//更改使用字库字体
	}	 
 }
}


void gui_notification_btn_evt(uint32_t Key_Value)
{
	
	//GUI_NOTIFICATION_PRINTF("[GUI_NOTIFICATION]:gui_notification_btn_evt\n");
	
	DISPLAY_MSG  msg = {0};
	
	home_or_minor_switch = MINOR_SCREEN_SWITCH;
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_msg_cnt > 0)
			{
				gui_notification_info_init();
				ScreenState = DISPLAY_SCREEN_APP_MSG_INFO;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
			}

			break;

#if defined WATCH_COM_SHORTCUNT_UP 
		case KEY_UP:
			if(((SetValue.U_CustomMonitor.val & 0x01)== 0) &&(((SetValue.U_CustomMonitor.val>>1) & 0x01)== 0)
				&&(((SetValue.U_CustomMonitor.val>>2) & 0x01)== 0)&&(((SetValue.U_CustomMonitor.val>>3) & 0x01)== 0))
				{
				   
					s_switch_screen_count = 0;
					switch_home_screen = MONITOR_HOME_SCREEN;
					
					back_monitor_screen = DISPLAY_SCREEN_NOTIFICATION;
                    ScreenState = DISPLAY_SCREEN_HOME;
			         //返回待机界面
			         msg.cmd = MSG_DISPLAY_SCREEN;
			         xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			    }
			else
				{
                  
				set_switch_screen_count_evt(Key_Value);
			    }
			break;
		case KEY_DOWN:
               
			if(((SetValue.U_CustomMonitor.val>>5) & 0x01)== 0)
						{
							switch_home_screen = MONITOR_HOME_SCREEN;
							
							back_monitor_screen = DISPLAY_SCREEN_NOTIFICATION;
							ScreenState = DISPLAY_SCREEN_HOME;
							 //返回待机界面
							 msg.cmd = MSG_DISPLAY_SCREEN;
							 xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
						}
					else
						{
						  
						set_switch_screen_count_evt(Key_Value);
						}

			break;
		case KEY_BACK:
			GUI_NOTIFICATION_PRINTF("[GUI_NOTIFICATION]:KEY_BACK\n");
			ScreenState = DISPLAY_SCREEN_HOME;
		
			back_monitor_screen = DISPLAY_SCREEN_NOTIFICATION;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
#else
		case KEY_UP:
			GUI_NOTIFICATION_PRINTF("[GUI_NOTIFICATION]:KEY_UP\n");
			set_home_index_up();
			ScreenState = Get_Curr_Home_Index();
			msg.cmd = MSG_DISPLAY_SCREEN;
//			msg.value =Get_Curr_Home_Index() ;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_DOWN:
			set_home_index_down();
			ScreenState = Get_Curr_Home_Index();
			msg.cmd = MSG_DISPLAY_SCREEN;
//			msg.value =Get_Curr_Home_Index() ;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			GUI_NOTIFICATION_PRINTF("[GUI_NOTIFICATION]:KEY_BACK\n");
			
			switch_home_screen = BACK_HOME_SCREEN;
			ScreenState = DISPLAY_SCREEN_HOME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
#endif
		default:
			break;
	}
}
/*

电话:只有标题或内容，标题是联系人名称，内容是电话号码
短信:标题+内容
QQ:只有内容
微信:标题+内容，标题是固定的微信



*/


void gui_notification_info_paint(void)				//通知信息界面：信息详情界面
{
		
	LCD_SetBackgroundColor(LCD_BLACK);
	uint8_t index = 0,line = 0;
	uint8_t offset = 0,length = 0;
	uint8_t num_display_character = 0;
	char p_str[64];

	SetWord_t word = {0};
	word.x_axis = 16;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	
	uint8_t type = (m_app_notification_msg.type==APP_MSG_TYPE_CALL_IOS)?APP_MSG_TYPE_CALL:m_app_notification_msg.type;

  if(type>5)
  	{
     
	 word.forecolor = fore_color[type -2];
	 LCD_SetPicture(16, LCD_CENTER_JUSTIFIED,  word.forecolor, LCD_NONE, &img_notifi_msg[type - 2]);
    }
  else
  	{
     
	 word.forecolor = fore_color[type - 1];
	 LCD_SetPicture(16, LCD_CENTER_JUSTIFIED,  word.forecolor, LCD_NONE, &img_notifi_msg[type - 1]);

    }

   word.forecolor = LCD_WHITE;

   word.x_axis = 16 + img_notifi_msg[0].height;
	
	
	word.x_axis += 8;
/*	if(m_app_notification_msg.date_time[0] != 0)					//显示信息发送的时间
	{
		memset(p_str,0,sizeof(p_str));
		sprintf(p_str,"%c%c%c%c.%c%c.%c%c %c%c:%c%c:%c%c",
		m_app_notification_msg.date_time[0],m_app_notification_msg.date_time[1],
		m_app_notification_msg.date_time[2],m_app_notification_msg.date_time[3],
		m_app_notification_msg.date_time[4],m_app_notification_msg.date_time[5],
		m_app_notification_msg.date_time[6],m_app_notification_msg.date_time[7],
		m_app_notification_msg.date_time[9],m_app_notification_msg.date_time[10],
		m_app_notification_msg.date_time[11],m_app_notification_msg.date_time[12],
		m_app_notification_msg.date_time[13],m_app_notification_msg.date_time[14]);
		LCD_SetString(p_str,&word);	
		word.x_axis += 26;
	}	*/
	if(m_app_notification_msg.title[0] != 0)
	{
			while(m_app_notification_msg.title[index] != 0)					//推送信息标题数据循环判断逻辑 
				{
				if (m_app_notification_msg.title[index] & 0x80)				//判断编码是否为中文
					{
						//中文
						index += 2;
						length += 2;
						num_display_character++;
					}
				else
					{
						//英文字符
						index++;
						length++;
						num_display_character++;
					}
				}

				if (index<=16)
					{
						LCD_DisplayGTString((char *)m_app_notification_msg.title,&word);
						index = 0;
						length = 0;
						num_display_character = 0;
					}
				else 
					{
						memset(p_str,0,sizeof(p_str));
						memcpy(p_str,m_app_notification_msg.title,12);
						LCD_DisplayGTString(p_str,&word);
						index = 0;
						length = 0;
						num_display_character = 0;
					}
		word.x_axis += 34;
	}	




	if(m_app_notification_msg.subtitle[0] != 0 && (m_app_notification_msg.type == APP_MSG_TYPE_CALL || m_app_notification_msg.type == APP_MSG_TYPE_CALL_IOS))
	{
		LCD_DisplayGTString((char *)m_app_notification_msg.subtitle,&word);
		word.x_axis += 34;
	}	

	while(m_app_notification_msg.message[index] != 0)						//推送信息内容数据循环判断逻辑 
	{
		if(m_app_notification_msg.message[index] & 0x80)					//判断编码是否为中文
		{
			//中文
			index += 2;
			length += 2;
			num_display_character++;
		}
		else
		{
			//英文字符
			index++;
			length++;
			num_display_character++;
		}
		if(m_app_notification_msg.type==APP_MSG_TYPE_CALL)
			{
              if((num_display_character%13 == 0 ) && (line < 2))
              	{
              	memset(p_str,0,sizeof(p_str));
				memcpy(p_str,m_app_notification_msg.message+offset,length);
				offset = index;
				length = 0;
				LCD_DisplayGTString(p_str,&word);
				word.x_axis += 28;			
				line++;
              	}
		  }
		else
		  {
			if(((num_display_character%8) == 0) && (line<2))				//以10为个数分行，暂时显示4行48个字符 Version 118
			{
				memset(p_str,0,sizeof(p_str));
				if(line==0)
					{
                    
					memcpy(p_str,m_app_notification_msg.message+offset,length);
					
                    offset = index;
					length = 0;
					LCD_DisplayGTString(p_str,&word);
				   }
				else
					{
                     
					 memcpy(p_str,m_app_notification_msg.message+offset,length - 2);
					 
					 memcpy(p_str+length-2,"...",2);
                     offset = index;
					length = 0;
					LCD_DisplayGTString(p_str,&word);
                   
				   }
			
				word.x_axis += 28;			
				line++;
			}
		   }
	}
	if((length > 0) && (line<2))						//显示不为设置分行倍数的剩余信息，暂时显示4行40个字符 Version 118
	{
		memset(p_str,0,sizeof(p_str));
		memcpy(p_str,m_app_notification_msg.message+offset,length);
		LCD_DisplayGTString(p_str,&word);
		word.x_axis += 26;
	}
	
	/*if(m_app_notification_msg.message_size[0] != 0)				//显示消息总长度
	{
		//memset(p_str,0,sizeof(p_str));
		//sprintf(p_str,"(%s)",m_app_notification_msg.message_size);
		//LCD_SetString(p_str,&word);	
		//word.x_axis += 20;
	}		*/
	
	if((m_msg_cnt - 1) > 0)
	{
	   
	   word.x_axis = 200;
	   word.size = LCD_FONT_16_SIZE;
       LCD_SetRectangle(word.x_axis - 3,word.size + 6,0,240,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	    word.forecolor = LCD_BLACK;
	    word.bckgrndcolor = LCD_NONE;
		LCD_SetString("下一条",&word);
		LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_WHITE, LCD_BLACK, &Img_Pointing_Down_12X8);
	}
	
}


void gui_notification_info_btn_evt(uint32_t Key_Value)
{
	
	GUI_NOTIFICATION_PRINTF("[GUI_NOTIFICATION]:gui_notification_info_btn_evt\n");
	
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{



		case KEY_BACK:{
			if(m_msg_cnt >= 0)
			{
				uint8_t status[1] = {APP_NOTIFY_MSG_STATUS_READ};
				dev_extFlash_enable();			
				dev_extFlash_write(m_extflash_address, status, sizeof(status));			
				dev_extFlash_disable();				
				m_msg_cnt--;
			}					
			ScreenState = DISPLAY_SCREEN_NOTIFICATION;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}break;		
		case KEY_DOWN:{
			if(m_msg_cnt >= 0)
			{
				uint8_t status[1] = {APP_NOTIFY_MSG_STATUS_READ};
				dev_extFlash_enable();			
				dev_extFlash_write(m_extflash_address, status, sizeof(status));			
				dev_extFlash_disable();				
				m_msg_cnt--;
				if(m_msg_cnt > 0)
				{
					gui_notification_info_init();
					ScreenState = DISPLAY_SCREEN_APP_MSG_INFO;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);					
				}
				else if (m_msg_cnt == 0)
				{
					ScreenStateSave = ScreenState;
					ScreenState = DISPLAY_SCREEN_NOTIFICATION;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
				}
			}
		}break;

		default:
			break;
	}
}

#include "gui_config.h"
#include "gui_clockline.h"

#include "drv_lcd.h"

#include "task_ble.h"

#include "gui_home.h"

#include "com_data.h"

#include "font_config.h"

#define PI 3.1415926f

/*
void gui_watchhand_test(void)
{
	uint16_t i,j;
	
	for(i = 0; i <= 360; )
	{
		for(j = 0; j <= 360; j += 6)
		{
			//背景为黑色
			LCD_SetBackgroundColor(LCD_DARKBLUE);
//			LCD_SetPicture(3, 3, LCD_NONE, LCD_NONE, &Img_Dial_Backgd_234X234);
		
			//时针
//			gui_clockline_1(i, 5, 30, 60);
//			gui_clockline_3(i, 5, 20, 80, LCD_WHITE);
//			gui_clockline_4(i, 60, 30, 18);
			gui_clockline_6(i, 56, 14, LCD_WHITE, LCD_DARKBLUE);
					
			//分针
//			gui_clockline_1(j, 4, 30, 90);
//			gui_clockline_3(j, 5, 20, 110, LCD_WHITE);
//			gui_clockline_4(j, 90, 40, 18);
			gui_clockline_5(j, 76, 14, LCD_WHITE, LCD_DARKBLUE);
			
			//圆点
			LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 4, LCD_WHITE, 0, LCD_FILL_ENABLE);
			LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 7, LCD_GRAY, 3, LCD_FILL_DISABLE);
			
			LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
			
			if((j%72) == 0)
			{
				i += 6;
			}
			
			am_util_delay_ms(300);
		}
	}
	am_util_delay_ms(1000);
}
*/
/*5 30 60*/
void gui_clockline_7(uint16_t angle, uint8_t r, uint8_t R1, uint8_t R2)
	{
		uint16_t i,j;
		uint8_t ang,R,q_buf[10];
		uint16_t p_buf[10];
		uint16_t buf[8],b_buf[6];
		uint16_t x1,y1,x2,y2,x3,y3;
		
		ang = ceil(asin((float)r/R2)*180.f/PI);//计算的角度为弧度制，转换为角度制
		
		i = (angle + 360 - ang)%360;
		get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, R2, &buf[0], &buf[1]);
		i = (angle + ang)%360;
		get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, R2, &buf[2], &buf[3]);
		
		x1 = buf[0];
		y1 = buf[1];
		x2 = buf[2];
		y2 = buf[3];
		
	
		
		i = (angle + 90)%360;
		get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[4], &buf[5]);

       i = (angle + 270)%360;
		get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[6], &buf[7]);

	for(j=0;j<6;j++)
		{
		  b_buf[j] = 0;
	
		 }
    //后段
	ang = ceil(atanf((float)5/20)*180.f/PI);//计算的角度为弧度制，转换为角度制

   	i = (angle  + 180 - ang)%360;
		get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, (uint16_t)(sqrtf(10*10+14*14)), &b_buf[0], &b_buf[1]);

       i = (angle + 180 +  ang )%360;
		get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, (uint16_t)(sqrtf(10*10+14*14)), &b_buf[2], &b_buf[3]);

		i = (angle + 180)%360;
		get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, 18, &b_buf[4], &b_buf[5]);
		   for(j=0;j<10;j++)
     {
       q_buf[j] = 0;

      }
		
	   q_buf[0] = buf[7];
	   q_buf[1] = buf[6];
	   q_buf[2] = buf[5];
	   q_buf[3] = buf[4];
	   q_buf[4] = b_buf[1];
	   q_buf[5] = b_buf[0];
	   q_buf[6] = b_buf[5];
	   q_buf[7] = b_buf[4];
	   q_buf[8] = b_buf[3];
	   q_buf[9] = b_buf[2];

		LCD_SetPentagon(q_buf, LCD_GRAY, 0, LCD_FILL_ENABLE);


//中间段
		
      for(j=0;j<8;j++)
     {
       p_buf[j] = 0;

      }
		
		//白色部分
		p_buf[0] = (buf[0]+buf[6])/2;
		p_buf[1] = (buf[1]+buf[7])/2;
		p_buf[2] = (buf[2]+buf[4])/2;
		p_buf[3] = (buf[3]+buf[5])/2;
		p_buf[4] = buf[4];
		p_buf[5] = buf[5];
		p_buf[6] = buf[6];
		p_buf[7] = buf[7];
		
		LCD_SetPolygon_7(p_buf, LCD_WHITE, 0, LCD_FILL_ENABLE);
			//边
		LCD_SetLine(p_buf[1], p_buf[0], p_buf[7], p_buf[6], LCD_GRAY, 3);
		LCD_SetLine(p_buf[3], p_buf[2], p_buf[5], p_buf[4], LCD_GRAY, 3);

    

		

		
        for(j=0;j<8;j++)
     {
       p_buf[j] = 0;

      }

		
		//灰色部分
        p_buf[0] = buf[0];
		p_buf[1] = buf[1];
		p_buf[2] = buf[2];
		p_buf[3] = buf[3];
        p_buf[4] = (buf[2]+buf[4])/2;
		p_buf[5] = (buf[3]+buf[5])/2;
		p_buf[6] = (buf[0]+buf[6])/2;
		p_buf[7] = (buf[1]+buf[7])/2;	
		
		LCD_SetPolygon_7(p_buf, LCD_GRAY, 0, LCD_FILL_ENABLE);
		
		
	//边
		LCD_SetLine(p_buf[1], p_buf[0], p_buf[7], p_buf[6], LCD_GREEN, 3);
		LCD_SetLine(p_buf[3], p_buf[2], p_buf[5], p_buf[4], LCD_GREEN, 3);

       
	   LCD_SetLine(p_buf[7], p_buf[6], p_buf[5], p_buf[4], LCD_GREEN, 3);



		
	/*	ang = ceil(asin((float)(r - 2)/R1)*180.f/PI);
		
		i = (angle + 360 - ang)%360;
		get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, R1, &buf[0], &buf[1]);
		i = (angle + ang)%360;
		get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, R1, &buf[2], &buf[3]);
		
		i = (angle + 270)%360;
		get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r-2, &buf[6], &buf[7]);
		i = (angle + 90)%360;
		get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r-2, &buf[4], &buf[5]);*/




		
		//白色部分
	/*	p_buf[0] = buf[1];
		p_buf[1] = buf[0];
		p_buf[2] = buf[3];
		p_buf[3] = buf[2];
		p_buf[4] = buf[5];
		p_buf[5] = buf[4];
		p_buf[6] = buf[7];
		p_buf[7] = buf[6];*/
		
		//LCD_SetPolygon(p_buf, LCD_WHITE, 0, LCD_FILL_ENABLE);





		
		//三角部分
		i = angle % 360;
		get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, R2 + 6, &x3, &y3);
		LCD_SetTriangle(y3, x3, y2, x2, y1, x1, LCD_GRAY, 0, LCD_FILL_ENABLE);//三角形
		
		LCD_SetTriangle(y3, x3, y2, x2, y1, x1, LCD_GREEN, 3, LCD_FILL_DISABLE);//三角边
		
		//黑边
	//	LCD_SetLine(y1, x1, y3, x3, LCD_BLACK, 1);
		//LCD_SetLine(y2, x2, y3, x3, LCD_BLACK, 1);
	}

void gui_clockline_1(uint16_t angle, uint8_t r, uint8_t R1, uint8_t R2)
{
	uint16_t i;
	uint8_t ang;
	uint8_t p_buf[8];
	uint16_t buf[8];
	uint16_t x1,y1,x2,y2,x3,y3;
	
	ang = ceil(asin((float)r/R2)*180.f/PI);//计算的角度为弧度制，转换为角度制
	
	i = (angle + 360 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, R2, &buf[0], &buf[1]);
	i = (angle + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, R2, &buf[2], &buf[3]);
	
	x1 = buf[0];
	y1 = buf[1];
	x2 = buf[2];
	y2 = buf[3];
	
	i = (angle + 270)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[6], &buf[7]);
	i = (angle + 90)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[4], &buf[5]);
	
	//灰色部分
	p_buf[0] = buf[1];
	p_buf[1] = buf[0];
	p_buf[2] = buf[3];
	p_buf[3] = buf[2];
	p_buf[4] = buf[5];
	p_buf[5] = buf[4];
	p_buf[6] = buf[7];
	p_buf[7] = buf[6];
	
	LCD_SetPolygon(p_buf, LCD_LIGHTGRAY, 0, LCD_FILL_ENABLE);
	
	//黑边
	LCD_SetLine(buf[1], buf[0], buf[7], buf[6], LCD_BLACK, 1);
	LCD_SetLine(buf[3], buf[2], buf[5], buf[4], LCD_BLACK, 1);
	
	ang = ceil(asin((float)(r - 2)/R1)*180.f/PI);
	
	i = (angle + 360 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, R1, &buf[0], &buf[1]);
	i = (angle + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, R1, &buf[2], &buf[3]);
	
	i = (angle + 270)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r-2, &buf[6], &buf[7]);
	i = (angle + 90)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r-2, &buf[4], &buf[5]);
	
	//白色部分
	p_buf[0] = buf[1];
	p_buf[1] = buf[0];
	p_buf[2] = buf[3];
	p_buf[3] = buf[2];
	p_buf[4] = buf[5];
	p_buf[5] = buf[4];
	p_buf[6] = buf[7];
	p_buf[7] = buf[6];
	
	LCD_SetPolygon(p_buf, LCD_WHITE, 0, LCD_FILL_ENABLE);
	
	//三角部分
	i = angle % 360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, R2 + 8, &x3, &y3);
	LCD_SetTriangle(y3, x3, y2, x2, y1, x1, LCD_LIGHTGRAY, 0, LCD_FILL_ENABLE);
	
	//黑边
	LCD_SetLine(y1, x1, y3, x3, LCD_BLACK, 1);
	LCD_SetLine(y2, x2, y3, x3, LCD_BLACK, 1);
}

void gui_clockline_2(void)
{
	uint16_t i,j;
	
	uint16_t x1,y1,x2,y2,x3,y3;
	
	for(j = 0; j <= 360; j += 5)
	{
		LCD_SetBackgroundColor(LCD_WHITE);
		
		//圆环
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 8, 0, j, LCD_RED);
		
		//三角形
		i = j % 360;
		get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, 100, &x1, &y1);
		
		i = i + 10;
		i = i % 360;
		get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, 80, &x2, &y2);
		
		i = i + 360 - 20;
		i = i % 360;
		get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, 80, &x3, &y3);
		LCD_SetTriangle(y3, x3, y2, x2, LCD_CENTER_LINE, LCD_CENTER_ROW, LCD_GRAY, 0, LCD_FILL_ENABLE);
		LCD_SetTriangle(y1, x1, y2, x2, y3, x3, LCD_BLACK, 0, LCD_FILL_ENABLE);
		
		//圆点
		LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 8, LCD_RED, 0, LCD_FILL_ENABLE);
		
		LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
		am_util_delay_ms(300);
	}
	
	am_util_delay_ms(1000);
}

void gui_clockline_3(uint16_t angle, uint16_t width, uint16_t H1, uint16_t H2, uint8_t color)
{
	uint8_t p_buf[8];
	uint16_t buf[8];
	uint16_t i,ang;
	uint16_t r1,r2;
		
	ang = ceil(atanf((float)width/H2)*180.f/PI);
	r2 = sqrtf(width*width + H2*H2);
	
	i = (angle + 360 - ang + 2)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r2, &buf[0], &buf[1]);
	
	i = (angle + ang - 1)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r2, &buf[2], &buf[3]);
	
	ang = ceil(atanf((float)width/H1)*180.f/PI);
	r1 = sqrtf(width*width + H1*H1);
	
	i = (angle + 180 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r1, &buf[4], &buf[5]);
	
	i = (angle + 180 + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r1, &buf[6], &buf[7]);
	
	p_buf[0] = buf[1];
	p_buf[1] = buf[0];
	p_buf[2] = buf[3];
	p_buf[3] = buf[2];
	p_buf[4] = buf[5];
	p_buf[5] = buf[4];
	p_buf[6] = buf[7];
	p_buf[7] = buf[6];
	
	LCD_SetPolygon(p_buf, color, 0, LCD_FILL_ENABLE);
	LCD_SetPolygon(p_buf, LCD_BLACK, 1, LCD_FILL_DISABLE);
}

//箭头+前半段（填充颜色）+中间段（填充颜色）+后半段
//60, 30, 18

void gui_clockline_4(uint16_t angle, uint16_t H1, uint16_t H2, uint16_t H3, uint8_t color)
{	
	uint8_t p_buf[10];
	uint16_t buf[10];
	uint16_t i,ang;
	uint16_t r;
	
	uint8_t h1 = 6;		//箭头长度
	uint8_t h2 = H1;	//前半段+中间段长度
	uint8_t h3 = H2;	//中间段长度
	uint8_t h4 = H3;	//后半段长度
	uint8_t width = 6;	//宽度
	
	//前半段
	//1
	ang = ceil(atanf((float)width/h2)*180.f/PI);
	r = sqrtf(width*width + h2*h2);
	i = (angle + 360 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[0], &buf[1]);
	
	//3
	i = (angle + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[4], &buf[5]);
	
	//2
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, angle, h1 + h2, &buf[2], &buf[3]);
	
	//4
	ang = ceil(atanf((float)width/h3)*180.f/PI);
	r = sqrtf(width*width + h3*h3);
	i = (angle + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[6], &buf[7]);
	
	//5
	i = (angle + 360 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[8], &buf[9]);
	
	p_buf[0] = buf[1];
	p_buf[1] = buf[0];
	p_buf[2] = buf[3];
	p_buf[3] = buf[2];
	p_buf[4] = buf[5];
	p_buf[5] = buf[4];
	p_buf[6] = buf[7];
	p_buf[7] = buf[6];
	p_buf[8] = buf[9];
	p_buf[9] = buf[8];
	LCD_SetPentagon(p_buf, color, 0, LCD_FILL_ENABLE);
	LCD_SetPentagon(p_buf, LCD_BLACK, 1, LCD_FILL_DISABLE);
	
	//中间+后半段
	//1
	h3 += 1;
	ang = ceil(atanf((float)width/h3)*180.f/PI);
	r = sqrtf(width*width + h3*h3);
	h3 -= 1;
	i = (angle + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[6], &buf[7]);
	
	//2
	i = (angle + 360 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[8], &buf[9]);
	
	//3
	ang = ceil(atanf((float)width/h4)*180.f/PI);
	r = sqrtf(width*width + h4*h4);
	i = (angle + 180 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[4], &buf[5]);
	
	//5
	i = (angle + 180 + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[0], &buf[1]);
	
	//4
	i = (angle + 180)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, h4 + h1, &buf[2], &buf[3]);
	
	p_buf[0] = buf[9];
	p_buf[1] = buf[8];
	p_buf[2] = buf[7];
	p_buf[3] = buf[6];
	p_buf[4] = buf[5];
	p_buf[5] = buf[4];
	p_buf[6] = buf[3];
	p_buf[7] = buf[2];
	p_buf[8] = buf[1];
	p_buf[9] = buf[0];
	LCD_SetPentagon(p_buf, LCD_GRAY, 0, LCD_FILL_ENABLE);
	LCD_SetLine(buf[7], buf[6], buf[5], buf[4], LCD_BLACK, 1);
	LCD_SetLine(buf[5], buf[4], buf[3], buf[2], LCD_BLACK, 1);
	LCD_SetLine(buf[3], buf[2], buf[1], buf[0], LCD_BLACK, 1);
	LCD_SetLine(buf[1], buf[0], buf[9], buf[8], LCD_BLACK, 1);
	
	//前半段填充
	width = 2;
	h1 = 4;
	h2 -= 2;
	h3 += 3;
	
	//1
	ang = ceil(atanf((float)width/h2)*180.f/PI);
	r = sqrtf(width*width + h2*h2);
	i = (angle + 360 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[0], &buf[1]);
	
	//3
	i = (angle + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[4], &buf[5]);
	
	//2
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, angle, h1 + h2, &buf[2], &buf[3]);
	
	//4
	ang = ceil(atanf((float)width/h3)*180.f/PI);
	r = sqrtf(width*width + h3*h3);
	i = (angle + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[6], &buf[7]);
	
	//5
	i = (angle + 360 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[8], &buf[9]);
	
	p_buf[0] = buf[1];
	p_buf[1] = buf[0];
	p_buf[2] = buf[3];
	p_buf[3] = buf[2];
	p_buf[4] = buf[5];
	p_buf[5] = buf[4];
	p_buf[6] = buf[7];
	p_buf[7] = buf[6];
	p_buf[8] = buf[9];
	p_buf[9] = buf[8];
	LCD_SetPentagon(p_buf, LCD_GRAY, 0, LCD_FILL_ENABLE);
	
	//中间段填充
	width = 2;
	h3 = H2 + 1;
	
	//1
	ang = ceil(atanf((float)width/h3)*180.f/PI);
	r = sqrtf(width*width + h3*h3);
	i = (angle + 360 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[0], &buf[1]);
	
	//2
	i = (angle + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[2], &buf[3]);
	
	//3
	i = (angle + 90)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, 2, &buf[4], &buf[5]);
	
	//4
	i = (angle + 270)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, 2, &buf[6], &buf[7]);
	
	p_buf[0] = buf[1];
	p_buf[1] = buf[0];
	p_buf[2] = buf[3];
	p_buf[3] = buf[2];
	p_buf[4] = buf[5];
	p_buf[5] = buf[4];
	p_buf[6] = buf[7];
	p_buf[7] = buf[6];
	LCD_SetPolygon(p_buf, LCD_WHITE, 0, LCD_FILL_ENABLE);
}

//箭头（填充颜色）+前半段（填充颜色）+后半段
void gui_clockline_5(uint16_t angle, uint16_t H1, uint16_t H2, uint8_t color1, uint8_t color2)
{
	uint8_t p_buf[10];
	uint16_t buf[10];
	uint16_t i,ang;
	uint16_t r;
	
	uint8_t h1 = 14;	//箭头长度
	uint8_t h2 = H1;	//前半段段长度
	uint8_t h3 = H2;	//后半段段长度
	uint8_t width1 = 8;	//宽度
	uint8_t width2 = 6;	//宽度
	uint8_t color = LCD_GRAY;	//边界颜色
	
	//箭头+前半段
	//1
	ang = ceil(atanf((float)width1/(h2+h3))*180.f/PI);
	r = sqrtf(width1*width1 + (h2+h3)*(h2+h3));
	i = (angle + 360 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[0], &buf[1]);
	
	//3
	i = (angle + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[4], &buf[5]);
	
	//2
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, angle, h1 + h2 + h3, &buf[2], &buf[3]);
	
	//4
	ang = ceil(atanf((float)width2/h3)*180.f/PI);
	r = sqrtf(width2*width2 + h3*h3);
	i = (angle + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[6], &buf[7]);
	
	//5
	i = (angle + 360 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[8], &buf[9]);
	
	p_buf[0] = buf[1];
	p_buf[1] = buf[0];
	p_buf[2] = buf[3];
	p_buf[3] = buf[2];
	p_buf[4] = buf[5];
	p_buf[5] = buf[4];
	p_buf[6] = buf[7];
	p_buf[7] = buf[6];
	p_buf[8] = buf[9];
	p_buf[9] = buf[8];
	LCD_SetPentagon(p_buf, color1, 0, LCD_FILL_ENABLE);
	LCD_SetPentagon(p_buf, color, 1, LCD_FILL_DISABLE);
	
	//后半段
	width1 = 2;
	
	//1
	ang = ceil(atanf((float)width1/(h3+1))*180.f/PI);
	r = sqrtf(width1*width1 + (h3+1)*(h3+1));
	i = (angle + 360 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[0], &buf[1]);
	
	//2
	i = (angle + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[2], &buf[3]);
	
	//3
	i = (angle + 90)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, 2, &buf[4], &buf[5]);
	
	//4
	i = (angle + 270)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, 2, &buf[6], &buf[7]);
	
	p_buf[0] = buf[1];
	p_buf[1] = buf[0];
	p_buf[2] = buf[3];
	p_buf[3] = buf[2];
	p_buf[4] = buf[5];
	p_buf[5] = buf[4];
	p_buf[6] = buf[7];
	p_buf[7] = buf[6];
	LCD_SetPolygon(p_buf, color1, 0, LCD_FILL_ENABLE);
//	LCD_SetLine(buf[3], buf[2], buf[5], buf[4], color, 1);
//	LCD_SetLine(buf[1], buf[0], buf[7], buf[6], color, 1);
	
	//箭头填充颜色
	width1 = 6;
	
	//1
	ang = ceil(atanf((float)width1/(h2+h3))*180.f/PI);
	r = sqrtf(width1*width1 + (h2+h3)*(h2+h3));
	i = (angle + 360 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[0], &buf[1]);
	
	//3
	i = (angle + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[4], &buf[5]);
	
	//2
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, angle, h1 + h2 + h3 - 4, &buf[2], &buf[3]);
	LCD_SetTriangle(buf[1], buf[0], buf[3], buf[2], buf[5], buf[4], color, 1, LCD_FILL_DISABLE);
	
	//前半段填充颜色
	width1 = 6;
	width2 = 3;
	
	//1
	ang = ceil(atanf((float)width1/(h2+h3-3))*180.f/PI);
	r = sqrtf(width1*width1 + (h2+h3-3)*(h2+h3-3));
	i = (angle + 360 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[0], &buf[1]);
	
	//2
	i = (angle + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[2], &buf[3]);
	
	//4
	ang = ceil(atanf((float)width2/(h3+4))*180.f/PI);
	r = sqrtf(width2*width2 + (h3+4)*(h3+4));
	i = (angle + 360 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[6], &buf[7]);
	
	//3
	i = (angle + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[4], &buf[5]);
	
	p_buf[0] = buf[1];
	p_buf[1] = buf[0];
	p_buf[2] = buf[3];
	p_buf[3] = buf[2];
	p_buf[4] = buf[5];
	p_buf[5] = buf[4];
	p_buf[6] = buf[7];
	p_buf[7] = buf[6];
	LCD_SetPolygon(p_buf, color2, 0, LCD_FILL_ENABLE);
//	LCD_SetPolygon(p_buf, color, 1, LCD_FILL_DISABLE);
}

//箭头（填充颜色）+前半段（填充颜色）+后半段
void gui_clockline_6(uint16_t angle, uint16_t H1, uint16_t H2, uint8_t color1, uint8_t color2)
{
	uint8_t p_buf[10];
	uint16_t buf[10];
	uint16_t i,ang;
	uint16_t r;
	
	uint8_t h1 = 5;		//箭头长度
	uint8_t h2 = H1;	//前半段段长度
	uint8_t h3 = H2;	//后半段段长度
	uint8_t width = 8;	//宽度
	uint8_t color = LCD_GRAY;	//边界颜色
	
	//箭头+前半段
	//1
	ang = ceil(atanf((float)width/(h2+h3))*180.f/PI);
	r = sqrtf(width*width + (h2+h3)*(h2+h3));
	i = (angle + 360 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[0], &buf[1]);
	
	//3
	i = (angle + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[4], &buf[5]);
	
	//2
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, angle, h1 + h2 + h3, &buf[2], &buf[3]);
	
	//4
	ang = ceil(atanf((float)width/h3)*180.f/PI);
	r = sqrtf(width*width + h3*h3);
	i = (angle + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[6], &buf[7]);
	
	//5
	i = (angle + 360 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[8], &buf[9]);
	
	p_buf[0] = buf[1];
	p_buf[1] = buf[0];
	p_buf[2] = buf[3];
	p_buf[3] = buf[2];
	p_buf[4] = buf[5];
	p_buf[5] = buf[4];
	p_buf[6] = buf[7];
	p_buf[7] = buf[6];
	p_buf[8] = buf[9];
	p_buf[9] = buf[8];
	LCD_SetPentagon(p_buf, color1, 0, LCD_FILL_ENABLE);
	LCD_SetPentagon(p_buf, color, 1, LCD_FILL_DISABLE);
	
	//后半段
	width = 2;
	
	//1
	ang = ceil(atanf((float)width/(h3+1))*180.f/PI);
	r = sqrtf(width*width + (h3+1)*(h3+1));
	i = (angle + 360 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[0], &buf[1]);
	
	//2
	i = (angle + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[2], &buf[3]);
	
	//3
	i = (angle + 90)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, 2, &buf[4], &buf[5]);
	
	//4
	i = (angle + 270)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, 2, &buf[6], &buf[7]);
	
	p_buf[0] = buf[1];
	p_buf[1] = buf[0];
	p_buf[2] = buf[3];
	p_buf[3] = buf[2];
	p_buf[4] = buf[5];
	p_buf[5] = buf[4];
	p_buf[6] = buf[7];
	p_buf[7] = buf[6];
	LCD_SetPolygon(p_buf, color1, 0, LCD_FILL_ENABLE);
//	LCD_SetLine(buf[3], buf[2], buf[5], buf[4], color, 1);
//	LCD_SetLine(buf[1], buf[0], buf[7], buf[6], color, 1);
	
	//箭头填充颜色
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, angle, h2 + h3 - 6, &buf[0], &buf[1]);
	LCD_SetArc(buf[1], buf[0], 6, 1, 0, 360, color);
	
	//前半段填充颜色
	width = 4;
	
	//1
	ang = ceil(atanf((float)width/(h2+h3-14))*180.f/PI);
	r = sqrtf(width*width + (h2+h3-14)*(h2+h3-14));
	i = (angle + 360 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[0], &buf[1]);
	
	//2
	i = (angle + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[2], &buf[3]);
	
	//4
	ang = ceil(atanf((float)width/(h3+4))*180.f/PI);
	r = sqrtf(width*width + (h3+4)*(h3+4));
	i = (angle + 360 - ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[6], &buf[7]);
	
	//3
	i = (angle + ang)%360;
	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, i, r, &buf[4], &buf[5]);
	
	p_buf[0] = buf[1];
	p_buf[1] = buf[0];
	p_buf[2] = buf[3];
	p_buf[3] = buf[2];
	p_buf[4] = buf[5];
	p_buf[5] = buf[4];
	p_buf[6] = buf[7];
	p_buf[7] = buf[6];
	LCD_SetPolygon(p_buf, color2, 0, LCD_FILL_ENABLE);
//	LCD_SetPolygon(p_buf, color, 1, LCD_FILL_DISABLE);
}



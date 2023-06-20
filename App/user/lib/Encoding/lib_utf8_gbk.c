#include "lib_utf8_gbk.h"
#include "task_ble.h"
#include "SEGGER_RTT.h"

#include "drv_gt24l24.h"
#include "HFMA2Ylib.h"


#define LIB_UTF8_GBK_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && LIB_UTF8_GBK_LOG_ENABLED == 1
	#define LIB_UTF8_GBK_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define LIB_UTF8_GBK_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define LIB_UTF8_GBK_WRITESTRING(...)
	#define LIB_UTF8_GBK_PRINTF(...)		        
#endif


static uint16_t Utf8TextBytes (const uint8_t *mstr)  
{  
    uint16_t textbytes = 0;  
    if (mstr == 0)  
    {                            
        return 0;               
    }  

    if (((*mstr) & 0x80) == 0)  
    {  
        textbytes = 1;          
    }  
    else if ((((*mstr) & 0xe0) == 0xc0) && (((*(mstr + 1)) & 0xc0) == 0x80))  
    {  
        if (((*mstr) & 0xfe) == 0xc0)
        {  
            textbytes = 0;  
        }  
        else  
        {  
            textbytes = 2;     
        }  
    }  
    else if ((((*mstr) & 0xf0) == 0xe0) && (((*(mstr + 1)) & 0xc0) == 0x80) && (((*(mstr + 2)) & 0xc0) == 0x80))  
    {  
        if ((*mstr == (uint8_t) 0xe0) && (((*(mstr + 1)) & 0xe0) == 0x80)) 
        {  
            textbytes = 0;  
        }  
        else  
        {  
            textbytes = 3;     
        }  
    }  
    else if ((((*mstr) & 0xf8) == 0xf0) && (((*(mstr + 1)) & 0xc0) == 0x80) && (((*(mstr + 2)) & 0xc0) == 0x80)  
             && (((*(mstr + 3)) & 0xc0) == 0x80))  
    {  
        if (((*mstr) == (uint8_t) 0xf0) && (((*(mstr + 1)) & 0xf0) == 0x80)) 
        {  
            textbytes = 0;  
        }  
        else  
        {  
            textbytes = 4;     
        }  
    }  
    else if ((((*mstr) & 0xfc) == 0xf8) && (((*(mstr + 1)) & 0xc0) == 0x80) &&  
             (((*(mstr + 2)) & 0xc0) == 0x80) && (((*(mstr + 3)) & 0xc0) == 0x80) && (((*(mstr + 4)) & 0xc0) == 0x80))  
    {  
        if ((*mstr == (uint8_t) 0xf8) && (((*(mstr + 1)) & 0xf8) == 0x80)) 
        {  
            textbytes = 0;  
        }  
        else  
        {  
            textbytes = 5;     
        }  
    }  
    else if ((((*mstr) & 0xfe) == 0xfc) && (((*(mstr + 1)) & 0xc0) == 0x80) &&  
             (((*(mstr + 2)) & 0xc0) == 0x80) && (((*(mstr + 3)) & 0xc0) == 0x80) && (((*(mstr + 4)) & 0xc0) == 0x80) &&  
             (((*(mstr + 5)) & 0xc0) == 0x80))  
    {  
        if ((*mstr == (uint8_t) 0xfc) && (((*(mstr + 1)) & 0xfc) == 0x80))  
        {  
            textbytes = 0;  
        }  
        else  
        {  
            textbytes = 6;      
        }  
    }  
    else  
    {  
        textbytes = 0;         
    }  
  
    return textbytes;  
}  



static uint16_t FromUTF8ToUnicode (uint8_t *utf8Str, uint16_t utf8StrLen, uint16_t * unStr, uint16_t unMaxLen)
{  
    uint16_t zen_han, len_dest;  
    uint8_t *p_src;  
    uint8_t utf8_len = 0;  
    len_dest = 0;  
    p_src = (uint8_t *) (utf8Str);  
  
 
    if (utf8StrLen <= 0)  
    {  
        return (0);  
    }  
  
 
    for (; (*p_src != 0x00) && (len_dest <= unMaxLen - 1) && (utf8_len < utf8StrLen);)  
    {  
        zen_han = Utf8TextBytes ((const uint8_t *) p_src);  
        if (zen_han == 1)      
        {  
            (*unStr) = *(p_src);  
            unStr++;  
            p_src++;  
            utf8_len++;  
            len_dest += 1;  
        }  
        else if (zen_han == 2)  
        {  
            *unStr = (((((*p_src) >> 2) & (uint8_t) 0x07)) & 0x00ff) << 8;  
            *unStr |= ((((*p_src) << 6) | ((*(p_src + 1)) & (uint8_t) 0x3f))) & 0x00ff;  
            unStr++;  
            p_src += 2;  
            utf8_len += 2;  
            len_dest += 1;  
  
        }  
        else if (zen_han == 3)  
        {  
            *(unStr) = ((((*(p_src)) << 4) | (((*(p_src + 1)) >> 2) & 0x0f)) & 0x00ff) << 8;  
  
            *unStr |= ((((*(p_src + 1)) << 6) | ((*(p_src + 2)) & (uint8_t) 0x3f))) & 0x00ff;  
            unStr++;  
            p_src += 3;  
            utf8_len += 3;  
            len_dest += 1;  
        }  
        else if (zen_han == 4)  
        {  
            *unStr = (*(p_src)) & 0x7;  
            for (int i=1; i<4;i++)  
            {  
                *unStr <<= 6; 
                *unStr = *unStr | ((*(p_src + i)) & 0x3f);
            }  
            unStr++;  
            p_src += 4;  
            utf8_len += 4;  
            len_dest += 1;  
        }  
        else if (zen_han == 5) 
        {  
            *unStr = (*(p_src)) & 0x3;  
            for (int i=1; i<5;i++)  
            {  
                *unStr <<= 6; 
                *unStr = *unStr | ((*(p_src + i)) & 0x3f);
            }  
            unStr++;  
            p_src += 5;  
            utf8_len += 5;  
            len_dest += 1;  
        }  
        else if (zen_han == 6)  
        {  
            *unStr = (*(p_src)) & 0x1;  
            for (int i=1; i<6;i++)  
            {  
                *unStr <<= 6; 
                *unStr = *unStr | ((*(p_src + i)) & 0x3f);
            }  
            unStr++;  
            p_src += 6;  
            utf8_len += 6;  
            len_dest += 1;  
        }  
        else                    
        {  
            p_src++;  
            utf8_len++;  
        }  
    }  
    *(unStr) = 0x0000;  
    return (len_dest);  
  
}  	

void lib_utf8_gbk(uint8_t *utf8_code,uint8_t *gbk_code,uint16_t *gbk_length)
{
	uint16_t unicode_length;
	uint16_t unicode_char[ATTR_MESSAGE_SIZE];
	uint16_t temp;
	uint16_t gbk_index = 0;;
	
#if 0	
	uint8_t *p_utf8_code = utf8_code;
	LIB_UTF8_GBK_PRINTF("[LIB_UTF8_GBK]: ");
	while(*(p_utf8_code++) != 0)
	{
		LIB_UTF8_GBK_PRINTF("%02X,",*p_utf8_code);
	}
	LIB_UTF8_GBK_PRINTF("\n");	
#endif	
	unicode_length = FromUTF8ToUnicode (utf8_code, ATTR_MESSAGE_SIZE, unicode_char, ATTR_MESSAGE_SIZE);
	
	LIB_UTF8_GBK_PRINTF("[LIB_UTF8_GBK]: ");

	for(uint16_t i=0;i<unicode_length;i++)
	{		
		LIB_UTF8_GBK_PRINTF("%04X,",unicode_char[i]);		
		if(unicode_char[i] & 0xff00)
		{

			drv_gt24l24_enable();
			drv_gt24l24_wakeup();			
			temp = U2G(unicode_char[i]);
			drv_gt24l24_sleep();
			drv_gt24l24_disable();
  			
			gbk_code[gbk_index++] = (temp&0xFF00)>>8;
			gbk_code[gbk_index++] = (temp&0xFF);			
		}else{
			gbk_code[gbk_index++] = unicode_char[i];		
		}			
	}		
	LIB_UTF8_GBK_PRINTF("\n");

	*gbk_length = gbk_index;
	
}


//code_len 2µÄ±¶Êý
void lib_utf16_gbk(uint8_t *utf16_code,uint16_t code_len,uint8_t *gbk_code,uint16_t *gbk_length)
{
	uint16_t unicode_char[ATTR_MESSAGE_SIZE];
	uint16_t temp;
	uint16_t gbk_index = 0;
	uint16_t unicode_index = 0;

	memset(unicode_char,0,ATTR_MESSAGE_SIZE);
	
	LIB_UTF8_GBK_PRINTF("[LIB_UTF16_GBK]: ");

	for(uint16_t i=0;i<code_len;i += 2)
	{		
		unicode_char[unicode_index] |= utf16_code[i] ;
		unicode_char[unicode_index] |= (utf16_code[i+1])<< 8;
		//LIB_UTF8_GBK_PRINTF("[%x,%x,%x]",utf16_code[i],utf16_code[i+1],unicode_char[unicode_index]);
		if(unicode_char[unicode_index] & 0xff00)
		{

			drv_gt24l24_enable();
			drv_gt24l24_wakeup();			
			temp = U2G(unicode_char[unicode_index]);
			drv_gt24l24_sleep();
			drv_gt24l24_disable();
  			
			gbk_code[gbk_index++] = (temp&0xFF00)>>8;
			gbk_code[gbk_index++] = (temp&0xFF);
			//LIB_UTF8_GBK_PRINTF("->%x",temp);
		}else{
			if(unicode_char[unicode_index] != 0x0000)
			{
			   gbk_code[gbk_index++] = unicode_char[unicode_index];
			}
			 //  LIB_UTF8_GBK_PRINTF("-%x",unicode_char[unicode_index]);
		}
		unicode_index++;
	}		
	//LIB_UTF8_GBK_PRINTF("gbk_index %d\r\n",gbk_index);

	*gbk_length = gbk_index;
	
}












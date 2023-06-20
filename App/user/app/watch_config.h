#ifndef WATCH_CONFIG_H
#define	WATCH_CONFIG_H


/*功能开关*/
#undef WACTH_THEME_APP_DOWNLOAD			//自定定义表盘功能
#undef WATCH_SPORT_RUNNING_TYPE			//跑步类手表 无越野类运动
#undef WATCH_UNLOCK_BACK_UP				//BACK+UP解锁
#undef WATCH_GPS_STREAM_READ			//gps一次度多个数据
#undef WATCH_STEP_ALGO          		//使用计步算法
#undef WATCH_PRECISION_TIME_CALIBRATION       //精密授时功能开关
#undef WATCH_PRECISION_TIME_JUMP_TEST  //精密授时时间突变问题测试 前提精密授时开关打开
#undef WATCH_SINGLE_BEIDOU           //单北斗功能
#undef WATCH_GPS_SET_TIME_AUTOMATICALLY  //自动授时功能
#undef WATCH_GPS_OR_NO_GPS_RESOURCE_SET_TIME //区分RTC时间来源 如重启授时、蓝牙授时、卫星授时等来源
#undef WATCH_RAISE_BRIGHT_SCREEN_ALGO         	//使用抬手亮屏算法,必须计步才可生效
#undef WATCH_GPS_ONLINE							//gps online星历

#undef WATCH_GPS_SNR_TEST           //测试GPS信噪比

#undef WATCH_COM_SPORT			//铁腕系列运动新UI及交互
#undef WATCH_COM_SHORTCUNT_UP 	//铁腕手表上键快捷功能
#undef WATCH_COM_SETTING			//铁腕手表设置功能
#undef WATCH_NOT_LEAP_SECOND_SET_TIME   //不获取闰秒直接授时 精密授时校准仍需要判断闰秒有效
#undef WATCH_SPORT_RECORD_TEST     //运动记录查询方式更改
#undef WATCH_IMU_CALIBRATION //加速度校准功能编译开关
#undef WATCH_AUTO_BACK_HOME  //无数据刷新页面超过10分钟无任何操作（无按键操作）返回待机界面 运动过程中、定位中、最大摄氧量测试中、乳酸阈测试中、心率测试中、配件搜索中、充电中除外
#undef WATCH_NO_ACCESSORY
#undef STORE_ORG_DATA_TEST_VERSION		//冒烟版本原始gps的存储，用于算法分析
#undef WATCH_NO_FONT_CN_16_24		//不使用代码自带中文16号24号字体
#undef WATCH_SPORT_EVENT_SCHEDULE   //越野跑等赛事赛段功能
#undef WATCH_SPORT_NEW_CLOUD_NAVIGATION  //新运动的云迹导航
#undef HRT_DEBUG_TEST  //心率调试功能
#undef WATCH_SONY_GNSS //sony定位芯片


/*硬件差异*/
#undef WACTH_HEARTRATE_VERSION			//philips心率版本号
#undef WATCH_HAS_NO_BLE					//不包含蓝牙功能
#undef WATCH_I2C_RESET					//心率卡死时重启手表
#undef WATCH_SPORT_NO_COMPASS			//运动中无指北针
#undef WATCH_SPORT_NO_PRESS				//运动中无气压高度
#undef WATCH_TOOL_COUNTDOWN				//倒计时功能(freeone除外)
#undef WATCH_MODEL_PLUS
#undef WATCH_COD_BSP
#undef WATCH_CHIP_TEST //贴片测试

#undef WATCH_HUADA_GNSS //华大定位芯片
#undef WATCH_GPS_HAEDWARE_ANALYSIS_INFOR //GPS硬件定位解析信息

/*新UI和交互*/
#undef WATCH_SIM_SPORT				//手表运动功能
#undef WATCH_SIM_UNLOCK				//解锁界面
#undef WATCH_SIM_LIGHTS_ON			//背光常亮
#undef WATCH_SIM_NUMBER				//手表数据
#undef STORE_ORG_DATA_TEST_VERSION //原始GPS数据,存储空间和数据处理后的GPS空间重叠(需打开WATCH_GPS_HAEDWARE_ANALYSIS_INFOR宏并关闭此GPS保存功能)





#if defined COD
#define	WACTH_THEME_APP_DOWNLOAD		//自定义表盘功能
#define WATCH_STEP_ALGO    				//计步算法
#define WATCH_RAISE_BRIGHT_SCREEN_ALGO  //使用抬手亮屏算法,必须计步才可生效

//#define WACTH_HEARTRATE_VERSION			//读取心率版本

//#define WATCH_GPS_ONLINE				//gps online星历
//#define WATCH_GPS_STREAM_READ			//gps一次度多个数据

#define WATCH_TOOL_COUNTDOWN			//倒计时功能(freeone除外)


#define	WATCH_UNLOCK_BACK_UP			//BACK+UP解锁
#define	WATCH_SIM_UNLOCK			//解锁界面
#define WATCH_SIM_LIGHTS_ON			//背光常亮

#define WATCH_SIM_NUMBER			//手表数据
#define WATCH_COM_SPORT			//运动新UI及交互
#define WATCH_COM_SHORTCUNT_UP 	//上键快捷功能
#define WATCH_COM_SETTING			//设置功能

#define WATCH_AUTO_BACK_HOME  //无数据刷新页面超过10分钟无任何操作（无按键操作）返回待机界面
//#define STORE_ALT_TEST_VERTION

//#define WATCH_MODEL_PLUS				//心率需使用vallencell文件
#define WATCH_NO_FONT_CN_16_24
#define WATCH_COD_BSP

//#define WATCH_LCD_ESD  //静电测试恢复
#define WATXCH_LCD_TRULY
//#define WATCH_LCD_LPM
//#define WATCH_HUADA_GNSS //华大定位模块
#define WATCH_SONY_GNSS //sony定位模块

//#define WATCH_UBLOX_MOUDULE //ubox定位模块
#define WATCH_RTC_RV8803

//存储GPS原始数据
//#define WATCH_GPS_HAEDWARE_ANALYSIS_INFOR
//#define STORE_ORG_DATA_TEST_VERSION

//存储GPS的LOG数据
//#define STORE_GPS_LOG_DATA_TEST

//定位系统选择
#define WATCH_GPS_MODE  1  // 1 (GPS + BDS + QZSS) 2 (GPS + GLONASS + QZSS) 3 (GPS + QZSS + Galileo)

#endif 

#if defined HY100
#define	WACTH_THEME_APP_DOWNLOAD		//自定义表盘功能
#define WATCH_STEP_ALGO    				//计步算法
#define WATCH_RAISE_BRIGHT_SCREEN_ALGO  //使用抬手亮屏算法,必须计步才可生效

//#define WACTH_HEARTRATE_VERSION			//读取心率版本

#define WATCH_GPS_ONLINE				//gps online星历
//#define WATCH_GPS_STREAM_READ			//gps一次度多个数据

#define WATCH_TOOL_COUNTDOWN			//倒计时功能(freeone除外)


#define	WATCH_UNLOCK_BACK_UP			//BACK+UP解锁
#define	WATCH_SIM_UNLOCK			//解锁界面
#define WATCH_SIM_LIGHTS_ON			//背光常亮

#define WATCH_SIM_NUMBER			//手表数据
#define WATCH_COM_SPORT			//运动新UI及交互
#define WATCH_COM_SHORTCUNT_UP 	//上键快捷功能
#define WATCH_COM_SETTING			//设置功能

#define WATCH_AUTO_BACK_HOME  //无数据刷新页面超过10分钟无任何操作（无按键操作）返回待机界面
//#define STORE_ORG_DATA_TEST_VERSION		//冒烟版本原始gps的存储，用于算法分析
//#define STORE_ALT_TEST_VERTION

#define WATCH_NO_FONT_CN_16_24
#define WATCH_COD_BSP

//#define WATCH_LCD_ESD  //静电测试恢复
#endif 

#if defined WATCH_SPORT_EVENT_SCHEDULE //越野跑等赛事赛段功能
#define WATCH_SPORT_NEW_CLOUD_NAVIGATION  //新运动的云迹导航
#endif


#if defined WATCH_SIM_SPORT
#define BLE_PROTOCAL_VERSION_CODE          0x00000103      //,支持运动参数修改，不同项目导致有不同版本号，APP不好通过版本号做兼容，所以增加协议版本号
#else
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
#define BLE_PROTOCAL_VERSION_CODE          0x00000004      //不同项目导致有不同版本号，APP不好通过版本号做兼容，所以增加协议版本号
#else
#define BLE_PROTOCAL_VERSION_CODE          0x00000003      //不同项目导致有不同版本号，APP不好通过版本号做兼容，所以增加协议版本号
#endif
#endif

#if defined WATCH_PRECISION_TIME_CALIBRATION
#define WATCH_GPS_STREAM_READ			//gps一次度多个数据
#define WATCH_GPS_OR_NO_GPS_RESOURCE_SET_TIME //设置时间区分GPS和非GPS设置来源功能
#define WATCH_NOT_LEAP_SECOND_SET_TIME  //不获取闰秒直接授时 精密授时校准仍需要判断闰秒有效
#endif

#define WATCH_GPS_SNR_TEST  //默认开启GPS信噪比测试
#define WATCH_SPORT_RECORD_TEST     //运动记录查询方式更改

#endif

/**
	|------------------------------- Copyright ----------------------------------|
	|                                                                            |
	|                        (C) Copyright 2019,海康平头哥,                       |
	|          1 Xuefu Rd, Huadu Qu, Guangzhou Shi, Guangdong Sheng, China       |
	|                            All Rights Reserved                             |
	|                                                                            |
	|           By(GCU The wold of team | 华南理工大学广州学院机器人野狼队)         |
	|                    https://github.com/GCUWildwolfteam                      |
	|----------------------------------------------------------------------------|
	|--FileName    : chassis.c                                                
	|--Version     : v1.0                                                            
	|--Author      : 海康平头哥                                                       
	|--Date        : 2019-01-19               
	|--Libsupports : 
	|--Description :  3508 顺时针为正                                                     
	|--FunctionList                                                       
	|-------1. ....                                                       
	|          <version>:                                                       
	|     <modify staff>:                                                       
	|             <data>:                                                       
	|      <description>:                                                        
	|-------2. ...                                                       
	|-------------------------------declaration of end---------------------------|
 **/
#include "chassis.h"
/* -------------- 模块标志位宏定义 ----------------- */
#define   CHASSIS_RC_MODE_READ                 (0x80000000U)//遥控模式 
 #define  CHASSIS_ELUDE_MODE_READ              (0x40000000U) //躲避模式     
 #define  CHASSIS_PC_SHOOT_MODE_READ           (0x20000000U) //自瞄打击模式准备就绪
 #define  CHASSIS_CRUISE_MODE_READ             (0x10000000U)//巡航模式模式准备就绪
 #define  DISABLE_CHA_MOD_READ			           (0xF0000000U)//失能就绪标志位
 #define  CHASSIS_RC_MODE_RUNING               (0x08000000U)//遥控模式执行 
 #define  CHASSIS_ELUDE_MODE_RUNING            (0x04000000U) //躲避模式 
 #define  CHASSIS_PC_SHOOT_MODE_RUNING         (0x02000000U) //自瞄打击模式执行   
 #define  CHASSIS_CRUISE_MODE_RUNING           (0x01000000U) //巡航模式执行
#define   DISABLE_CHA_MOD_RUNNING			         (0x0F000000U)//失能就绪标志位
#define   DISABLE_CHASSIS                      (0xFF000000U)//失能底盘
#define   CHASSIS_JUDGE_READ                   (0xF0000000U)//判断决策
// #define  DISABLE_MOD                        (0xF0000000U)//使能运行模块

// #define  RC_MODE_READ               (0x02000000U)//遥控模式准备就绪
// #define  JUDGE_READ                 (0x0F000000U)//判断决策
// #define  DELEC_USER_MODE            (0x00FFFFFFU)//清除用户自定义标志位


#define SET_CHA_RUNING_STATUS(_status_)  											\
				do																								\
				{																									\
					CLEAR_BIT(chassis_t.status,DISABLE_CHA_MOD_RUNNING);	\
					SET_BIT(chassis_t.status,_status_);							\
				}while(0)	
#define SET_CHA_READ_STATUS(__status_)  											\
				do																								\
				{																									\
					CLEAR_BIT(chassis_t.status,DISABLE_CHA_MOD_READ);	\
					SET_BIT(chassis_t.status,__status_);							\
				}while(0)																					

/* -------------- 外部链接 ----------------- */
extern TIM_HandleTypeDef htim5;
extern	osThreadId startChassisTaskHandle;
/* -------------- 发送队列 ----------------- */
  xQueueHandle chassis_queue;
/* -------------- 静态变量 ----------------- */
	 chassisStruct chassis_t;
    static RM3508Struct wheel1_t;         //轮子1
      static speedPidStruct wheel1Speed_t;
    static RM3508Struct wheel2_t;         //轮子2
      static speedPidStruct wheel2Speed_t;
     static powerBufferPoolStruct powerBufferPool_t;
			static currentMeterStruct currtenMeter_t;
       static gy955Struct gyroByCan_t;
/*-------------------------------------------------------
  |       型号     |  线数 |  brown | bule  |black|while|
  -------------------------------------------------------
  |欧姆龙 E6A2-CW3C|500P/R |5to12VDC|0V(GND)|OUT A|OUT B|
 -------------------------------------------------------*/
		static incrementalEnconderStruct chassisEnconder_t; //编码器结构体
/* -------------- 私有宏 ----------------- */
	#define CHASSIS_QUEUE_LEN      5U//深度为5
	#define CHASSIS_QUEUE_SIZE    8U//长度为5;
	/**
	* @Data    2019-01-27 17:09
	* @brief   底盘结构体数据初始化
	* @param  CAN_HandleTypeDef* hcanx（x=1,2）
	* @retval  void
	*/
	void ChassisInit(CAN_HandleTypeDef *hcan,const dbusStruct*rc,const pcDataStruct* pPc_t)
	{
		chassis_t.hcanx = hcan;
    chassis_t.rc_t =  rc;
		chassis_t.pPc_t = pPc_t;
    chassis_t.status = 0;
    chassis_t.p_refereeSystem_t = &ext_refereeSystem_t;
    chassis_t.pgyroByCan_t  = &gyroByCan_t;
    chassis_t.ppowerBufferPool_t = PowerBufferPoolInit();
		/* ------ 轮子1结构体数据初始化 ------- */
		chassis_t.pwheel1_t = wheel1Init();
		/* ------ 轮子2结构体数据初始化 ------- */
		chassis_t.pwheel2_t = wheel2Init();
  /* ------ 创建云台发送队列 ------- */
	  chassis_queue	= xQueueCreate(CHASSIS_QUEUE_LEN,CHASSIS_QUEUE_SIZE);//一定要在用之前创建队列
    /* -------- can配置 --------- */
      if(UserCanConfig(hcan)!= HAL_OK)
        while(1){}//待添加防重复配置功能
		/* ------ 编码器初始化 ------- */
	 	chassis_t.pchassisEnconder_t = &chassisEnconder_t;
		  if(EnconderInit(&chassisEnconder_t,RADIUS,ENCONDER_POLES) !=HAL_OK)
        while(1){}
		/* ------ 超声波初始化 ------- */
	  	HCSR04Init();
    /* -------- 底盘模块初始化判断 --------- */
      SET_BIT(chassis_t.status,INIT_OK);//初始化成功
	/* ------ 挂起任务，等待初始化 ------- */
    	vTaskSuspend(startChassisTaskHandle);
	/* ------ 设置机器人初始化状态 ------- */
     SetSetInitStatus();
//    vTaskSuspend(startChassisTaskHandle);
	}
/**
	* @Data    2019-01-28 11:40
	* @brief   底盘数据解析
	* @param   void
	* @retval  void
	*/
	void ChassisParseDate(uint16_t id,uint8_t *data)
	{
		switch (id)
		{
			case WHEEL1_RX_ID:
				RM3508ParseData(&wheel1_t,data);
				break;
			case WHEEL2_RX_ID:
				RM3508ParseData(&wheel2_t,data);
				break;
	  	case CURRENT_METER_RX_ID:
				 CurrentMeterAnalysis(&currtenMeter_t,data);
				break;
      case BIN_GE_GYRO_CAN_RX_ID:
        BingeGyroByCan(&gyroByCan_t,data);
        break;
			default:
				break;
		}
    SET_BIT(chassis_t.status,RX_OK);//接受成功
	}
///**
//	* @Data    2019-02-15 15:10
//	* @brief   底盘数据发送
//	* @param   void
//	* @retval  void
//	*/
//	void ChassisUserCanTx(int16_t w1,int16_t w2)
//	{
//		uint8_t s[8]={0};
//		s[0] = (uint8_t)(w1<<8);
//		s[1] = (uint8_t)w1;
//		s[2] = (uint8_t)(w2<<8);
//		s[3] = (uint8_t)w2;
//		CanTxMsg(chassis_t.hcanx,CHASSIS_CAN_TX_ID,s);
//	}
/**
	* @Data    2019-03-12 16:25
	* @brief   底盘控制函数
	* @param   void
	* @retval  void
	*/
  uint32_t chassiscommot =0;
	void ChassisControl(void)
	{
		int16_t pid_out[2];
    Inject(&powerBufferPool_t);//更新功率缓存池状态
		SetArea();//待测试
 chassiscommot	= ChassisControlDecision();
	ChassisControlSwitch(chassiscommot);
    chassis_t.pwheel1_t->error = chassis_t.pwheel1_t->target - chassis_t.pwheel1_t->real_speed;
     chassis_t.pwheel2_t->error = chassis_t.pwheel2_t->target - chassis_t.pwheel2_t->real_speed;
		pid_out[0] = SpeedPid(chassis_t.pwheel1_t->pspeedPid_t,chassis_t.pwheel1_t->error);
  	pid_out[1] = SpeedPid(chassis_t.pwheel2_t->pspeedPid_t,chassis_t.pwheel2_t->error);
    SetInPut(&powerBufferPool_t,pid_out,2);//开功率缓存池
		ChassisCanTx(pid_out[0],pid_out[1]);
    SET_BIT(chassis_t.status,RUNING_OK);
	}
 /*
* @Data    2019-02-24 11:59
* @brief   获取底盘结构体地址
* @param   void
* @retval  void
*/
const chassisStruct* GetChassisStructAddr(void)
{
  return &chassis_t;
}
  /**
  * @Data    2019-03-13 03:48
  * @brief   获取底盘结构体地址 可读写，不能乱调用
  * @param   void
  * @retval  void
  */
  chassisStruct *RWGetChassisStructAddr(void)
  {
    return &chassis_t;
  }
  /**
  * @Data    2019-03-13 01:55
  * @brief   获取底盘状态
  * @param   void
  * @retval  void
  */
  uint32_t GetChassisStatus(void)
  {
    return chassis_t.status;
  }
  /**
  * @Data    2019-03-13 02:56
  * @brief   设置电机目标速度
  * @param   void
  * @retval  void
  */
  void SetMotorTarget(int16_t w1,int16_t w2)
  {
    	wheel1_t.target = w1;
      wheel2_t.target = w2;
  }
  /**
  * @Data    2019-03-13 03:19
  * @brief   遥控模式
  * @param   void
  * @retval  void
  */
int16_t rc_coe=7;
  void ChassisRcControlMode(void)
  {
		if((chassis_t.status & CHASSIS_RC_MODE_RUNING)!= CHASSIS_RC_MODE_RUNING)
		{
			chassis_t.pwheel1_t->target = 0;
			chassis_t.pwheel2_t->target = 0;
			SET_CHA_RUNING_STATUS(CHASSIS_RC_MODE_RUNING);
		}
		chassis_t.pwheel1_t->target = chassis_t.rc_t->ch1 *rc_coe;
		chassis_t.pwheel2_t->target = chassis_t.rc_t->ch1 *rc_coe;
  }
  /**
  * @Data    2019-03-25 00:28
  * @brief   控制优先权决策
  * @param   void
  * @retval  void
  */
  // void ChassisControlPriorityDecision(void)
  // {
  //   if(chassis_t.rc_t->switch_right ==1)
	// 	{
	// 		switch ()
	// 	}
	// 	else if(chassis_t.rc_t->switch_right ==2)
	// 	{
	// 	 	SET_BIT(chassis_t.status,RC_MODE_READ);
  //     CLEAR_BIT(chassis_t.status,ELUDE_MODE_READ);
  //     CLEAR_BIT(chassis_t.status,PC_SHOOT_MODE_READ);
	// 		CLEAR_BIT(chassis_t.status,CRUISE_MODE_READ);
	// 	}

  //     // SET_BIT(chassis_t.status,RC_MODE_RUNING);
  //     // CLEAR_BIT(chassis_t.status,ELUDE_MODE_RUNING);
  //     // CLEAR_BIT(chassis_t.status,PC_SHOOT_MODE_RUNING);
  //     // CLEAR_BIT(chassis_t.status,CRUISE_MODE_RUNING);
  // }
/**
	* @Data    2019-02-15 15:10
	* @brief   云台数据发送
	* @param   void
	* @retval  void
	*/
	HAL_StatusTypeDef ChassisCanTx(int16_t w1,int16_t w2)
	{
		uint8_t s[8];
    memset(s,0,8);
    s[0] = (uint8_t)(w1>>8) & 0xFF;
    s[1] = (uint8_t)w1;
    s[2] = (uint8_t)(w2>>8) & 0xFF;
    s[3] = (uint8_t)w2;
   	xQueueSendToBack(chassis_queue,s,0);
		return HAL_OK;
	}
// /**
// 	* @Data    2019-03-25 19:05
// 	* @brief   获取云台命令
// 	* @param   void
// 	* @retval  void
// 	*/
// 	void GetGimbalCom(void)
// 	{
		
// 	}

// /**
// 	* @Data    2019-03-25 19:11
// 	* @brief   获取机器人状态
// 	* @param   void
// 	* @retval  void
// 	*/
// 	void GetRotbStatus(void)
// 	{
// 		if()
// 	}
/**
	* @Data    2019-03-25 19:13
	* @brief   巡航模式
	* @param   void
	* @retval  void
	*/
		 int16_t zhongd = 1500;
		 int16_t cru_speed = 800;
		 int16_t direction =0;
	void ChassisCruiseModeControl(void)
	{
//			uint32_t position;
		if((chassis_t.status & CHASSIS_CRUISE_MODE_RUNING) != CHASSIS_CRUISE_MODE_RUNING)
		{
			ChassisCruiseModeInit();
      chassis_t.pwheel1_t->target = cru_speed;
		}
		if(chassis_t.State.r_area == MID_ROAD)
		{
			 direction = chassis_t.State.last_area -chassis_t.State.r_area;//看3508的方向
			 chassis_t.pwheel1_t->target = direction*chassis_t.pwheel1_t->target;
			 chassis_t.pwheel2_t->target = chassis_t.pwheel1_t->target;
		}
		else if(chassis_t.State.r_area != MID_ROAD)
		{
			 direction = -(chassis_t.State.last_area -chassis_t.State.r_area);//看3508的方向
			 chassis_t.pwheel1_t->target = direction*chassis_t.pwheel1_t->target;
			 chassis_t.pwheel2_t->target = chassis_t.pwheel1_t->target;
		}
	}
/**
	* @Data    2019-03-30 15:50
	* @brief   设置续航模式初始化
	* @param   void
	* @retval  void
	*/
	void ChassisCruiseModeInit(void)
	{
		SET_CHA_RUNING_STATUS(CHASSIS_CRUISE_MODE_READ);
	}

/**
	* @Data    2019-03-25 19:16
	* @brief   更新机器人位置和状态
	* @param   void
	* @retval  void
	*/

// 	void GetRobotStatus(void)
// 	{
// 		


// 	}

/**
	* @Data    2019-03-25 19:16
	* @brief   获取机器人被攻击的状态
	* @param   void
	* @retval  void
	*/
int16_t f_hurt_flag=0;
int16_t f_hurt_fre=0;
int16_t b_hurt_flag=0;
int16_t b_hurt_fre=0;
uint8_t GetHurtStatus(void)
{
	if(chassis_t.p_refereeSystem_t->p_robot_hurt_t->hurt_type ==0)
	{
		f_hurt_flag =500;
	}
	else
	{
    
		if(f_hurt_flag >0)
    {
      f_hurt_flag --;
     return (chassis_t.p_refereeSystem_t->p_robot_hurt_t->armor_id);
    }
 else f_hurt_flag =0;
 
    
	}
  	 return 0x55;
}

/**
* @Data    2019-03-20 21:27
* @brief   控制权切换
* @param   void
* @retval  void
*/
void ChassisControlSwitch(uint32_t commot)
{
 switch (commot)
 {
   case CHASSIS_RC_MODE_READ:
       ChassisRcControlMode();
     break;
   case CHASSIS_ELUDE_MODE_READ:
    		ChassisEludeControlMode();
		  break;
	case CHASSIS_PC_SHOOT_MODE_READ:
	    ChassisPcShootMode();
    break;
      case CHASSIS_CRUISE_MODE_READ:
       ChassisCruiseModeControl();
     break;
   default:
    ChassisDeinit();
	    break;
 }
}
/**
* @Data    2019-03-20 21:27
* @brief   决策控制判断
* @param   void
* @retval  void
*/
  uint32_t ChassisControlDecision(void)
  {
    if(chassis_t.rc_t ->switch_left ==2)
    {
      CLEAR_BIT(chassis_t.status,DISABLE_CHASSIS);
     return (chassis_t.status & CHASSIS_JUDGE_READ);
    }
    else if(chassis_t.rc_t->switch_right ==1)
    {
		 if(GetHurtStatus() !=0x55)
      {
        SET_CHA_READ_STATUS(CHASSIS_ELUDE_MODE_READ);
      }
     if(chassis_t.pPc_t->commot ==1)
      {
        SET_CHA_READ_STATUS(CHASSIS_PC_SHOOT_MODE_READ);
      }
      else SET_CHA_READ_STATUS(CHASSIS_CRUISE_MODE_READ);
    }
    else if(chassis_t.rc_t->switch_right ==2)
    {
			 SET_CHA_READ_STATUS(CHASSIS_RC_MODE_READ);
    }   
    return (chassis_t.status & CHASSIS_JUDGE_READ);
  }
/**
* @Data    2019-03-21 00:46
* @brief   底盘模块失能//待完善
* @param   void
* @retval  void
*/
void ChassisDeinit(void)
{
chassis_t.pwheel1_t->target = 0;
chassis_t.pwheel2_t->target = 0;
}
/**
* @Data    2019-03-21 00:46
* @brief   躲避模式//待完善
* @param   void
* @retval  void
*/
void ChassisEludeControlMode(void)
{
	ChassisEludeControlModeInit();
	if(GetHurtStatus() == AHEAD_OF_ARMOR)
	{
		switch (chassis_t.State.r_area) 
		{
			case UP_ROAD:
			  Go(DOWN_ROAD,4000);
				break;
			case MID_ROAD:
		  	Go(UP_ROAD,4000);
				break;
			case DOWN_ROAD:
		  	Go(UP_ROAD,4000);
				break;
			default:
				break;
		}
	}
}

/**
	* @Data    2019-03-31 12:06
	* @brief   躲避模式初始化
	* @param   void
	* @retval  void
	*/
	void ChassisEludeControlModeInit(void)
	{
	if((chassis_t.status & CHASSIS_ELUDE_MODE_RUNING) != CHASSIS_ELUDE_MODE_RUNING)
		{
			SET_CHA_RUNING_STATUS(CHASSIS_ELUDE_MODE_RUNING);
		}
	}

/**
	* @Data    2019-03-30 22:13
	* @brief   跑到指定目的地
	* @param   void
	* @retval  void
	*/
	int16_t Go(int16_t target,int16_t speed)
	{
		int16_t dire;
		dire =chassis_t.State.r_area  - target;
			if(ABS(dire) >1)
			dire =(int16_t)( dire *0.5);
			speed = dire * speed;
			SetMotorTarget(speed,speed); 
			return dire;


	}

/**
	* @Data    2019-03-31 12:02
	* @brief   击打模式控制
	* @param   void
	* @retval  void
	*/
	void ChassisPcShootMode(void)
	{
		ChassisPcShootModeInit();
		SetMotorTarget(0,0);
	}
/**
	* @Data    2019-03-31 12:03
	* @brief   击打模式初始化
	* @param   void
	* @retval  void
	*/
	void ChassisPcShootModeInit(void)
	{
	 if((chassis_t.status & CHASSIS_PC_SHOOT_MODE_RUNING) != CHASSIS_PC_SHOOT_MODE_RUNING)
		{
			SET_CHA_RUNING_STATUS(CHASSIS_PC_SHOOT_MODE_RUNING);
		}
	}



  /**
  * @Data    2019-03-25 00:08
  * @brief   初始化功率缓存池
  * @param   void
  * @retval  void
  */
  powerBufferPoolStruct* PowerBufferPoolInit(void)
  {
    powerBufferPool_t.pcurrentMeter_t = &currtenMeter_t;
    powerBufferPool_t.max_p = 20.0;
    powerBufferPool_t.max_w = 200.0;//功率单位mW
    powerBufferPool_t.r_p = 0.0;
    powerBufferPool_t.r_w = 200.0;//功率单位mW
    powerBufferPool_t.current_mapp_coe = 0.00122;//电流映射系数
    powerBufferPool_t.high_water_level = 150.0;
    powerBufferPool_t.low_water_level = 30.0;
    powerBufferPool_t.mid_water_level = 80.0;
    powerBufferPool_t.period = 0.01;//运行周期，单位/s
    powerBufferPool_t.high_current_threshold = 5.0;//mA
    powerBufferPool_t.mid_current_threshold = 2.0;//mA
    powerBufferPool_t.low_current_threshold = 0.9;//mA
    powerBufferPool_t.safe_current_threshold = 0.7;//mA
    return &powerBufferPool_t;
  }
  /**
  * @Data    2019-03-25 00:12
  * @brief   初始化轮子参数
  * @param   void
  * @retval  void
  */
  RM3508Struct* wheel1Init(void)
  {
      wheel1_t.id = WHEEL1_RX_ID;//电机can的 ip
//			wheel1_t.target = 0;		 //目标值
			wheel1_t.tem_target = 0;//临时目标值
			wheel1_t.real_current = 0; //真实电流
			wheel1_t.real_angle = 0;//真实角度
			wheel1_t.tem_angle = 0;//临时角度
  	  wheel1_t.real_speed = 0;//真实速度
  	  wheel1_t.tem_speed = 0;//真实速度
			wheel1_t.zero = 0;			 //电机零点
			wheel1_t.Percentage = 0;//转换比例（减速前角度:减速后的角度 = x:1
			wheel1_t.thresholds = 0; //电机反转阀值
  	  wheel1_t.error = 0;//当前误差
      wheel1_t.ppostionPid_t = NULL;
      wheel1_t.pspeedPid_t = &wheel1Speed_t;
//				wheel1Speed_t.kp = 1;
//				wheel1Speed_t.kd = 0;
//				wheel1Speed_t.ki = 0;
				wheel1Speed_t.error = 0;
				wheel1Speed_t.last_error = 0;//上次误差
				wheel1Speed_t.before_last_error = 0;//上上次误差
				wheel1Speed_t.integral_er = 0;//误差积分
				wheel1Speed_t.pout = 0;//p输出
				wheel1Speed_t.iout = 0;//i输出
				wheel1Speed_t.dout = 0;//k输出
				wheel1Speed_t.pid_out = 0;//pid输出
//				wheel1Speed_t.limiting = W1_LIMIT_SPEED;//轮子1速度限幅
        return &wheel1_t;
  }
  /**
  * @Data    2019-03-25 00:15
  * @brief   轮子2参数初始化
  * @param   void
  * @retval  void
  */
  RM3508Struct* wheel2Init(void)
  {
			wheel2_t.id = WHEEL2_RX_ID;//电机can的 ip
//			wheel2_t.target = 0;		 //目标值
			wheel2_t.tem_target = 0;//临时目标值
			wheel2_t.real_current = 0; //真实电流
			wheel2_t.real_angle = 0;//真实角度
			wheel2_t.tem_angle = 0;//临时角度
			wheel2_t.real_speed = 0;//真实速度
			wheel2_t.tem_speed = 0;//真实速度
			wheel2_t.zero = 0;			 //电机零点
			wheel2_t.Percentage = 0;//转换比例（减速前角度:减速后的角度 = x:1
			wheel2_t.thresholds = 0; //电机反转阀值
			wheel2_t.error = 0;//当前误差
 			wheel2_t.ppostionPid_t = NULL;
      wheel2_t.pspeedPid_t = &wheel2Speed_t;
//				wheel2Speed_t.kp = 1;
//				wheel2Speed_t.kd = 0;
//				wheel2Speed_t.ki = 0;
				wheel2Speed_t.error = 0;
				wheel2Speed_t.last_error = 0;//上次误差
				wheel2Speed_t.before_last_error = 0;//上上次误差
				wheel2Speed_t.integral_er = 0;//误差积分
				wheel2Speed_t.pout = 0;//p输出
				wheel2Speed_t.iout = 0;//i输出
				wheel2Speed_t.dout = 0;//k输出
				wheel2Speed_t.pid_out = 0;//pid输出
//				wheel2Speed_t.limiting = W2_LIMIT_SPEED;//轮子2速度限幅
      return &wheel2_t;
  }
 /**
	 * @Data    2019-03-19 17:58
	 * @brief   底盘初始化状态设置
	 * @param   void
	 * @retval  void
	 */
 	void SetSetInitStatus(void)
  {
    	wheel1_t.target = 0;		 //目标值
    		wheel1Speed_t.kp = 9;
				wheel1Speed_t.kd = 1.5;
				wheel1Speed_t.ki = 0.9;
     	wheel1Speed_t.limiting = W1_LIMIT_SPEED;//轮子1速度限幅
    
    	wheel2_t.target = 0;		 //目标值
    	wheel2Speed_t.kp = 9;
			wheel2Speed_t.kd = 1.5;
			wheel2Speed_t.ki = 0.9;
    	wheel2Speed_t.limiting = W2_LIMIT_SPEED;//轮子2速度限幅
		/* ------ 方向状态初始化 ------- */
			chassis_t.State.r_dire = (int16_t)chassis_t.pgyroByCan_t->Yaw;
		/* ------ 设置初始化区域位置 ------- */
			chassis_t.State.r_area  = UP_ROAD;
			chassis_t.State.last_area = UP_ROAD;//添加超声波检测是否位置正确
     /* ------ 设置启动标志位 ------- */  
        SET_BIT(chassis_t.status,START_OK);  
  }
	/**
		* @Data    2019-03-30 17:16
		* @brief   机器人所处区域获取
		* @param   void
		* @retval  void
		*/
uint8_t up_turn = 1;
 uint8_t road_flag =0;
	void SetArea(void)
	{
//	if(GetOrgans() != 0)
//	{
   up_turn =GetOrgans();
//	}
		if(GetGyroDire()<TURNING_ANGLE)//陀螺仪状态//up
		{
				// temp = UP_ROAD;
				if(up_turn ==1)
				{
					if(chassis_t.State.r_area  != UP_ROAD)
					{
						chassis_t.State.last_area = chassis_t.State.r_area;
					}
					chassis_t.State.r_area = UP_ROAD;
				}
			else
			{
				if(chassis_t.State.r_area  != DOWN_ROAD)
				{
					chassis_t.State.last_area = chassis_t.State.r_area;
				}
				chassis_t.State.r_area = DOWN_ROAD;
			}

		}
		else if(GetGyroDire() >= TURNING_ANGLE)//mid
		{
			if(chassis_t.State.r_area  != MID_ROAD)
			{
				chassis_t.State.last_area = chassis_t.State.r_area;
			}
			chassis_t.State.r_area = MID_ROAD;
		}
	}
/**
	* @Data    2019-03-30 19:58
	* @brief   
	* @param   void
	* @retval  void
	*/
	int16_t  GetGyroDire(void)
	{
		int16_t dire=0;
		dire = chassis_t.State.r_dire - (int16_t)chassis_t.pgyroByCan_t->Yaw;
		return ABS(dire);
	}
int16_t organs_flag =1;//初始化为1，既是初始化为上路
int16_t organs = 1;
int16_t temp_organs =0;
	uint8_t GetOrgans(void)
	{
    	 temp_organs = HAL_GPIO_ReadPin(LASER_SWITCH_GPIO,LASER_SWITCH);//读io口
    
//   	if(temp_organs ==0)//激光开关
//	  {
//		  organs_flag ++;
//	  }
//    if(GetGyroDire() >= TURNING_ANGLE  && organs_flag >10)//
//    {
      if(GetGyroDire() <= 50  && temp_organs == 0) //1是被遮住，0是没遮住
      {
        organs_flag  =1;//从上路回到中路
      }
      if(GetGyroDire() >= 50)
      {
        organs_flag = 0;
      }
//      else if(GetGyroDire() >= TURNING_ANGLE &&organs_flag ==1)
//      {
//        organs_flag  =0;//从中路回到上路
//      }
       return  organs_flag;
		// temp_organs = HAL_GPIO_ReadPin(LASER_SWITCH_GPIO,LASER_SWITCH);//读io口
//  	if(temp_organs ==0)//激光开关
//	{
//		organs_flag ++;
//	}
//	else organs_flag --;
//	if(organs_flag <0)
//    return  0;  
//	else if(organs_flag >30)
//   	return  1; 
//    if(temp_organs !=0 )
//    {
//      organs = 0;
//    }
//    
//    return  organs;
		// uint8_t temp;
//	if(organs ==0)//激光开关
//	{
//		organs_flag ++;
//	}
//	else organs_flag --;
//	if(organs_flag <0)
//    return  0;  
//	else if(organs_flag >30)
//   	return  1; 
//  else   return  0; 
	}
	/**
		* @Data    2019-03-30 17:16
		* @brief   底盘自检
		* @param   void
		* @retval  void
		*/
 uint8_t AutoCalibratorMode(void)
 {
   
 }
/*----------------------------------file of end-------------------------------*/

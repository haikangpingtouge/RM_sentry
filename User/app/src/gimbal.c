/**
	|--------------------------------- Copyright --------------------------------|
	|                                                                            |
	|                      (C) Copyright 2019,����ƽͷ��,                         |
	|           1 Xuefu Rd, Huadu Qu, Guangzhou Shi, Guangdong Sheng, China      |
	|                           All Rights Reserved                              |
	|                                                                            |
	|           By(GCU The wold of team | ����������ѧ����ѧԺ������Ұ�Ƕ�)         |
	|                    https://github.com/GCUWildwolfteam                      |
	|----------------------------------------------------------------------------|
	|--FileName    : gimbal.c                                              
	|--Version     : v2.0                                                          
	|--Author      : ����ƽͷ��                                                     
	|--Date        : 2019-01-26             
	|--Libsupports : 
	|--Description :                                                     
	|--FunctionList                                                     
	|-------1. ....                                                     
	|          <version>:                                                     
	|     <modify staff>:                                                       
	|             <data>:                                                       
	|      <description>:                                                        
	|-------2. ...                                                       
	|-----------------------------declaration of end-----------------------------|
 **/
#include "gimbal.h" 
extern UART_HandleTypeDef huart2;//����1
/* -------------- ˽�к궨�� ----------------- */
	#define RAMMER_RX_ID	  0x207
	#define YAW_RX_ID 			0x205
	#define PITCH_RX_ID 		0x206
	#define GIMBAL_CAN_TX_ID 0x1ff
/* -------------- �ṹ������ ----------------- */
	gimbalStruct gimbal_t;//��̨�ṹ��
//	M2006Struct rammer_t;//��������ṹ��
//	postionPidStruct rammerOuterLoopPid_t;//��������⻷pid
//	speedPidStruct rammerInnerLoopPid_t;//��������ڻ�pid
//	xQueueHandle gimbal_queue;
	/**
	* @Data    2019-01-27 17:09
	* @brief   ��̨�ṹ���ʼ��
	* @param  CAN_HandleTypeDef* hcanx��x=1,2��
	* @retval  void
	*/
	void GimbalStructInit(CAN_HandleTypeDef *hcanx)
	{
		gimbal_t.hcanx = hcanx;
		/* ------ ���������ʼ�� ------- */
//			gimbal_t.prammer_t = &rammer_t;
//			rammer_t.id = RAMMER_RX_ID;//���can�� ip
//		  rammer_t.target = 0; //Ŀ��ֵ
//		  rammer_t.real_current = 0;//��ʵ����
//		  rammer_t.real_angle = 0;//��ʵ�Ƕ�
//		  rammer_t.real_speed = 0;//��ʵ�ٶ�
//			rammer_t.error = 0;
//			/* ------ �⻷pid���� ------- */
//				rammer_t.ppostionPid_t = &rammerOuterLoopPid_t;
//				rammerOuterLoopPid_t.kp = 0;
//				rammerOuterLoopPid_t.kd = 0;
//				rammerOuterLoopPid_t.ki = 0;
//				rammerOuterLoopPid_t.error = 0;
//				rammerOuterLoopPid_t.last_error = 0;//�ϴ����
//				rammerOuterLoopPid_t.integral_er = 0;//������
//				rammerOuterLoopPid_t.pout = 0;//p���
//				rammerOuterLoopPid_t.iout = 0;//i���
//				rammerOuterLoopPid_t.dout = 0;//k���
//				rammerOuterLoopPid_t.pid_out = 0;//pid���
//			/* ------ �ڻ�pid���� ------- */
//				rammer_t.pspeedPid_t = &rammerInnerLoopPid_t;
//				rammerInnerLoopPid_t.kp = 0;
//				rammerInnerLoopPid_t.kd = 0;
//				rammerInnerLoopPid_t.ki = 0;
//				rammerInnerLoopPid_t.error = 0;
//				rammerInnerLoopPid_t.last_error = 0;//�ϴ����
//				rammerInnerLoopPid_t.before_last_error = 0;//���ϴ����
//				rammerInnerLoopPid_t.integral_er = 0;//������
//				rammerInnerLoopPid_t.pout = 0;//p���
//				rammerInnerLoopPid_t.iout = 0;//i���
//				rammerInnerLoopPid_t.dout = 0;//k���
//				rammerInnerLoopPid_t.pid_out = 0;//pid���
				// /* ------ ��̨��Ϣ���д��� ------- */
				// gimbal_queue = xQueueCreate(5,3);
        UserCanConfig(hcanx);//�����ӷ��ظ����ù���

	}
/**
	* @Data    2019-01-28 11:40
	* @brief   ��̨���ݽ���
	* @param   void
	* @retval  void
	*/
	void GimbalParseDate(uint32_t id,uint8_t *data)
	{
		switch (id)
		{
			case RAMMER_RX_ID:
//				RM2006ParseData(&rammer_t,data);
				break;
			case YAW_RX_ID:
				RM6623ParseData(gimbal_t.pYaw_t,data);
        /* -------- ����ת�� --------- */
        gimbal_t.pYaw_t->real_angle = RatiometricConversion  \
        (gimbal_t.pYaw_t->real_angle,gimbal_t.pYaw_t->thresholds,gimbal_t.pYaw_t->Percentage);
        /* -------- ���㴦�� --------- */
         zeroArgument(gimbal_t.pYaw_t->real_angle,gimbal_t.pYaw_t->thresholds); 
				break;
			case PITCH_RX_ID:
				RM6623ParseData(gimbal_t.pPitch_t,data);
        /* -------- ���㴦�� --------- */
         zeroArgument(gimbal_t.pPitch_t->real_angle,gimbal_t.pPitch_t->thresholds); 
				break;
		
			default:
				break;
		}

	}
/**
	* @Data    2019-02-14 21:01
	* @brief   ��̨����
	* @param   void
	* @retval  void
	*/
	void GimbalControl(const dbusStruct* dbus)
	{
//		int16_t pid_out = -500;
//		rammer_t.target = 3*(DbusAntiShake(20,dbus->ch1)); //Ŀ��ֵ
//		rammer_t.error = rammer_t.target - rammer_t.real_speed;
//		pid_out = SpeedPid(&rammerInnerLoopPid_t,rammer_t.error);
//		pid_out = MAX(pid_out,2000); //������ֵ
//	  pid_out = MIN(pid_out,-2000); //����Сֵ
//		GimbalCanTx(pid_out,0);
	}
/**
	* @Data    2019-02-15 15:10
	* @brief   ��̨���ݷ���
	* @param   void
	* @retval  void
	*/
	HAL_StatusTypeDef GimbalCanTx(int16_t yaw,int16_t pitch,int16_t rammer)
	{
		uint8_t s[8]={0};
    s[0] = (uint8_t)(yaw>>8);
    s[1] = (uint8_t)yaw;
    s[2] = (uint8_t)(pitch>>8);
    s[3] = (uint8_t)pitch;
		s[4] = (uint8_t)(rammer>>8);
		s[5] = (uint8_t)rammer;
		return(CanTxMsg(gimbal_t.hcanx,GIMBAL_CAN_TX_ID,s));
	}
// /*---------------------------------80�ַ�����-----------------------------------*/
//   /**
//   * @Data    2019-02-24 23:55
//   * @brief   С�������ݽ���
//   * @param   void
//   * @retval  void
//   */
//   HAL_StatusTypeDef RxPCMsg(void)
//   {
//     return (HAL_UART_Receive(gimbal_t.huartx,pc_data,3,1));
//   }
/*-----------------------------------file of end------------------------------*/
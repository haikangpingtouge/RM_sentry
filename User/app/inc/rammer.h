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
	|--FileName    : rammer.h                                                
	|--Version     : v1.0                                                            
	|--Author      : ����ƽͷ��                                                       
	|--Date        : 2019-03-16               
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
#ifndef __RAMMER_H 
#define __RAMMER_H 
#include "motor.h"
#include "DJI_dbus.h" 
	#define RAMMER_RX_ID	 					  0x207//�����������id
	#define RAMMER_ANGER_COE         (0.0012F)//��������Ƕ�ת��ϵ��
	#define RAMMER_REDUCTION         (36U)
	#define STUCK_BULLET_THRE        (3000U)//����������ֵ
	#define RAMMER_TIME             (50U)  // 500ms ��λ10ms �������
  #define LOCK_ROTOT_TIME         (100U) //1s ��תʱ��
  #define CLOCK_WISE             //˳ʱ��Ϊ��
  #define PARTITION_NUMB           (1365U)//ÿ��ת��ֵPARTITION_NUMB = M2006_POLES/x
  #define SHAKE_VAULE               (600U)//������Χ
  // #define ANTI_CLOCK_WISE	        //��ʱ��Ϊ������
	M2006Struct* RammerInit(void);
	void RammerControl(void);
		int16_t PCycleNumerical(int16_t data);
		int16_t MCycleNumerical(int16_t data);
      void RammerShake(void);
#endif	// __RAMMER_H
/*-----------------------------------file of end------------------------------*/


/**
	|-------------------------------- Copyright -----------------------------------|
	|                                                                              |
	|                        (C) Copyright 2019,����ƽͷ��,                         |
	|            1 Xuefu Rd, Huadu Qu, Guangzhou Shi, Guangdong Sheng, China       |
	|                            All Rights Reserved                               |
	|                                                                              |
	|            By(GCU The wold of team | ����������ѧ����ѧԺ������Ұ�Ƕ�)          |
	|                     https://github.com/GCUWildwolfteam                       |
	|------------------------------------------------------------------------------|
	|--FileName    : bsp_can.h                                                
	|--Version     : v1.0                                                            
	|--Author      : ����ƽͷ��                                                       
	|--Date        : 2019-01-18               
	|--Libsupports : STM32CubeF1 Firmware Package V1.6.0(�ñ�Ŀ�����������)
	|--Description :                                                       
	|--FunctionList                                                       
	|-------1. ....                                                       
	|          <version>:                                                       
	|     <modify staff>:                                                       
	|             <data>:                                                       
	|      <description>:                                                        
	|-------2. ...                                                       
	|---------------------------------declaration of end----------------------------|
 **/
#ifndef __BSP_CAN_H 
#define __BSP_CAN_H 
#include "baseclass.h"
typedef struct canDataStrcut
{
	CAN_TxHeaderTypeDef txMsg;
	CAN_RxHeaderTypeDef rxMsg;
	CAN_FilterTypeDef filter;
	uint8_t txdata[8];
	uint8_t rxdata[8];
	xQueueHandle can_queue; //���о��
	char queue_data[16];//�������ݿռ�
} canDataStrcut;
HAL_StatusTypeDef UserCanConfig(CAN_HandleTypeDef *hcanx);
HAL_StatusTypeDef CanFilterInit(CAN_HandleTypeDef *hcanx);
HAL_StatusTypeDef CanTxInit(CAN_HandleTypeDef *hcanx);
HAL_StatusTypeDef CanRxInit(CAN_HandleTypeDef *hcanx);
HAL_StatusTypeDef AllocateCanxSpace(CAN_HandleTypeDef *hcanx);
canDataStrcut* GetCantAddr(CAN_HandleTypeDef *hcanx);
HAL_StatusTypeDef CanQueueCreate(canDataStrcut *canx,uint8_t len,uint8_t deep);
#endif // __BSP_CAN_H

/*------------------------------------file of end-------------------------------*/


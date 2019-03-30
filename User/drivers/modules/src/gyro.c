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
  |--FileName    : gyro.c                                                
  |--Version     : v1.0                                                            
  |--Author      : ����ƽͷ��                                                       
  |--Date        : 2019-03-25               
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
#include "gyro.h" 
#define GY955BACKLEN   3
#define GY955LEN   (18+GY955BACKLEN)
#define EULERANGLE 0x08
#define GYRO       0x04
  /**
  * @Data    2019-03-25 01:32
  * @brief   gy955��ʼ��
  * @param   void
  * @retval  void
  */
uint8_t GB955Buffer[18];//���ݽ���
void Gy955Init(UART_HandleTypeDef *huartx)
{
  UsartAndDMAInit(huartx,GY955LEN,ENABLE);
}
  /**
  * @Data    2019-03-25 01:32
  * @brief   gy955���ݽ���
  * @param   void
  * @retval  void
  */
void AnalysisGyro(gy955Struct* Gyc)
{
  uint8_t sum,i;
	if ((GB955Buffer[0]==0x5A)&&(GB955Buffer[1]==0x5A))
	 {
	 
		for (sum=0,i = 0; i < GB955Buffer[3]+4; i++)  sum+=GB955Buffer[i];
	
		if(GB955Buffer[i]==sum)//У����ж�
		{
			if((GB955Buffer[2]&GYRO)==GYRO)
			 {
				Gyc->Gyr_X=((GB955Buffer[4]<<8)|GB955Buffer[5]);
				Gyc->Gyr_y=((GB955Buffer[6]<<8)|GB955Buffer[7]);
				Gyc->Gyr_z=((GB955Buffer[8]<<8)|GB955Buffer[9]);
			 }
			if ((GB955Buffer[2]&EULERANGLE)==EULERANGLE)
			 {
				Gyc->Yaw=(uint16_t)((GB955Buffer[10]<<8)|GB955Buffer[11])/100.0f;
				Gyc->Roll=(int16_t)((GB955Buffer[12]<<8)|GB955Buffer[13])/100.0f;
				Gyc->Pitch=(int16_t)((GB955Buffer[14]<<8)|GB955Buffer[15])/100.0f;
			 }
		
			Gyc->counter++;
		}
	 }
}
void BingeGyroByCan(gy955Struct* Gyc,uint8_t *data)
{
  floatToUnion p;
  p.u_8[0] = data[0];
  p.u_8[1] = data[1];
  p.u_8[2] = data[2];
  p.u_8[3] = data[3];
  Gyc->Yaw = p.f;
  p.f = 0;
  p.u_8[0] = data[4];
  p.u_8[1] = data[5];
  p.u_8[2] = data[6];
  p.u_8[3] = data[7];
  Gyc->Gyrz = p.f;
}
/*------------------------------------file of end-------------------------------*/


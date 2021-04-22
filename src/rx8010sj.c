#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <termios.h>  
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>     // for socket
#include <sys/socket.h>    // for socket
#include <arpa/inet.h>     //for inet_addr
#include <sys/wait.h>
#include <pthread.h> 
#include <sys/mman.h>  
#include <semaphore.h>
#include <ctype.h>
#include <sys/time.h>
#include <stdint.h>
#include <linux/types.h>

#include "getcfg.h"
#include "setnetinfo.h"
#include "arm_gpio.h"
#include "ethpro.h"
#include "emif.h"
#include "acquisition.h"
#include "rx8010sj.h"




void delayus(unsigned int us)
{
	unsigned int i;
	while(us--)
	{
		for(i=0;i<30;i++)
			;
	}	
}
/******************************************************************************
                               ������ʾʱ���ʽ
                        Ҫ�ı���ʾ�ĸ�ʽ���޸Ĵ�����
******************************************************************************/

u8 Display_Time[8] = {0x30,0x30,0x3a,0x30,0x30,0x3a,0x30,0x30};	
					//ʱ����ʾ����   ��ʽ  00:00:00

u8 Display_Date[13] = {0x32,0x30,0x31,0x37,0x2f,0x30,0x31,0x2f,0x32,0x30,0x20,0x37,0x57};
					//������ʾ����   ��ʽ  2013/10/20 7W

/******************************************************************************
                            ������صı�������
******************************************************************************/

Time_Typedef TimeValue,TimeDefault;	//����ʱ������ָ��
//tm timer;
u8 Time_Buffer[8];	//ʱ���������ݻ���
//u8 Time_Buffer1[16];
unsigned char buffer[4];

/******************************************************************************
                            IIC��������
******************************************************************************/
//��ʼ��IIC
void IIC_Init(void)
{					     
	;
}


//����SDAΪ����ģʽ
void SDA_IN(void) 
{
	seti2csdadir(SDAIN);
	delayus(1);
	fpga_sda_dir(FPGASDA2ARM);
	delayus(1);
}

//����SDAΪ���ģʽ
void SDA_OUT(void) 
{
	fpga_sda_dir(ARM2FPGASDA);
	delayus(1);
	seti2csdadir(SDAOUT);
	delayus(1);
}

/*
//����SDAΪ����ģʽ
void SDA_IN(void) 
{

	fpga_sda_dir(FPGASDA2ARM);
	delayus(1);
	seti2csdadir(SDAIN);
	delayus(1);
}

//����SDAΪ���ģʽ
void SDA_OUT(void) 
{

	seti2csdadir(SDAOUT);
	delayus(1);
	fpga_sda_dir(ARM2FPGASDA);
	delayus(1);
}
*/

//��ʱ����
void delay_nus(uint16_t time)
{
	uint16_t j;
	while(time--)
	{
		j=30;  //17
		while(j--);
	}
}

/******************************************************************************
* Function --> IIC������ʼ�ź�
* Param    --> none            
* Reaturn  --> none
* Brief    --> none
******************************************************************************/
//����IIC��ʼ�ź�
void IIC_Start(void)
{
	SDA_OUT();     //sda�����
 	delay_nus(5);	
	seti2csck(GPIO_H);//IIC_SCL=1;
	seti2csda(GPIO_H);//IIC_SDA=1;
	delay_nus(30);
 	seti2csda(GPIO_L);//IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_nus(30);
	seti2csck(GPIO_L);//IIC_SCL=0;//ǯסI2C���ߣ�׼�����ͻ�������� 
	delay_nus(30);
}

/******************************************************************************
* Function --> IIC����ֹͣ�ź�
* Param    --> none            
* Reaturn  --> none
* Brief    --> none
******************************************************************************/
void IIC_Stop(void)
{
	SDA_OUT();//sda�����
	delay_nus(5);//1.20
	seti2csda(GPIO_L);//IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_nus(30);
	seti2csck(GPIO_H);//IIC_SCL=1; 
	delay_nus(30);
	seti2csda(GPIO_H);//IIC_SDA=1;//����I2C���߽����ź�
	delay_nus(30);				
}

/******************************************************************************
* Function --> IIC�ȴ�Ӧ���źŵ���
* Param    --> none            
* Reaturn  --> 1������Ӧ��ʧ��  0������Ӧ��ɹ�
* Brief    --> none
******************************************************************************/
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
   SDA_IN();      //SDA����Ϊ����  
	delay_nus(5);	   
	seti2csck(GPIO_H);//IIC_SCL=1;
	delay_nus(30);
	ucErrTime=0;
	while(read_armi2c_sda())//while(READ_SDA)
	{
		
		delay_nus(300);
		ucErrTime++;
		if(ucErrTime>250)
		{
			//printf("IIC_Wait_Ack not ok\n");
			return 1;
		}
	}
	seti2csck(GPIO_L);//IIC_SCL=0;//ʱ�����0 	   
	//printf("IIC_Wait_Ack ok\n");
	return 0;  
} 

/******************************************************************************
* Function --> IIC����Ӧ��
* Param    --> none            
* Reaturn  --> none
* Brief    --> none
******************************************************************************/
//����ACKӦ��
void IIC_Ack(void)
{
	seti2csck(GPIO_L);//IIC_SCL=0;
	delay_nus(30);
	SDA_OUT();
	delay_nus(5);
	seti2csda(GPIO_L);//IIC_SDA=0;
	delay_nus(30);
	seti2csck(GPIO_H);//IIC_SCL=1;
	delay_nus(30);
	seti2csck(GPIO_L);//IIC_SCL=0;
	delay_nus(30);
}

/******************************************************************************
* Function --> IIC������Ӧ��
* Param    --> none            
* Reaturn  --> none
* Brief    --> none
******************************************************************************/	
//������ACKӦ��		    
void IIC_NAck(void)
{
	seti2csck(GPIO_L);//IIC_SCL=0;
	delay_nus(30);
	SDA_OUT();
	delay_nus(5);
	seti2csda(GPIO_H);//IIC_SDA=1;
	delay_nus(30);
	seti2csck(GPIO_H);//IIC_SCL=1;
	delay_nus(30);
	seti2csck(GPIO_L);//IIC_SCL=0;
	delay_nus(30);
}

/******************************************************************************
* Function --> IIC����һ���ֽں���
* Param    --> �ӵ�ַ             
* Reaturn  --> ��ȡӦ�� 
* Brief    --> ���شӻ�����Ӧ�� 1����Ӧ�� 0����Ӧ��
******************************************************************************/	  
u8 IIC_Write_Byte(u8 txd)
{                        
   u8 i;   
	SDA_OUT(); 	
	delay_nus(5); 
	seti2csck(GPIO_L);//SCL_L;
	delay_nus(5);
   for(i=0;i<8;i++)
    {
      if((txd & 0x80) == 0x80)
        seti2csda(GPIO_H);//SDA_H;  
      else 
        seti2csda(GPIO_L);//SDA_L;   
		delay_nus(10);
      txd = txd<<1;
      seti2csck(GPIO_H);//SCL_H;
      delay_nus(30);
		seti2csck(GPIO_L);//SCL_L;
		delay_nus(20);
    }
	seti2csda(GPIO_H);//SDA_H; 
 	delay_nus(30);

	return 0;
} 	

/******************************************************************************
* Function --> IIC��ȡһ���ֽں���
* Param    --> none             
* Reaturn  --> none 
* Brief    --> ��ȡ�����ֽ�
******************************************************************************/
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
u8 IIC_Read_Byte(void)
{
   u8 i=8;
   u8 ReceiveByte=0;
	SDA_IN();//SDA����Ϊ����
	delay_nus(5);
   ReceiveByte=0;
   while(i--)
    {
        ReceiveByte<<=1;      
        seti2csck(GPIO_H);//SCL_H;
        delay_nus(30);
        if(read_armi2c_sda())
          ReceiveByte=ReceiveByte|0x01;
         seti2csck(GPIO_L);//SCL_L;
         delay_nus(30);   
    }
   return ReceiveByte;
}


/******************************************************************************
* Function --> RX8010ĳ�Ĵ���д��һ���ֽ�����
* Param    --> REG_ADD��Ҫ�����Ĵ�����ַ
*              dat��    Ҫд�������
* Reaturn  --> none 
* Brief    --> none
******************************************************************************/
void RX8010_Write_Byte(u8 REG_ADD,u8 dat)
{
	IIC_Start();
	IIC_Write_Byte(RX8010_Write);
	if(IIC_Wait_Ack()){IIC_Stop(); return;}
	IIC_Write_Byte(REG_ADD);
	IIC_Wait_Ack();
	IIC_Write_Byte(dat);	
	IIC_Wait_Ack();
	IIC_Stop();
}  

/******************************************************************************
* Function --> RX8010ĳ�Ĵ�����ȡһ���ֽ�����
* Param    --> REG_ADD��Ҫ�����Ĵ�����ַ
* Reaturn  --> ��ȡ�õ��ļĴ�����ֵ
* Brief    --> none
******************************************************************************/
u8 RX8010_Read_Byte(u8 REG_ADD)
{
	u8 ReData = 0;
	IIC_Start();
	IIC_Write_Byte(RX8010_Write);	//����д������Ӧ��λ
	if(IIC_Wait_Ack()){IIC_Stop(); return;}
	IIC_Write_Byte(REG_ADD);			//ȷ��Ҫ�����ļĴ���
	IIC_Wait_Ack();
	IIC_Start();							//��������
	IIC_Write_Byte(RX8010_Read);	//���Ͷ�ȡ����
	IIC_Wait_Ack();
	ReData = IIC_Read_Byte();		//��ȡ����
	IIC_NAck();
	IIC_Stop();
	return ReData;
}

/******************************************************************************
* Function --> RX8010��ʼ������
* Param    --> none             
* Reaturn  --> none 
* Brief    --> none
******************************************************************************/
void rx8010_init(void)
{
	RX8010_Write_Byte(0x17,0xd8);
	delay_nus(1000);
	RX8010_Write_Byte(0x30,0x00);
	delay_nus(1000);
	RX8010_Write_Byte(0x31,0x08);
	delay_nus(1000);
	RX8010_Write_Byte(0x32,0x00);
	delay_nus(1000);
	RX8010_Write_Byte(0x1d,0x04);
	delay_nus(1000);
	RX8010_Write_Byte(0x1e,0x00);
	delay_nus(1000);
	RX8010_Write_Byte(0x1f,0x00);
	delay_nus(1000);
}

/******************************************************************************
* Function --> RX8010д���������
* Param    --> REG_ADD��Ҫ�����Ĵ�����ַ
               num��д����������
               *pBuff��д�����ݻ���
* Reaturn  --> none
* Brief    --> none
******************************************************************************/
void RX8010_Write_nByte(u8 REG_ADD, u8 num, u8 *pBuff)
{
    u8 i = 0;
             
    IIC_Start();
    IIC_Write_Byte(RX8010_Write);  //����д������Ӧ��λ
	 if(IIC_Wait_Ack()){IIC_Stop(); return;}
    IIC_Write_Byte(REG_ADD);  //��λ��ʼ�Ĵ�����ַ
	 IIC_Wait_Ack();
    for(i = 0;i < num;i++)
        {
          IIC_Write_Byte(*pBuff);  //д������
          pBuff++;
			IIC_Wait_Ack();
        }
    IIC_Stop();
}

/******************************************************************************
* Function --> RX8010����Ƿ����
* Param    --> none
* Reaturn  --> 0������
*              1��RX8010���������
* Brief    --> ��ʱ������ʱ�Ĵ���д��һ����ֵ�ٶ�ȡ�������Աȣ���ͬ��ȷ����ͬ�����
******************************************************************************/
u8 test_value=0;
u8 TCount=0;
u8 RX8010_Check(void)
{
	//u8 test_value=0;
//	u8 Time_Count=0;	//��ʱ������ʱ���ݻ���
//	if(RX8010_Read_Byte(Address_Time) & 0x80)	//������˶�ʱ�������ȹر�
//	{
//		RX8010_Write_Byte(Address_Time, Time_Close);	//�ȹرն�ʱ��
//		Time_Count = RX8010_Read_Byte(Address_Time_VAL);	//�ȱ������ֵ
//	}

	RX8010_Write_Byte(Address_Time_VAL, RX8010_Check_Data);	//д����ֵ
	for(test_value = 0;test_value < 250;test_value++)	{}	//��ʱһ��ʱ���ٶ�ȡ
	test_value = RX8010_Read_Byte(Address_Time_VAL);	//�ٶ�ȡ����

//	if(Time_Count != 0)	//�����˶�ʱ�����ܣ���ָ�
//	{
//		RX8010_Write_Byte(Address_Time_VAL, Time_Count);	//�ָ��ֳ�
//		RX8010_Write_Byte(Address_Time, Time_Open);	//������ʱ��
//	}

	if(test_value != RX8010_Check_Data)	return 1;	//�������������
	
	return 0;	//����
}

/******************************************************************************
* Function --> RX8010��ʱ�������Ĵ���������д�����ݻ��߶�ȡ����
* Param    --> REG_ADD��Ҫ�����Ĵ�����ʼ��ַ
*              *WBuff�� д�����ݻ���
*              num��    д����������
*              mode��   ����ģʽ��0��д�����ݲ�����1����ȡ���ݲ���
* Reaturn  --> none
* Brief    --> ����д��n�ֽڻ���������ȡn�ֽ�����
******************************************************************************/
void RX8010_Operate_Register(u8 REG_ADD,u8 *pBuff,u8 num,u8 mode)
{
	u8 i;
	if(mode)	//��ȡ����
	{
		IIC_Start();
		IIC_Write_Byte(RX8010_Write);	//����д������Ӧ��λ
		if(IIC_Wait_Ack()){IIC_Stop(); return;}
		IIC_Write_Byte(REG_ADD);	//��λ��ʼ�Ĵ�����ַ
		IIC_Wait_Ack();
		IIC_Start();	//��������
		IIC_Write_Byte(RX8010_Read);	//���Ͷ�ȡ����
		IIC_Wait_Ack();
		for(i = 0;i < num;i++)
		{
			*pBuff = IIC_Read_Byte();		//��ȡ����
			if(i == (num -1))	IIC_NAck();	//IIC_Ack(1);	//���ͷ�Ӧ���ź�
			else IIC_Ack();						//IIC_Ack(0);	//����Ӧ���ź�
			pBuff++;
		}
		IIC_Stop();	
	}
	else	//д������
	{		 	
		RX8010_Write_nByte(REG_ADD,num, pBuff);
	}
}
/******************************************************************************
* Function --> RX8010��ȡ����д��ʱ����Ϣ
* Param    --> *pBuff��д�����ݻ���
*              mode������ģʽ��0��д�����ݲ�����1����ȡ���ݲ���
* Reaturn  --> none
* Brief    --> ����д��n�ֽڻ���������ȡn�ֽ�����
******************************************************************************/
u8 Time_Reg[8];
void RX8010_ReadWrite_Time(u8 mode)
{
	//u8 Time_Reg[8];	//����ʱ�仺��
	
	if(mode)	//��ȡʱ����Ϣ
	{
		RX8010_Operate_Register(Sec,Time_Reg,7,1);	//�����ַ��0x10����ʼ��ȡʱ����������
		
		/******�����ݸ��Ƶ�ʱ��ṹ���У��������������******/
		//�����¶�ȡ��ʽҲ����
//		TimeValue.second = RX8010_Read_Byte(Sec) & Shield_secondBit;	//������
//		TimeValue.minute = RX8010_Read_Byte(Min) & Shield_minuteBit;	//��������
//		TimeValue.hour   = RX8010_Read_Byte(Hour) & Shield_hourBit;	//Сʱ����
//		TimeValue.week   = RX8010_Read_Byte(Week) & Shield_weekBit;	//������
//		TimeValue.date   = RX8010_Read_Byte(Day) & Shield_dateBit;	//��������
//		TimeValue.month  = RX8010_Read_Byte(Month) & Shield_monthBit;	//������
//		TimeValue.year   = RX8010_Read_Byte(Years) | RX8010_YEARDATA;	//������
		
		TimeValue.second = Time_Reg[0] & Shield_secondBit;	//������
		TimeValue.minute = Time_Reg[1] & Shield_minuteBit;	//��������
		TimeValue.hour   = Time_Reg[2] & Shield_hourBit;	//Сʱ����
		TimeValue.week   = Time_Reg[3] & Shield_weekBit;	//������
		TimeValue.date   = Time_Reg[4] & Shield_dateBit;	//��������
		TimeValue.month  = Time_Reg[5] & Shield_monthBit ;	//������
		TimeValue.year   = Time_Reg[6] | RX8010_YEARDATA;	//������
		
		printf("second=%x\n",TimeValue.second );
		printf("minute=%x\n",TimeValue.minute );
		printf("hour=%x\n",TimeValue.hour );
		printf("week=%x\n",TimeValue.week );
		printf("date=%x\n",TimeValue.date );
		printf("month=%x\n",TimeValue.month );
		printf("year=%x\n",TimeValue.year );
		
	}
	else
	{
		if(TimeValue.week == 0x07)	TimeValue.week = 0x00;	//�Ĵ�����ֵΪ0 ~ 6

		/******��ʱ��ṹ���и������ݽ���******/		
		Time_Reg[0] = TimeValue.second ;	//�룬��֤׼ȷʱ��
		Time_Reg[1] = TimeValue.minute;						//����
		Time_Reg[2] = TimeValue.hour;							//Сʱ
		Time_Reg[3] = TimeValue.week;							//��
		Time_Reg[4] = TimeValue.date;							//����
		Time_Reg[5] = TimeValue.month | (0<<7);				//�£�����������λ20xx��
		Time_Reg[6] = (u8)TimeValue.year;						//��
		
		RX8010_Operate_Register(Sec,Time_Reg,7,0);	//�����ַ��0x10����ʼд��ʱ����������
	}
}
/******************************************************************************
* Function --> ʱ��������ʼ��
* Param    --> *TimeVAL��RTCоƬ�Ĵ���ֵָ��
* Reaturn  --> none
* Brief    --> none
******************************************************************************/
void RX8010_Time_Init(Time_Typedef *TimeVAL)
{
	if(TimeVAL->week == 0x07)	TimeVAL->week = 0x00;	//�Ĵ�����ֵΪ0 ~ 6
	
	//ʱ����������
//	Time_Buffer[0]  = TimeVAL->second | Accuracy_Clock_Yes;	//��֤׼ȷʱ��
	Time_Buffer[0]  = TimeVAL->second ;	//��֤׼ȷʱ��
	Time_Buffer[1]  = TimeVAL->minute;
	Time_Buffer[2]  = TimeVAL->hour;
	Time_Buffer[3]  = TimeVAL->week;
	Time_Buffer[4]  = TimeVAL->date;
	Time_Buffer[5]  = TimeVAL->month | Set_Year20xx ;	//����Ϊ20xx��
	Time_Buffer[6]  = (u8)TimeVAL->year;
	//���屨��
//	Time_Buffer[9]  = TimeVAL->alarmmin | Alarm_minute_Close;	//���ӱ���
//	Time_Buffer[10] = TimeVAL->alarmhour | Alarm_hour_Close;	//Сʱ����
//	Time_Buffer[11] = TimeVAL->alarmdate | Alarm_date_Close;	//�ձ���
//	Time_Buffer[12] = TimeVAL->alarmweek | Alarm_week_Close;	//���ڱ���
	//Ƶ��������ã���ʱ������
//	Time_Buffer[13] = TimeVAL->CLKcont;	//Ƶ���������
//	Time_Buffer[14] = TimeVAL->timecont;	//��ʱ������
//	Time_Buffer[15] = TimeVAL->timeconut;	//��ʱ������ʱ
	
	RX8010_Operate_Register(Sec,Time_Buffer,7,0);	//�ӿ���/״̬�Ĵ���1��0x10����ʼд��16������
}
/******************************************************************************
* Function --> ʱ���������ݴ�����
* Param    --> none
* Reaturn  --> none
* Brief    --> ����ȡ����ʱ��������Ϣת����ASCII�󱣴浽ʱ���ʽ������
******************************************************************************/
/*
void RX8010_Time_Handle(void)
{
	//////////////////////////////////////////////////////////
	 ///                  ��ȡʱ��������Ϣ
	/////////////////////////////////////////////////////////
	
	RX8010_ReadWrite_Time(1);	//��ȡʱ����������
	
	/////////////////////////////////////////////////////////////////
	//            ʱ����Ϣת��ΪASCII������ַ�
	////////////////////////////////////////////////////////////////
	Display_Time[6] = (TimeValue.second >> 4) + 0x30;
	Display_Time[7] = (TimeValue.second & 0x0f) + 0x30;	//Second

	Display_Time[3] = (TimeValue.minute >> 4) + 0x30;
	Display_Time[4] = (TimeValue.minute & 0x0f) + 0x30;	//Minute

	Display_Time[0] = (TimeValue.hour >> 4) + 0x30;
	Display_Time[1] = (TimeValue.hour & 0x0f) + 0x30;	//Hour 

	Display_Date[8] = (TimeValue.date >> 4) + 0x30;
	Display_Date[9] = (TimeValue.date & 0x0f) + 0x30;	//Date

	Display_Date[5] = (TimeValue.month >> 4) + 0x30;
	Display_Date[6] = (TimeValue.month & 0x0f) + 0x30;	//Month

	Display_Date[0] = (u8)(TimeValue.year >> 12) + 0x30;	//2
	Display_Date[1] = (u8)((TimeValue.year & 0x0f00) >> 8) + 0x30;	//0
	Display_Date[2] = (TimeValue.year >> 4) + 0x30;
	Display_Date[3] = (TimeValue.year & 0x0f) + 0x30;	//Year

	/////////////////////////////////////////////////////////
	   ��������ʾ����7�������ľ��Ƕ�Ӧ��������ʾ����0ת��Ϊ7
	//////////////////////////////////////////////////////
	
	if(TimeValue.week == 0x00)
	{	Display_Date[11] = 0x37;	}	//week
	else
	{	Display_Date[11] = (TimeValue.week & 0x0f) + 0x30;	}
	
}
*/
/******************************************************************************
* Function --> ʱ�����ڳ�ʼ������
* Param    --> none
* Reaturn  --> none
* Brief    --> none
******************************************************************************/
void RX8010_RealTimeInit(void)
{
	TimeDefault.second = 	0x00;
	TimeDefault.minute = 	0x01;
	TimeDefault.hour = 	0x01;
	TimeDefault.week = 	0x04;
	TimeDefault.date = 	0x14;
	TimeDefault.month = 	0x05;
	TimeDefault.year = 	0x20;
	RX8010_Time_Init(&TimeDefault);	
}



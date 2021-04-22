#ifndef _RX8010SJ_H
#define _RX8010SJ_H
typedef unsigned char 	u8;
typedef unsigned short 	u16;
typedef struct
{
	u8 year;	//��
	u8 month;	//��
	u8 date;	//����
	u8 week;	//��
	u8 hour;	//Сʱ
	u8 minute;	//����
	u8 second;	//����
}Time_Typedef;
extern Time_Typedef TimeValue;	
extern unsigned char Time_Register[16];	
extern unsigned char Display_Time[8];	
extern unsigned char Display_Date[13];	
#define BCD_TO_HEX(bcd) ((((bcd)>>4)*10)+((bcd)&0x0F)) 
#define HEX_TO_BCD(hex) ((((hex)/10)<<4)+((hex)%10))
//#define IIC_SCL    PBout(6) //SCL
//#define IIC_SDA    PBout(7) //SDA	 
//#define READ_SDA   GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7)//PBin(7)  
#define RX8010_Check_Data		0xaa			
#define RX8010_YEARDATA			(u16)0x2000	
#define RX8010_Write				0x64	
#define RX8010_Read				0x65	
#define Years  						0x16
#define Month  						0x15
#define Day    						0x14
#define Week  						0x13
#define Hour  						0x12
#define Min  						0x11
#define Sec  						0x10
#define Address_Time_VAL 		0x1A
#define Control_V 					0x1d
#define Shield_secondBit			0x7f
#define Shield_minuteBit			0x7f
#define Shield_hourBit			0x3f
#define Shield_weekBit			0x07
#define Shield_dateBit			0x3f
#define Shield_monthBit			0x1f
#define Set_Year19xx				(1<<7)	
#define Set_Year20xx	  	    	(0<<7)	
#define Accuracy_Clock_Yes		(0<<7)	
#define Accuracy_Clock_No		(1<<7)	
//RX8010��������
void RX8010_Write_Byte(u8 REG_ADD,u8 dat);
u8 RX8010_Read_Byte(u8 REG_ADD);
u8 BCD_To_Decimal(u8 source_char);
void RX8010_Write_nByte(u8 REG_ADD, u8 num, u8 *pBuff);
u8 RX8010_Check(void);
void RX8010_Operate_Register(u8 REG_ADD,u8 *pBuff,u8 num,u8 mode);
void RX8010_ReadWrite_Time(u8 mode);
void RX8010_Time_Init(Time_Typedef *TimeVAL);
void RX8010_Time_Handle(void);
void RX8010_RealTimeInit(void);
void rx8010_init(void); 
//IIC���в�������
void IIC_Init(void);                	//��ʼ��IIC��IO��				 
void IIC_Start(void);						//����IIC��ʼ�ź�
void IIC_Stop(void);	  					//����IICֹͣ�ź�
u8 IIC_Write_Byte(u8 txd);				//IIC����һ���ֽ�
////void IIC_Write_Byte(u8 txd);
u8 IIC_Read_Byte(void);//IIC��ȡһ���ֽ�
//u8 IIC_Read_Byte(unsigned char ack) ;
u8 IIC_Wait_Ack(void); 					//IIC�ȴ�ACK�ź�
void IIC_Ack(void);						//IIC����ACK�ź�
void IIC_NAck(void);						//IIC������ACK�ź�
void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);
#endif

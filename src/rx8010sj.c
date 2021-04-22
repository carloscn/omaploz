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
                               定义显示时间格式
                        要改变显示的格式请修改此数组
******************************************************************************/

u8 Display_Time[8] = {0x30,0x30,0x3a,0x30,0x30,0x3a,0x30,0x30};	
					//时间显示缓存   格式  00:00:00

u8 Display_Date[13] = {0x32,0x30,0x31,0x37,0x2f,0x30,0x31,0x2f,0x32,0x30,0x20,0x37,0x57};
					//日期显示缓存   格式  2013/10/20 7W

/******************************************************************************
                            定义相关的变量函数
******************************************************************************/

Time_Typedef TimeValue,TimeDefault;	//定义时间数据指针
//tm timer;
u8 Time_Buffer[8];	//时间日历数据缓存
//u8 Time_Buffer1[16];
unsigned char buffer[4];

/******************************************************************************
                            IIC驱动函数
******************************************************************************/
//初始化IIC
void IIC_Init(void)
{					     
	;
}


//设置SDA为输入模式
void SDA_IN(void) 
{
	seti2csdadir(SDAIN);
	delayus(1);
	fpga_sda_dir(FPGASDA2ARM);
	delayus(1);
}

//设置SDA为输出模式
void SDA_OUT(void) 
{
	fpga_sda_dir(ARM2FPGASDA);
	delayus(1);
	seti2csdadir(SDAOUT);
	delayus(1);
}

/*
//设置SDA为输入模式
void SDA_IN(void) 
{

	fpga_sda_dir(FPGASDA2ARM);
	delayus(1);
	seti2csdadir(SDAIN);
	delayus(1);
}

//设置SDA为输出模式
void SDA_OUT(void) 
{

	seti2csdadir(SDAOUT);
	delayus(1);
	fpga_sda_dir(ARM2FPGASDA);
	delayus(1);
}
*/

//延时函数
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
* Function --> IIC产生起始信号
* Param    --> none            
* Reaturn  --> none
* Brief    --> none
******************************************************************************/
//产生IIC起始信号
void IIC_Start(void)
{
	SDA_OUT();     //sda线输出
 	delay_nus(5);	
	seti2csck(GPIO_H);//IIC_SCL=1;
	seti2csda(GPIO_H);//IIC_SDA=1;
	delay_nus(30);
 	seti2csda(GPIO_L);//IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_nus(30);
	seti2csck(GPIO_L);//IIC_SCL=0;//钳住I2C总线，准备发送或接收数据 
	delay_nus(30);
}

/******************************************************************************
* Function --> IIC产生停止信号
* Param    --> none            
* Reaturn  --> none
* Brief    --> none
******************************************************************************/
void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	delay_nus(5);//1.20
	seti2csda(GPIO_L);//IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_nus(30);
	seti2csck(GPIO_H);//IIC_SCL=1; 
	delay_nus(30);
	seti2csda(GPIO_H);//IIC_SDA=1;//发送I2C总线结束信号
	delay_nus(30);				
}

/******************************************************************************
* Function --> IIC等待应答信号到来
* Param    --> none            
* Reaturn  --> 1，接收应答失败  0，接收应答成功
* Brief    --> none
******************************************************************************/
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
   SDA_IN();      //SDA设置为输入  
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
	seti2csck(GPIO_L);//IIC_SCL=0;//时钟输出0 	   
	//printf("IIC_Wait_Ack ok\n");
	return 0;  
} 

/******************************************************************************
* Function --> IIC产生应答
* Param    --> none            
* Reaturn  --> none
* Brief    --> none
******************************************************************************/
//产生ACK应答
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
* Function --> IIC不产生应答
* Param    --> none            
* Reaturn  --> none
* Brief    --> none
******************************************************************************/	
//不产生ACK应答		    
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
* Function --> IIC发送一个字节函数
* Param    --> 从地址             
* Reaturn  --> 获取应答 
* Brief    --> 返回从机有无应答 1，有应答 0，无应答
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
* Function --> IIC读取一个字节函数
* Param    --> none             
* Reaturn  --> none 
* Brief    --> 读取到的字节
******************************************************************************/
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 IIC_Read_Byte(void)
{
   u8 i=8;
   u8 ReceiveByte=0;
	SDA_IN();//SDA设置为输入
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
* Function --> RX8010某寄存器写入一个字节数据
* Param    --> REG_ADD：要操作寄存器地址
*              dat：    要写入的数据
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
* Function --> RX8010某寄存器读取一个字节数据
* Param    --> REG_ADD：要操作寄存器地址
* Reaturn  --> 读取得到的寄存器的值
* Brief    --> none
******************************************************************************/
u8 RX8010_Read_Byte(u8 REG_ADD)
{
	u8 ReData = 0;
	IIC_Start();
	IIC_Write_Byte(RX8010_Write);	//发送写命令并检查应答位
	if(IIC_Wait_Ack()){IIC_Stop(); return;}
	IIC_Write_Byte(REG_ADD);			//确定要操作的寄存器
	IIC_Wait_Ack();
	IIC_Start();							//重启总线
	IIC_Write_Byte(RX8010_Read);	//发送读取命令
	IIC_Wait_Ack();
	ReData = IIC_Read_Byte();		//读取数据
	IIC_NAck();
	IIC_Stop();
	return ReData;
}

/******************************************************************************
* Function --> RX8010初始化函数
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
* Function --> RX8010写入多组数据
* Param    --> REG_ADD：要操作寄存器地址
               num：写入数据数量
               *pBuff：写入数据缓存
* Reaturn  --> none
* Brief    --> none
******************************************************************************/
void RX8010_Write_nByte(u8 REG_ADD, u8 num, u8 *pBuff)
{
    u8 i = 0;
             
    IIC_Start();
    IIC_Write_Byte(RX8010_Write);  //发送写命令并检查应答位
	 if(IIC_Wait_Ack()){IIC_Stop(); return;}
    IIC_Write_Byte(REG_ADD);  //定位起始寄存器地址
	 IIC_Wait_Ack();
    for(i = 0;i < num;i++)
        {
          IIC_Write_Byte(*pBuff);  //写入数据
          pBuff++;
			IIC_Wait_Ack();
        }
    IIC_Stop();
}

/******************************************************************************
* Function --> RX8010检测是否存在
* Param    --> none
* Reaturn  --> 0：正常
*              1：RX8010错误或者损坏
* Brief    --> 向定时器倒计时寄存器写入一个数值再读取出来做对比，相同正确，不同则错误
******************************************************************************/
u8 test_value=0;
u8 TCount=0;
u8 RX8010_Check(void)
{
	//u8 test_value=0;
//	u8 Time_Count=0;	//定时器倒计时数据缓存
//	if(RX8010_Read_Byte(Address_Time) & 0x80)	//如果打开了定时器，则先关闭
//	{
//		RX8010_Write_Byte(Address_Time, Time_Close);	//先关闭定时器
//		Time_Count = RX8010_Read_Byte(Address_Time_VAL);	//先保存计数值
//	}

	RX8010_Write_Byte(Address_Time_VAL, RX8010_Check_Data);	//写入检测值
	for(test_value = 0;test_value < 250;test_value++)	{}	//延时一定时间再读取
	test_value = RX8010_Read_Byte(Address_Time_VAL);	//再读取回来

//	if(Time_Count != 0)	//启动了定时器功能，则恢复
//	{
//		RX8010_Write_Byte(Address_Time_VAL, Time_Count);	//恢复现场
//		RX8010_Write_Byte(Address_Time, Time_Open);	//启动定时器
//	}

	if(test_value != RX8010_Check_Data)	return 1;	//器件错误或者损坏
	
	return 0;	//正常
}

/******************************************************************************
* Function --> RX8010对时间日历寄存器操作，写入数据或者读取数据
* Param    --> REG_ADD：要操作寄存器起始地址
*              *WBuff： 写入数据缓存
*              num：    写入数据数量
*              mode：   操作模式。0：写入数据操作。1：读取数据操作
* Reaturn  --> none
* Brief    --> 连续写入n字节或者连续读取n字节数据
******************************************************************************/
void RX8010_Operate_Register(u8 REG_ADD,u8 *pBuff,u8 num,u8 mode)
{
	u8 i;
	if(mode)	//读取数据
	{
		IIC_Start();
		IIC_Write_Byte(RX8010_Write);	//发送写命令并检查应答位
		if(IIC_Wait_Ack()){IIC_Stop(); return;}
		IIC_Write_Byte(REG_ADD);	//定位起始寄存器地址
		IIC_Wait_Ack();
		IIC_Start();	//重启总线
		IIC_Write_Byte(RX8010_Read);	//发送读取命令
		IIC_Wait_Ack();
		for(i = 0;i < num;i++)
		{
			*pBuff = IIC_Read_Byte();		//读取数据
			if(i == (num -1))	IIC_NAck();	//IIC_Ack(1);	//发送非应答信号
			else IIC_Ack();						//IIC_Ack(0);	//发送应答信号
			pBuff++;
		}
		IIC_Stop();	
	}
	else	//写入数据
	{		 	
		RX8010_Write_nByte(REG_ADD,num, pBuff);
	}
}
/******************************************************************************
* Function --> RX8010读取或者写入时间信息
* Param    --> *pBuff：写入数据缓存
*              mode：操作模式。0：写入数据操作。1：读取数据操作
* Reaturn  --> none
* Brief    --> 连续写入n字节或者连续读取n字节数据
******************************************************************************/
u8 Time_Reg[8];
void RX8010_ReadWrite_Time(u8 mode)
{
	//u8 Time_Reg[8];	//定义时间缓存
	
	if(mode)	//读取时间信息
	{
		RX8010_Operate_Register(Sec,Time_Reg,7,1);	//从秒地址（0x10）开始读取时间日历数据
		
		/******将数据复制到时间结构体中，方便后面程序调用******/
		//用以下读取方式也可以
//		TimeValue.second = RX8010_Read_Byte(Sec) & Shield_secondBit;	//秒数据
//		TimeValue.minute = RX8010_Read_Byte(Min) & Shield_minuteBit;	//分钟数据
//		TimeValue.hour   = RX8010_Read_Byte(Hour) & Shield_hourBit;	//小时数据
//		TimeValue.week   = RX8010_Read_Byte(Week) & Shield_weekBit;	//日数据
//		TimeValue.date   = RX8010_Read_Byte(Day) & Shield_dateBit;	//星期数据
//		TimeValue.month  = RX8010_Read_Byte(Month) & Shield_monthBit;	//月数据
//		TimeValue.year   = RX8010_Read_Byte(Years) | RX8010_YEARDATA;	//年数据
		
		TimeValue.second = Time_Reg[0] & Shield_secondBit;	//秒数据
		TimeValue.minute = Time_Reg[1] & Shield_minuteBit;	//分钟数据
		TimeValue.hour   = Time_Reg[2] & Shield_hourBit;	//小时数据
		TimeValue.week   = Time_Reg[3] & Shield_weekBit;	//日数据
		TimeValue.date   = Time_Reg[4] & Shield_dateBit;	//星期数据
		TimeValue.month  = Time_Reg[5] & Shield_monthBit ;	//月数据
		TimeValue.year   = Time_Reg[6] | RX8010_YEARDATA;	//年数据
		
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
		if(TimeValue.week == 0x07)	TimeValue.week = 0x00;	//寄存器数值为0 ~ 6

		/******从时间结构体中复制数据进来******/		
		Time_Reg[0] = TimeValue.second ;	//秒，保证准确时间
		Time_Reg[1] = TimeValue.minute;						//分钟
		Time_Reg[2] = TimeValue.hour;							//小时
		Time_Reg[3] = TimeValue.week;							//日
		Time_Reg[4] = TimeValue.date;							//星期
		Time_Reg[5] = TimeValue.month | (0<<7);				//月，设置年数据位20xx年
		Time_Reg[6] = (u8)TimeValue.year;						//年
		
		RX8010_Operate_Register(Sec,Time_Reg,7,0);	//从秒地址（0x10）开始写入时间日历数据
	}
}
/******************************************************************************
* Function --> 时间日历初始化
* Param    --> *TimeVAL：RTC芯片寄存器值指针
* Reaturn  --> none
* Brief    --> none
******************************************************************************/
void RX8010_Time_Init(Time_Typedef *TimeVAL)
{
	if(TimeVAL->week == 0x07)	TimeVAL->week = 0x00;	//寄存器数值为0 ~ 6
	
	//时间日历数据
//	Time_Buffer[0]  = TimeVAL->second | Accuracy_Clock_Yes;	//保证准确时间
	Time_Buffer[0]  = TimeVAL->second ;	//保证准确时间
	Time_Buffer[1]  = TimeVAL->minute;
	Time_Buffer[2]  = TimeVAL->hour;
	Time_Buffer[3]  = TimeVAL->week;
	Time_Buffer[4]  = TimeVAL->date;
	Time_Buffer[5]  = TimeVAL->month | Set_Year20xx ;	//设置为20xx年
	Time_Buffer[6]  = (u8)TimeVAL->year;
	//闹铃报警
//	Time_Buffer[9]  = TimeVAL->alarmmin | Alarm_minute_Close;	//分钟报警
//	Time_Buffer[10] = TimeVAL->alarmhour | Alarm_hour_Close;	//小时报警
//	Time_Buffer[11] = TimeVAL->alarmdate | Alarm_date_Close;	//日报警
//	Time_Buffer[12] = TimeVAL->alarmweek | Alarm_week_Close;	//星期报警
	//频率输出设置；定时器设置
//	Time_Buffer[13] = TimeVAL->CLKcont;	//频率输出控制
//	Time_Buffer[14] = TimeVAL->timecont;	//定时器控制
//	Time_Buffer[15] = TimeVAL->timeconut;	//定时器倒计时
	
	RX8010_Operate_Register(Sec,Time_Buffer,7,0);	//从控制/状态寄存器1（0x10）开始写入16组数据
}
/******************************************************************************
* Function --> 时间日历数据处理函数
* Param    --> none
* Reaturn  --> none
* Brief    --> 将读取到的时间日期信息转换成ASCII后保存到时间格式数组中
******************************************************************************/
/*
void RX8010_Time_Handle(void)
{
	//////////////////////////////////////////////////////////
	 ///                  读取时间日期信息
	/////////////////////////////////////////////////////////
	
	RX8010_ReadWrite_Time(1);	//获取时间日历数据
	
	/////////////////////////////////////////////////////////////////
	//            时间信息转换为ASCII码可视字符
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
	   星期日显示的是7，其他的就是对应的数字显示，将0转换为7
	//////////////////////////////////////////////////////
	
	if(TimeValue.week == 0x00)
	{	Display_Date[11] = 0x37;	}	//week
	else
	{	Display_Date[11] = (TimeValue.week & 0x0f) + 0x30;	}
	
}
*/
/******************************************************************************
* Function --> 时间日期初始化函数
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



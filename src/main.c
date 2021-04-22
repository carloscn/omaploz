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
void defparathreadfun(void);
void emif_read(void);
void Get_filecreat_Time(void);
void wrsd_threadfun(void);
void pinmux_init(void);
void emif_ce2init(void);
void gpiodir_init(void);
#define SERVER_PORT 		5000			
#define BACKLOG 			5
///////////////////////////////////////////////////////////////////////////////////////
#define 			PINMUX_SIZE           	0x4
#define 			PINMUX5_BASE	   				0x01C14134
#define 			GPIO_SIZE           		0xD4 
#define 			GPIO_BASE	   					0x01E26000 
#define 			EMIF_SIZE           		0x10          
#define 			EMIF_BASE	   					0x60000000 
#define 			CECFG_SIZE           		0x10
#define 			CE2CFG_BASE	   				0x68000010 
void 		*emif_map_base		=NULL;
void 		*emif_virt_addr		=NULL;
size_t 	emif_length			=0;
off_t 		emif_phy_addr		=0;
void 		*pinmux_map_base	=NULL;
void 		*pinmux_virt_addr	=NULL;
size_t 	pinmux_length		=0;
off_t 		pinmux_phy_addr		=0;
void 		*gpio_map_base		=NULL;
void 		*gpio_virt_addr		=NULL;
size_t 	gpio_length			=0;
off_t 		gpio_phy_addr		=0;
void 		*cecfg_map_base		=NULL;
void 		*cecfg_virt_addr	=NULL;
size_t 	cecfg_length			=0;
off_t 		cecfg_phy_addr		=0;
uint32_t * gpio2_datareg;
uint32_t * gpio2_dirreg;
uint32_t * gpio2_indatareg;

struct 	sockaddr_in local;
int 		s_s;
int 		s_c;
char	   ipaddr[40];
char     	gateway[40];
char     	netmask[40];
int			modbustcpport;
int			hsport;
int    	ethfd;
int    	defparafd;
int 		memfd=0;
int      	socketestablished;
int			fd_wrsd;
#define BUFSIZE  	1024*16
#define SDBUFSIZE  1024*32

FILE *rec_fp;
volatile unsigned int	pcnt;
volatile uint16_t 		*emifinout;
volatile uint8_t 		startcov=0;
volatile uint16_t 		Frecnt=0;


off_t 			length;
uint16_t 		*addr;
int 			filefd;
char 			filename[48]="/mnt/mmcblk0p1/";
char    		filename_time[24];
uint8_t  		prostate;

int main(int argc, char **argv)
{
	int				ret=0;
	int 			i, j, size;
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	socketestablished =0;
	Frecnt=200;
	///////////////////////////////////////////////////////////////////////////////////////
	memset(ipaddr, 0, sizeof(ipaddr));
	memset(gateway, 0, sizeof(gateway));
	memset(netmask, 0, sizeof(netmask));
	memset(configfilepath, 0, sizeof(configfilepath));
	strcpy(configfilepath, "/usr/configinfo.txt");
	GetConfigValue(configfilepath);
	SetIfAddr("eth0", ipaddr, netmask, gateway);
	conver_netstr_to_netchar();
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	pinmux_init();
	usleep(100);
	memfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (memfd < 0)
	{
		exit(0);
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	gpio_length 		=	GPIO_SIZE;
	gpio_phy_addr	=	GPIO_BASE;
	if (map_addr(&memfd, gpio_length, gpio_phy_addr, &gpio_map_base, &gpio_virt_addr) == -1)
	{
		exit(0);
	}
	gpio2_dirreg	 		= 	(uint32_t *)(gpio_virt_addr+0x38);
	gpio2_datareg		=	(uint32_t *)(gpio_virt_addr+0x3C);
	gpio2_indatareg 	=	(uint32_t *)(gpio_virt_addr+0x48);
	gpiodir_init();
	i2cgpio_init();
	setad7606rden(GPIO_L);
	emif_ce2init();
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	emif_length 		=	EMIF_SIZE;
	emif_phy_addr	=	EMIF_BASE;
	if (map_addr(&memfd, emif_length, emif_phy_addr, &emif_map_base, &emif_virt_addr) == -1)
	{
		exit(0);
	}
	emifinout = (uint16_t *)emif_virt_addr;
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	s_s = socket(AF_INET, SOCK_STREAM, 0);
	fcntl(s_s, F_SETFL, fcntl(s_s, F_GETFL, 0)|O_NONBLOCK);
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(modbustcpport);
	bind(s_s, (struct sockaddr*)&local, sizeof(local));
	listen(s_s, BACKLOG);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	ret=pthread_create(&ethfd, NULL, eththreadfun, NULL);
	if (ret != 0)
	{
		exit(0);
	}
	fpga_reset();
	usleep(200000);
	setadc_samfre(Frecnt);
	usleep(1000);
	set_adc_range(ADC10V);
	prostate=0;
	pcnt=0;
	sleep(1);
	RX8010_ReadWrite_Time(1);
	u8421second	=TimeValue.second;
	u8421minute	=TimeValue.minute;
	u8421hour		=TimeValue.hour;
	u8421week		=TimeValue.week;
	u8421day		=TimeValue.date;
	u8421month	=TimeValue.month;
	u8421year		=TimeValue.year;
	u8424_to_decimal();
	setlinuxtimechar(u10year, u10month, u10day, u10hour, u10minute, u10second);
	while (1)
	{

		switch (prostate)
		{
		case 1:
			memset(filename, 0, sizeof(filename));
			strcpy(filename, "/mnt/mmcblk0p1/");
			Get_filecreat_Time();
			strcat(filename, filename_time);
			strcat(filename, ".txt");
			filefd= open(filename, O_RDWR|O_CREAT|O_APPEND|O_ASYNC|O_NONBLOCK, 0666);
			if (filefd <0)
			{
				prostate=2;
				break;
			}
			if (ftruncate(filefd, 2097152) == -1)
			{
				prostate=2;
				break;
			}
			addr=(uint16_t *)mmap(NULL, 2097152, PROT_READ | PROT_WRITE, MAP_SHARED, filefd, 0);
			adc_start();
			prostate=3;
			break;
		case 2:
			startcov=0;
			setad7606rden(GPIO_L);
			adc_stop();
			fpga_reset();
			prostate=0;
			break;
		case 3:	if (read_fifoad7606_profull()==1)
		{
			setad7606rden(GPIO_H);
			prostate=4;
		}
			  break;
		case 4:
			while (1)
			{
				*(addr+pcnt)=*emifinout;
				pcnt=pcnt+1;
				if (pcnt >= 1048576)
				{
					close(filefd);
					pcnt=0;
					prostate=5;
					break;
				}
				if (read_fifoad7606_proempty()==1)
				{
					setad7606rden(GPIO_L);
					prostate=3;
					break;
				}
			}
			break;
		case 5:
			seti2csck(GPIO_L);
			memset(filename, 0, sizeof(filename));
			strcpy(filename, "/mnt/mmcblk0p1/");
			Get_filecreat_Time();
			strcat(filename, filename_time);
			strcat(filename, ".txt");
			filefd= open(filename, O_RDWR|O_CREAT|O_APPEND|O_ASYNC|O_NONBLOCK, 0666);
			if (filefd <0)
			{
				prostate=2;
				break;
			}
			if (ftruncate(filefd, 2097152) == -1)
			{
				prostate=2;
				break;
			}
			addr=(uint16_t *)mmap(NULL, 2097152, PROT_READ | PROT_WRITE, MAP_SHARED, filefd, 0);
			prostate=3;
			seti2csck(GPIO_H);
			break;
		case 0:
			if (startcov == 1)
			{
				pcnt=0;
				prostate=1;
			}
			break;
		}

		if ((prostate!=0) && (startcov==0))
		{
			prostate=2;
		}
	}
	pthread_join(ethfd, NULL);
	munmap(emif_map_base, emif_length);
	munmap(pinmux_map_base, pinmux_length);
	munmap(gpio_map_base, gpio_length);
	munmap(cecfg_map_base, cecfg_length);
	close(memfd);
	return 0;
}


void pinmux_init(void)
{
	int mapfd;
	int pinmuxvalue;
	mapfd = open("/dev/mem", O_RDWR | O_NDELAY);
	if (memfd < 0)
	{
		exit(0);
	}
	pinmux_length 	=	PINMUX_SIZE;
	pinmux_phy_addr	=	PINMUX5_BASE;
	if (map_addr(&mapfd, pinmux_length, pinmux_phy_addr, &pinmux_map_base, &pinmux_virt_addr) == -1)
	{
		exit(0);
	}
	pinmuxvalue= *((uint32_t *)(pinmux_virt_addr));
	printf("readpinmuxvalue=%x\n", pinmuxvalue);
	pinmuxvalue |= 0x00888888;
	*((uint32_t *)(pinmux_virt_addr))=pinmuxvalue;
	pinmuxvalue= *((uint32_t *)(pinmux_virt_addr));
	printf("writepinmuxvalue=%x\n", pinmuxvalue);
	munmap(pinmux_map_base, pinmux_length);
	close(mapfd);
}

void gpiodir_init(void)
{
	int 			tmp;
	tmp= *gpio2_dirreg;
	tmp &= ~(1<<14);
	tmp &= ~(1<<15);
	tmp &= ~(1<<10);
	tmp |= (1<<13);
	tmp |= (1<<11);
	*gpio2_dirreg=tmp;
	tmp= *gpio2_dirreg;
}
void emif_ce2init(void)
{
	int 			tmp;
	cecfg_length 		=	CECFG_SIZE;
	cecfg_phy_addr		=	CE2CFG_BASE;
	if (map_addr(&memfd, cecfg_length, cecfg_phy_addr, &cecfg_map_base, &cecfg_virt_addr) == -1)
	{
		exit(0);
	}
	tmp= *((uint32_t *)(cecfg_virt_addr));
	*((uint32_t *)(cecfg_virt_addr))=0x2091;
	tmp= *((uint32_t *)(cecfg_virt_addr));
}
void Get_filecreat_Time(void)
{
	memset(filename_time, 0, 24);
	time_t now;
	struct tm *curTime;
	now = time(NULL);
	curTime = localtime(&now);
	sprintf(filename_time, "%.4d%.2d%.2d%.2d%.2d%.2d", (2000+curTime->tm_year-100), curTime->tm_mon+1, curTime->tm_mday, curTime->tm_hour, curTime->tm_min, curTime->tm_sec);
	// printf("year=%.4d mounth=%.2d date=%.2d  hour=%.2d  min=%.2d  sec=%.2d \n\r",2000+curTime->tm_year-100,curTime->tm_mon+1,curTime->tm_mday, curTime->tm_hour,curTime->tm_min, curTime->tm_sec);
	// printf("filename_time=s% \r",filename_time);
}

void defparathreadfun(void)
{

}

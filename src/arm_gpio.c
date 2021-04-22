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


void seti2csdadir(uint8_t out_in)
{
	int 		tmp;
	tmp= *gpio2_dirreg;
	if(out_in ==SDAOUT)
		tmp &= ~(1<<15); 	
	else
		tmp |=  (1<<15); 	
	*gpio2_dirreg= tmp; 	
}


void fpga_sda_dir(uint8_t sdadir)
{
	if(sdadir == 1)
		*emifinout=3; 
	else
		*emifinout=4; 
}

void adc_start(void)
{
	*emifinout=5;
}
void adc_stop(void)
{
	*emifinout=6;
}

void fpga_reset(void)
{
	*emifinout=254;
	usleep(300);
	*emifinout=255;
}


void setadc_samfre(uint8_t fre)
{
  uint16_t tmp;
  switch(fre)
	{
		case 10:
					tmp=10;break;
		case 50:
					tmp=50;break;
		case 100:
					tmp=100;break;
		case 150:
					tmp=150;break;
		case 180:
					tmp=180;break;
		case 200:
					tmp=200;break;
		default:
					tmp=200;break;
	}
	*(emifinout+2)=tmp;     
}




void setad7606rden(uint8_t high_low)
{
	int gpio2_data;
	gpio2_data =*gpio2_datareg;
	if(high_low==GPIO_H)
		gpio2_data |=(1<<10);
	else
		gpio2_data &=(~(1<<10));
	*gpio2_datareg=gpio2_data;
}


void seti2csck(uint8_t high_low)
{
	int gpio2_data;
	gpio2_data =*gpio2_datareg;
	if(high_low==GPIO_H)
		gpio2_data |=(1<<14);
	else
		gpio2_data &=(~(1<<14));
	*gpio2_datareg=gpio2_data;
}

void seti2csda(uint8_t high_low)
{
	int gpio2_data;
	gpio2_data =*gpio2_datareg;
	if(high_low==GPIO_H)
		gpio2_data |=(1<<15);
	else
		gpio2_data &=(~(1<<15));
	*gpio2_datareg=gpio2_data;
}

void setfifoad7606_profull(uint8_t high_low)
{
	int gpio2_data;
	gpio2_data =*gpio2_datareg;
	if(high_low==GPIO_H)
		gpio2_data |=(1<<11);
	else
		gpio2_data &=(~(1<<11));
	*gpio2_datareg=gpio2_data;
}

void setfifoad7606_proempty(uint8_t high_low)
{
	int gpio2_data;
	gpio2_data =*gpio2_datareg;
	if(high_low==GPIO_H)
		gpio2_data |=(1<<13);
	else
		gpio2_data &=(~(1<<13));
	*gpio2_datareg=gpio2_data;
}




uint8_t read_fifoad7606_profull(void)
{
	uint16_t gpio2_indata;
	uint32_t gpio2_in32data;

	gpio2_in32data=*gpio2_indatareg;
	gpio2_indata =(uint16_t)(gpio2_in32data);

	if((gpio2_indata & 0x0800) == 0x0800)
		return 1;
	else
		return 0;
}

uint8_t read_fifoad7606_proempty(void)
{
	uint16_t gpio2_indata;
	uint32_t gpio2_in32data;

	gpio2_in32data=*gpio2_indatareg;
	gpio2_indata =(uint16_t)(gpio2_in32data);

	if((gpio2_indata & 0x2000) == 0x2000)
		return 1;
	else
		return 0;
}


uint8_t read_armi2c_sda(void)
{
	uint16_t gpio2_indata;
	uint32_t gpio2_in32data;

	gpio2_in32data=*gpio2_indatareg;
	gpio2_indata =(uint16_t)(gpio2_in32data);
	if((gpio2_indata & 0x8000) == 0x8000)
		return 1;
	else
		return 0;
}
void i2cgpio_init(void)
{
	seti2csda(GPIO_H);
 	seti2csck(GPIO_H);
}


void set_adc_range(uint8_t range)
{
	if(range == ADC10V)
	  *emifinout=7;
	else if(range == ADC5V)
	  *emifinout=8;
	else
	  *emifinout=8;	
}







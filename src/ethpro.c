#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/ioctl.h>
#include <sys/mman.h>
#include <memory.h>
#include <errno.h>
#include <semaphore.h>
#include <netinet/in.h>    // for sockaddr_in
#include <sys/socket.h>    // for socket
#include <arpa/inet.h>     //for inet_addr
#include <sys/wait.h>
#include <termios.h>  
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <netinet/if_ether.h>


#include "getcfg.h"
#include "setnetinfo.h"
#include "arm_gpio.h"
#include "ethpro.h"
#include "main.h"
#include "systime.h"
#include "rx8010sj.h"

#define BUFFLEN 128

unsigned char buff[128];       //以太网接收缓存
unsigned char ethbuff[128];
int len=0;
unsigned char funcode=0;
unsigned char IO_TEST=0;

unsigned char IONUMB=0; 	
unsigned char IOState=0; 
unsigned char IOCurrState=0; 


unsigned char  localmac[6]; 
unsigned char  localip[4];								
unsigned char  localsubnet[4]  ;								  
unsigned char  localgateway[4] ;	 


										
				
unsigned char macaddr[6];
struct ifreq req;
int err,i;

char strlocalip0[10];	
char strlocalip1[10];	
char strlocalip2[10];	
char strlocalip3[10];	

char strlocalsubnet0[10];	
char strlocalsubnet1[10];	
char strlocalsubnet2[10];	
char strlocalsubnet3[10];	

char strlocalmac0[10];	
char strlocalmac1[10];	
char strlocalmac2[10];	
char strlocalmac3[10];	

char strlocalgateway0[10];	
char strlocalgateway1[10];
char strlocalgateway2[10];
char strlocalgateway3[10];


char strlocalip[40];	
char strlocalsubnet[40];	
char strlocalmac[40];	
char strlocalgateway[40];	

void netchar_to_netstr(void)
{
    
	memset(strlocalip,0,sizeof(strlocalip));
	memset(strlocalip0,0,sizeof(strlocalip0));
	sprintf(strlocalip0,"%d",localip[0]);
	memset(strlocalip1,0,sizeof(strlocalip1));
	sprintf(strlocalip1,"%d",localip[1]);
	memset(strlocalip2,0,sizeof(strlocalip2));
	sprintf(strlocalip2,"%d",localip[2]);
	memset(strlocalip3,0,sizeof(strlocalip3));
	sprintf(strlocalip3,"%d",localip[3]);
	strcat(strlocalip,strlocalip0);
	strcat(strlocalip,".");
	strcat(strlocalip,strlocalip1);
	strcat(strlocalip,".");
	strcat(strlocalip,strlocalip2);
	strcat(strlocalip,".");
	strcat(strlocalip,strlocalip3);
 
	memset(strlocalsubnet,0,sizeof(strlocalsubnet));
	memset(strlocalsubnet0,0,sizeof(strlocalsubnet0));
	sprintf(strlocalsubnet0,"%d",localsubnet[0]);
	memset(strlocalsubnet1,0,sizeof(strlocalsubnet1));
	sprintf(strlocalsubnet1,"%d",localsubnet[1]);
	memset(strlocalsubnet2,0,sizeof(strlocalsubnet2));
	sprintf(strlocalsubnet2,"%d",localsubnet[2]);
	memset(strlocalsubnet3,0,sizeof(strlocalsubnet3));
	sprintf(strlocalsubnet3,"%d",localsubnet[3]);

	strcat(strlocalsubnet,strlocalsubnet0);
	strcat(strlocalsubnet,".");
	strcat(strlocalsubnet,strlocalsubnet1);
	strcat(strlocalsubnet,".");
	strcat(strlocalsubnet,strlocalsubnet2);
	strcat(strlocalsubnet,".");
	strcat(strlocalsubnet,strlocalsubnet3);

	memset(strlocalgateway,0,sizeof(strlocalgateway));
	memset(strlocalgateway0,0,sizeof(strlocalgateway0));
	sprintf(strlocalgateway0,"%d",localgateway[0]);
	memset(strlocalgateway1,0,sizeof(strlocalgateway1));
	sprintf(strlocalgateway1,"%d",localgateway[1]);
	memset(strlocalgateway2,0,sizeof(strlocalgateway2));
	sprintf(strlocalgateway2,"%d",localgateway[2]);
	memset(strlocalgateway3,0,sizeof(strlocalgateway3));
	sprintf(strlocalgateway3,"%d",localgateway[3]);

	strcat(strlocalgateway,strlocalgateway0);
	strcat(strlocalgateway,".");
	strcat(strlocalgateway,strlocalgateway1);
	strcat(strlocalgateway,".");
	strcat(strlocalgateway,strlocalgateway2);
	strcat(strlocalgateway,".");
	strcat(strlocalgateway,strlocalgateway3);

   printf("strlocalip= %s \n",strlocalip);
   printf("strlocalsubnet= %s \n",strlocalsubnet);
	printf("strlocalgateway= %s \n",strlocalgateway);

}

void netstr_to_netchar_ipaddr(char *strt)
{
  int i=0,j=0;
  char str[6];
  int dotcnt;
  int dotcnt1cnt;
  int dotcnt2cnt;
  int dotcnt3cnt;
  printf("%s \n",strt);
  dotcnt=0;
  dotcnt1cnt=0;
  dotcnt2cnt=0;
  dotcnt3cnt=0;
  for(i=0;i<40;i++)
   {
    if(dotcnt<3)
     {
     if(strt[i]==46) //"."=46
		{   
         dotcnt++;
         if(dotcnt ==1)
            {
            memset(str,0,sizeof(str));
            dotcnt1cnt=i;
            for(j=0;j<dotcnt1cnt;j++)
               str[j]= strt[j];
            localip[0]=atoi(str);
            }
         else if(dotcnt==2)
            {
            memset(str,0,sizeof(str));
            dotcnt2cnt=i;
            for(j=dotcnt1cnt+1;j<dotcnt2cnt;j++)
               str[j-dotcnt1cnt-1]= strt[j];
            localip[1]=atoi(str);
            }
          else if(dotcnt==3)
            {
            memset(str,0,sizeof(str));
            dotcnt3cnt=i;
            for(j=dotcnt2cnt+1;j<dotcnt3cnt;j++)
               str[j-dotcnt2cnt-1]= strt[j];
            localip[2]=atoi(str);            
            }   
         else
            {
               ;
            }
        }
      }
     else
        {
         if(ipaddr[i]==0)
            {
           if((i-dotcnt3cnt)==1)
             {    localip[3]=0;
                  break;
				 }
           else
                {
                memset(str,0,sizeof(str));
            		for(j=dotcnt3cnt+1;j<i;j++)
               		str[j-dotcnt3cnt-1]= strt[j];
            		localip[3]=atoi(str);
                break;  
                }
            } 

        }
   }

}
void netstr_to_netchar_netmask(char *strt)
{
  int i=0,j=0;
  char str[6];
  int dotcnt;
  int dotcnt1cnt;
  int dotcnt2cnt;
  int dotcnt3cnt;
  printf("%s \n",strt);
  dotcnt=0;
  dotcnt1cnt=0;
  dotcnt2cnt=0;
  dotcnt3cnt=0;
  for(i=0;i<40;i++)
   {
    if(dotcnt<3)
     {
     if(strt[i]==46) //"."=46
		{   
         dotcnt++;
         if(dotcnt ==1)
            {
            memset(str,0,sizeof(str));
            dotcnt1cnt=i;
            for(j=0;j<dotcnt1cnt;j++)
               str[j]= strt[j];
            localsubnet[0]=atoi(str);
            }
         else if(dotcnt==2)
            {
            memset(str,0,sizeof(str));
            dotcnt2cnt=i;
            for(j=dotcnt1cnt+1;j<dotcnt2cnt;j++)
               str[j-dotcnt1cnt-1]= strt[j];
            localsubnet[1]=atoi(str);
            }
          else if(dotcnt==3)
            {
            memset(str,0,sizeof(str));
            dotcnt3cnt=i;
            for(j=dotcnt2cnt+1;j<dotcnt3cnt;j++)
               str[j-dotcnt2cnt-1]= strt[j];
            localsubnet[2]=atoi(str);            
            }   
         else
            {
               ;
            }
        }
      }
     else
        {
         if(ipaddr[i]==0)
            {
           if((i-dotcnt3cnt)==1)
             {    localsubnet[3]=0;
                  break;
				 }
           else
                {
                memset(str,0,sizeof(str));
            		for(j=dotcnt3cnt+1;j<i;j++)
               		str[j-dotcnt3cnt-1]= strt[j];
            		localsubnet[3]=atoi(str);
                break;  
                }
            } 

        }
   }

}
void netstr_to_netchar_gateway(char *strt)
{
  int i=0,j=0;
  char str[6];
  int dotcnt;
  int dotcnt1cnt;
  int dotcnt2cnt;
  int dotcnt3cnt;
  printf("%s \n",strt);
  dotcnt=0;
  dotcnt1cnt=0;
  dotcnt2cnt=0;
  dotcnt3cnt=0;
  for(i=0;i<40;i++)
   {
    if(dotcnt<3)
     {
     if(strt[i]==46) //"."=46
		{   
         dotcnt++;
         if(dotcnt ==1)
            {
            memset(str,0,sizeof(str));
            dotcnt1cnt=i;
            for(j=0;j<dotcnt1cnt;j++)
               str[j]= strt[j];
            localgateway[0]=atoi(str);
            }
         else if(dotcnt==2)
            {
            memset(str,0,sizeof(str));
            dotcnt2cnt=i;
            for(j=dotcnt1cnt+1;j<dotcnt2cnt;j++)
               str[j-dotcnt1cnt-1]= strt[j];
            localgateway[1]=atoi(str);
            }
          else if(dotcnt==3)
            {
            memset(str,0,sizeof(str));
            dotcnt3cnt=i;
            for(j=dotcnt2cnt+1;j<dotcnt3cnt;j++)
               str[j-dotcnt2cnt-1]= strt[j];
            localgateway[2]=atoi(str);            
            }   
         else
            {
               ;
            }
        }
      }
     else
        {
         if(ipaddr[i]==0)
            {
           if((i-dotcnt3cnt)==1)
             {    localgateway[3]=0;
                  break;
				 }
           else
                {
                memset(str,0,sizeof(str));
            		for(j=dotcnt3cnt+1;j<i;j++)
               		str[j-dotcnt3cnt-1]= strt[j];
            		localgateway[3]=atoi(str);
                break;  
                }
            } 

        }
   }

}
void conver_netstr_to_netchar(void)
{
 	netstr_to_netchar_ipaddr(ipaddr);
	netstr_to_netchar_netmask(netmask);
	netstr_to_netchar_gateway(gateway);

}



extern uint8_t startcov;

extern uint16_t Frecnt;

uint8_t u8421second,u8421minute,u8421hour,u8421week,u8421day,u8421month,u8421year;
uint8_t u10second,u10minute,u10hour,u10week,u10day,u10month,u10year;


Time_Typedef  RX8010SetTimeVAL;

void u8424_to_decimal(void)
{

  	u10second=(u8421second >> 4)*10 + (u8421second	&	0x0F);
	u10minute=(u8421minute >> 4)*10 + (u8421minute	&	0x0F);
	u10hour	=(u8421hour 	 >> 4)*10 + 	(u8421hour	&	0x0F);
	u10week	=(u8421week   >> 4)*10 + (u8421week	&	0x0F);
	u10day		=(u8421day   >> 4)*10 + 	(u8421day		&	0x0F);
	u10month	=(u8421month >> 4)*10 + 	(u8421month	&	0x0F);
	u10year	=(u8421year  >> 4)*10 + 	(u8421year	&	0x0F);
}


void decimal_to_8424u(void)
{
  	u8421second	=((u10second/10) <<4) +(u10second%10); 
	u8421minute	=((u10minute/10) <<4) +(u10minute%10); 
	u8421hour		=((u10hour/10) <<4) 	+(u10hour%10); 
	u8421week		=((u10week/10) <<4) 	+(u10week%10); 
	u8421day		=((u10day/10) <<4) 	+(u10day%10); 
	u8421month	=((u10month/10) <<4) 	+(u10month%10); 
	u8421year		=((u10year/10) <<4) 	+(u10year%10); 
}

void Eth_Cmd_Pro(void)
{
                   
  funcode= buff[1];
  switch(funcode)
   {
					case 0x51:  
								 send(s_c, buff, len,0);		   
								 usleep(1000);
					          localip[0]=buff[2];
					          localip[1]=buff[3];
					          localip[2]=buff[4];
								 localip[3]=buff[5];
					          localsubnet[0]=buff[6];
					          localsubnet[1]=buff[7];
					          localsubnet[2]=buff[8];
								 localsubnet[3]=buff[9];					
					          localgateway[0]=buff[10];
					          localgateway[1]=buff[11];
					          localgateway[2]=buff[12];
								 localgateway[3]=buff[13];	
                          netchar_to_netstr(); 
	                       memset(ipaddr,0,sizeof(ipaddr));
                          strcpy(ipaddr,strlocalip);
                          memset(gateway,0,sizeof(gateway));
                          strcpy(gateway,strlocalgateway);
	                       memset(netmask,0,sizeof(netmask));
                          strcpy(netmask,strlocalsubnet);
								 Write_Configfile(ipaddr,gateway,netmask,5000,6000);//					         
								 conver_netstr_to_netchar();                       
								 memset(ipaddr,0,sizeof(ipaddr));
								 memset(gateway,0,sizeof(gateway));
								 memset(netmask,0,sizeof(netmask)); 
								 memset(configfilepath,0,sizeof(configfilepath));
								 strcpy(configfilepath,"/usr/configinfo.txt");
								 GetConfigValue(configfilepath);								
								 SetIfAddr("eth0", ipaddr, netmask,gateway); 								                        
						       break;						
               case  0x52: 
                          send(s_c, buff, len,0);		        
								 usleep(1000);
                          system("reboot"); 
                          break; 
					case 0x53:							
								send(s_c, buff, len,0);		     
								usleep(1000);
								if(buff[2]<19 || buff[2]>100)
									goto timeerr;
								if(buff[3]>12)
									goto timeerr;
								if(buff[4]>31)
									goto timeerr;
								if(buff[5]>23)
									goto timeerr;	
								if(buff[6]>59)
									goto timeerr;	
								if(buff[7]>59)
									goto timeerr;		
								u10second= buff[7];
								u10minute	= buff[6];	
								u10hour	= buff[5];	
								u10week	= 1;			
								u10day		= buff[4];	
								u10month	= buff[3];
								u10year	= buff[2];
								decimal_to_8424u();
								RX8010SetTimeVAL.second=u8421second;
								RX8010SetTimeVAL.minute=u8421minute;
								RX8010SetTimeVAL.hour=u8421hour;
								RX8010SetTimeVAL.week=u8421week;
								RX8010SetTimeVAL.date=u8421day;
								RX8010SetTimeVAL.month=u8421month;
								RX8010SetTimeVAL.year=u8421year;
								RX8010_Time_Init(&RX8010SetTimeVAL); 
								setlinuxtimechar(buff[2],buff[3],buff[4],buff[5],buff[6],buff[7]);
								break;
					timeerr:
								printf("setting_systime_data_format_error!\n"); 
								break;
					case 0x54:
								send(s_c, buff, len,0);		    
								usleep(1000);
								if(buff[2]==10)					
									Frecnt=10;
								else if(buff[2]==50)		
									Frecnt=50;
								else if(buff[2]==100)				
									Frecnt=100;
								else if(buff[2]==150)		
									Frecnt=150;
								else if(buff[2]==180)			
									Frecnt=180;
								else if(buff[2]==200)
									Frecnt=200;				
								else																	
									Frecnt=200;					
								setadc_samfre(Frecnt);
								break;
					case 0x55:
								send(s_c, buff, len,0);		        
								usleep(1000);
								startcov=1;
								break;
					case 0x56:
								send(s_c, buff, len,0);		      
								usleep(1000);
								startcov=0;
								break;

   }

}

int heart1mscnt=0;


struct 	sockaddr_in from;	   			
void eththreadfun(void)
{
  memset(localmac,0,sizeof(localmac));
  socklen_t lenf = sizeof(from);
 
  while(1)
  {
	s_c = accept(s_s,(struct sockaddr*)&from,&lenf);    		
	if(s_c > 0)
	{
		while(1)
		{
			len = recv(s_c, buff, BUFFLEN,0); 					
			if(len >0)							
			{
				if(len == 15)
				{
					if((buff[0]==0xA5) && (buff[14]==0xE9))		
		   			{						
						Eth_Cmd_Pro();                 	
					}
				}
			}
			else  if(len == 0)                      		
			{
		  		break;                              					  
			}
			else 														
			{       
		   		if( (errno == EINTR) || (errno == EWOULDBLOCK) || (errno == EAGAIN))
		    		;
		   		else
		     	{ 
		      		break;  
		     	}
		  	}
		  usleep(500000);	
		}
	}
	usleep(1000);	
  }
 close(s_c);
 close(s_s);
 pthread_exit(NULL);
}








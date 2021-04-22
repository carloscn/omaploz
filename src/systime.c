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
#include "systime.h"

int getsystime(void)
{
	time_t time_now;
	struct tm *curr_time = NULL;
	time(&time_now);
	curr_time = gmtime(&time_now);
   cursystime.year=1900 + curr_time->tm_year;
	cursystime.month=1 + curr_time->tm_mon;
	cursystime.day=curr_time->tm_mday;
	cursystime.hour=curr_time->tm_hour;
	cursystime.minute=curr_time->tm_min;
	cursystime.second=curr_time->tm_sec;
	//printf("localtime=%d.%02d.%02d %02d:%02d:%02d\n",cursystime.year,cursystime.month,cursystime.day,cursystime.hour,cursystime.minute,cursystime.second);
   return 0;
}


int setlinuxtimestr(char *dt)
{
	 struct tm rtc_time;
    struct tm _tm;
    struct timeval tv;
    time_t timep;
    sscanf(dt,"%d-%d-%d %d:%d:%d", &rtc_time.tm_year,&rtc_time.tm_mon, &rtc_time.tm_mday,&rtc_time.tm_hour,&rtc_time.tm_min, &rtc_time.tm_sec);
    _tm.tm_sec 		= rtc_time.tm_sec;
    _tm.tm_min 		= rtc_time.tm_min;
    _tm.tm_hour 	= rtc_time.tm_hour;
    _tm.tm_mday 	= rtc_time.tm_mday;
    _tm.tm_mon 		= rtc_time.tm_mon - 1;
    _tm.tm_year 	= rtc_time.tm_year - 1900;
    timep 			= mktime(&_tm);
    tv.tv_sec 		= timep;
    tv.tv_usec 		= 0;
    if(settimeofday (&tv, (struct timezone *) 0) < 0)
     {
        return -1;
     }
    system("hwclock -w ");
    return 0;
}

int setlinuxtimechar(unsigned char year,unsigned char month,unsigned char day,unsigned char hour,unsigned char minute,unsigned char second)
{
	 struct tm rtc_time;
    struct tm _tm;
    struct timeval tv;
    time_t timep;

    //sscanf(dt,"%d-%d-%d %d:%d:%d", &rtc_time.tm_year,&rtc_time.tm_mon, &rtc_time.tm_mday,&rtc_time.tm_hour,&rtc_time.tm_min, &rtc_time.tm_sec);
     
	 rtc_time.tm_year=year+2000;
	 rtc_time.tm_mon=month;
	 rtc_time.tm_mday=day;
	 rtc_time.tm_hour=hour;
	 rtc_time.tm_min=minute;
	 rtc_time.tm_sec=second;


    _tm.tm_sec 		= rtc_time.tm_sec;
    _tm.tm_min 		= rtc_time.tm_min;
    _tm.tm_hour 	= rtc_time.tm_hour;
    _tm.tm_mday 	= rtc_time.tm_mday;
    _tm.tm_mon 		= rtc_time.tm_mon - 1;
    _tm.tm_year 	= rtc_time.tm_year - 1900;
    timep 			= mktime(&_tm);
    tv.tv_sec 		= timep;
    tv.tv_usec 		= 0;
    if(settimeofday (&tv, (struct timezone *) 0) < 0)
     {
        return -1;
     }
    system("hwclock -w "); 
    return 0;
}














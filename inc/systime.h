#ifndef _SYSTIME_H
#define _SYSTIME_H
int getsystime(void);
int setlinuxtimestr(char *dt);
int setlinuxtimechar(unsigned char year,unsigned char month,unsigned char day,unsigned char hour,unsigned char minute,unsigned char second);
struct _gettime{
int year;
char month;
char day;
char hour;
char minute;
char second;
}cursystime,settime;
#endif

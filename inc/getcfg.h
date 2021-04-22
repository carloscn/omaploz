#ifndef __GETCFG_H_
#define __GETCFG_H_
int Creat_Default_Configfile(void);
//int GetConfigValue( char* FileName, char *ipaddr,char *gateway,char *netmask,int modbustcpport,int hsport);
int GetConfigValue( char* FileName);
int Write_Configfile(char * ipaddr,char * gateway,char * netmask,int modbustcpport,int hsport);
#endif

#ifndef __ETHPRO_H_
#define __ETHPRO_H_

void eththreadfun(void);
void mosthreadfun(void);
void conver_netstr_to_netchar(void);
void u8424_to_decimal(void);
////接收计算机发送的网络参数缓存////////////
extern unsigned char  localmac[6]; 
extern unsigned char  localip[4];			 /*默认IP*/							
extern unsigned char  localsubnet[4]  ;			 /*默认子网掩码*/							  
extern unsigned char  localgateway[4] ;			 /*默认网关*/	
extern uint8_t u8421second,u8421minute,u8421hour,u8421week,u8421day,u8421month,u8421year;
extern uint8_t u10second,u10minute,u10hour,u10week,u10day,u10month,u10year;


#endif

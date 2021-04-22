#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <termios.h>
#include <errno.h>
#include "getcfg.h"
#include "setnetinfo.h"
#include "main.h"

/*查找读取配置参数*/
int GetCFGValue( char* CFGBuffer, int Buflen, char *pKeyName, char *pItemName, char* pStr )
{
	int   i1, i2, len1, len2;

	len1 = strlen( pKeyName );
	len2 = strlen( pItemName );

	for( i1=0; i1<Buflen; i1++ )
	{
		if( strncmp( &CFGBuffer[i1], pKeyName, len1 ) == 0 )
		{
			i1 += len1;
			break;
		}
	}
	if( i1==Buflen )    return  -1;
	for( ; i1<Buflen; i1++ )
	{
		if( strncmp( &CFGBuffer[i1], pItemName, len2 )== 0 )
		{
			i1+= len2;
			break;
		}
	}
	if( i1==Buflen )    return -1;
	int Flg=0;
	for( i2=0; i1<Buflen; i1++)
	{
   

		if( (CFGBuffer[i1]==0x22) &&(	Flg==0 ) )/*第一个“*/
		{
			Flg = 1;
			continue;		
		}
		if( (CFGBuffer[i1]==0x22)&&(Flg>0) )/*第二个“*/
		{	
			break;			
		}
		pStr[i2] = CFGBuffer[i1];
		i2++;
          //printf("i1=%d i2=%d CFGBuffer=%d \n",i1,i2,CFGBuffer[i1]);

	}
	pStr[i2] = 0;
	return 0;
}


/*创建默认配置文件*/
int Creat_Default_Configfile(void)
{
   	memset(ipaddr,0,sizeof(ipaddr));
   	memset(gateway,0,sizeof(gateway));
   	memset(netmask,0,sizeof(netmask));
   	modbustcpport =5000;					/*设置默认低速AD采集端口*/
	hsport =6000;							/*设置默认高速AD采集端口*/
   strcpy(ipaddr,"192.168.1.10"); 		/*设置默认IP*/
	strcpy(gateway,"192.168.1.1");		/*设置默认网关*/
	strcpy(netmask,"255.255.255.0");	/*设置默认掩码*/			
   Write_Configfile(ipaddr,gateway,netmask,modbustcpport,hsport);
	printf("Creat Default ConfigFile is OK\n");
   return 1;
}

/*写配置参数到配置文件*/
int Write_Configfile(char * ipaddr,char * gateway,char * netmask,int modbustcpport,int hsport)
{
   char s_ipaddr[40];
	char s_gateway[40];
	char s_netmask[40];
	char s_modbustcpport[20];
	char s_hsport[20];
   uint8_t len;
	int fd = -1;
   memset(s_ipaddr,0,sizeof(s_ipaddr));
	memset(s_gateway,0,sizeof(s_gateway));
	memset(s_netmask,0,sizeof(s_netmask));
	memset(s_modbustcpport,0,sizeof(s_modbustcpport));	
	memset(s_hsport,0,sizeof(s_hsport));	

  	strcpy(s_ipaddr,ipaddr);
	strcpy(s_gateway,gateway);
	strcpy(s_netmask,netmask);
  	sprintf(s_modbustcpport,"%d",modbustcpport);		/*将整数 转化为字符串*/
	sprintf(s_hsport,"%d",hsport);							/*将整数 转化为字符串*/

   system("rm -rf configfilepath");                  /*删除文件*/
	fd= open(configfilepath,O_RDWR | O_CREAT,S_IRWXU);/*新建立文件*/
	/*写文件*/
   len = strlen(s_ipaddr);
   write(fd,"[IPADDR]\n",sizeof("[IPADDR]"));
   write(fd,"ip=\"",sizeof("ip="));
	write(fd,s_ipaddr,len);
	write(fd,"\"\n",sizeof("\""));

   len = strlen(s_gateway);
   write(fd,"[GATEWAY]\n",sizeof("[GATEWAY]"));
   write(fd,"GateWay=\"",sizeof("GateWay="));
	write(fd,s_gateway,len);
	write(fd,"\"\n",sizeof("\""));

   len = strlen(s_netmask);
   write(fd,"[NETMASK]\n",sizeof("[NETMASK]"));
   write(fd,"NetMask=\"",sizeof("NetMask="));
	write(fd,s_netmask,len);
	write(fd,"\"\n",sizeof("\""));

   len = strlen(s_modbustcpport);
   write(fd,"[MODBUSPORT]\n",sizeof("[MODBUSPORT]"));
   write(fd,"ModbusPort=\"",sizeof("ModbusPort="));
	write(fd,s_modbustcpport,len);
	write(fd,"\"\n",sizeof("\""));

   len = strlen(s_hsport);
   write(fd,"[HSPORT]\n",sizeof("[HSPORT]"));
   write(fd,"HsPort=\"",sizeof("HsPort="));
	write(fd,s_hsport,len);
	write(fd,"\"\n",sizeof("\""));
   close(fd);
	printf("Write ConfigFile is OK\n");
   return 1;
}

/*通过配置文件读取配置参数*/
//int GetConfigValue( char* FileName, char *ipaddr,char *gateway,char *netmask,int modbustcpport,int hsport)
int GetConfigValue( char* FileName)

{
	FILE*	fp;
	char  	ValueStr[24];
   char   Buffer[512];
	int		nBytes;
	int		i;
   int   errcfg=0;

   errcfg =0;
   fp = fopen( FileName, "rb+" );
   if(fp==NULL ) /*配置文件是否存在*/
	{
      Creat_Default_Configfile();  /*创建默认配置文件*/
		printf("Creat Default ConfigFile\n");
	}
	else
	{
		fread( Buffer, 512, 1, fp );
		nBytes = strlen( Buffer );
        /*
		printf("nBytes= %d\n",nBytes);
      for(i=0;i<nBytes;i++)
         printf("i=%d Buffer=%d \n",i,Buffer[i]);
		*/
		fclose( fp );
		if( nBytes > 0 )
		{
			printf(" ConfigFile Open Well\n");
			//get ip address
			if (0 ==  GetCFGValue( Buffer, nBytes, "[IPADDR]", "ip=", ValueStr )) {
				strcpy(ipaddr,ValueStr);
             printf("IPADDRSring=%s \n",ValueStr);
			} else {
				errcfg =1;	/*读参数出错*/
				//strcpy(ipaddr,"192.168.1.50");
			}
			printf("localip read ok\n");
			//get ip GateWay 
			if (0 ==  GetCFGValue( Buffer, nBytes, "[GATEWAY]", "GateWay=", ValueStr )){
				strcpy(gateway,  ValueStr);
             printf("GateWaySring=%s \n",ValueStr);
			}else{
				errcfg =1; /*读参数出错*/
				//strcpy( gateway ,"192.168.1.1");
			}
			printf("gateway read ok\n");
			//get ip NetMask  
			if (0 ==  GetCFGValue( Buffer, nBytes, "[NETMASK]", "NetMask=", ValueStr )){
				strcpy(netmask, ValueStr);
             printf("NetMaskSring=%s \n",ValueStr);
			}else{
				errcfg =1; /*读参数出错*/
				//strcpy( netmask , "255.255.255.0");
			}           
			printf("netmask read ok\n");              
			//get ModbusPort
			if (0 ==  GetCFGValue( Buffer, nBytes, "[MODBUSPORT]", "ModbusPort=", ValueStr ))
			{
             printf("ModbusPortSring=%s \n",ValueStr);
				modbustcpport = atoi( ValueStr );
             printf("modbustcpport=%d \n",modbustcpport);
			}
			else
				errcfg =1;  /*读参数出错*/
			printf("modbustcpport read ok\n"); 
			//get HsPort
			if (0 ==  GetCFGValue( Buffer, nBytes, "[HSPORT]", "HsPort=", ValueStr ))
			{
           printf("HsPortSring=%s \n",ValueStr);
			  hsport = atoi( ValueStr );
			  printf("hsport=%d \n",hsport);
			}
			else
				errcfg =1;  /*读参数出错*/
			printf("hsport read ok\n"); 

		}
      else /*文件存在但是为空文件*/
		{
			printf("ConfigFile is Zero Creat Default ConfigFile\n");
          Creat_Default_Configfile();  /*创建默认配置文件*/
		}
	}
  

   if(errcfg ==1) /*读配置文件出现错误,生成默认配置文件*/
  	{
      	printf("Read Err Creat Default ConfigFile\n");
		Creat_Default_Configfile();  /*创建默认配置文件*/
      	errcfg =0;
	}

	return 1;
}


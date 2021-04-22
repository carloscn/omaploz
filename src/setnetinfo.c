#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <error.h>
#include <net/route.h>
#include "getcfg.h"
#include "setnetinfo.h"
#include "main.h"


char configfilepath[64]="/usr/configinfo.txt"; 
 

int SetIfAddr(char *ifname, char *Ipaddr, char *mask,char *gateway)
{
    int fd;
    int rc;
    struct ifreq ifr; 
    struct sockaddr_in *sin;
    struct rtentry  rt;
    
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0)
    {
            perror("socket   error");     
            return -1;     
    }
    memset(&ifr,0,sizeof(ifr)); 
    strcpy(ifr.ifr_name,ifname); 
    sin = (struct sockaddr_in*)&ifr.ifr_addr;     
    sin->sin_family = AF_INET;  
   
    //ipaddr
    if(inet_aton(Ipaddr,&(sin->sin_addr)) < 0)   
    {     
        perror("inet_aton   error");     
        return -2;     
    }    
    
    if(ioctl(fd,SIOCSIFADDR,&ifr) < 0)   
    {     
        perror("ioctl   SIOCSIFADDR   error");     
        return -3;     
    }

    //netmask
    if(inet_aton(mask,&(sin->sin_addr)) < 0)   
    {     
        perror("inet_pton   error");     
        return -4;     
    }    
    if(ioctl(fd, SIOCSIFNETMASK, &ifr) < 0)
    {
        perror("ioctl");
        return -5;
    }

    //gateway
    memset(&rt, 0, sizeof(struct rtentry));
    memset(sin, 0, sizeof(struct sockaddr_in));
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    if(inet_aton(gateway, &sin->sin_addr)<0)
    {
       printf ( "inet_aton error\n" );
    }
    memcpy ( &rt.rt_gateway, sin, sizeof(struct sockaddr_in));
    ((struct sockaddr_in *)&rt.rt_dst)->sin_family=AF_INET;
    ((struct sockaddr_in *)&rt.rt_genmask)->sin_family=AF_INET;
    rt.rt_flags = RTF_GATEWAY;
    if (ioctl(fd, SIOCADDRT, &rt)<0)
     {
       // zError( "ioctl(SIOCADDRT) error in set_default_route\n");
        close(fd);
          
        return -1;
     }
    printf("NetInfo Setting OK \n");
    close(fd);
    return rc;
}



















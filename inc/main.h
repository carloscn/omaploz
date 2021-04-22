#ifndef _MAIN_H
#define _MAIN_H
extern int    dev_gpio_fd;  
extern int 	s_s;						//服务器套接字文件描述符
extern int 	s_c;						//客户端套接字文件描述符
extern char	   ipaddr[40];
extern char     gateway[40];
extern char     netmask[40];
extern int			modbustcpport;
extern int			hsport;
extern unsigned char 	*GPIOBASE;   /*GPIO BASE_ADDR*/

extern int 		emif_fd; 		// 打开文件设备获得文件描述符 
extern void 		*emif_map_base;	// emif_map_base是mmap返回的起始地址, 
extern void 		*emif_virt_addr;	// emif_virt_addr是用户实际操作的地址	
extern size_t 	emif_length; 		// 用户输入的读写长度
extern off_t 		emif_phy_addr; 	// 用户所指定的物理地址
extern char 		emif_access_type; 	// 访问存储的位宽8位、16位、32位
#endif












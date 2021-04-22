#ifndef __SETNETINFO_H_
#define __SETNETINFO_H_

int SetIfAddr(char *ifname, char *Ipaddr, char *mask,char *gateway);
extern char		ipaddr[40];
extern char         	gateway[40];
extern char         	netmask[40];
extern int		modbustcpport;
extern int		hsport;
extern char 		configfilepath[64];

#endif

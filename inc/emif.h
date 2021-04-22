#ifndef __EMIF_H_
#define __EMIF_H_
int map_addr(int fd, size_t length, off_t phy_addr, void **map_base, void **virt_addr); 
int read_test(void *start, size_t length, char access_type, uint32_t writeval);
int emif(void);
#endif

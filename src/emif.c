#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <stdint.h>
#include <linux/types.h>

#define MEGA 1048576 // 1024 * 1024
#define FATAL() do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
__LINE__, __FILE__, errno, strerror(errno)); exit(-1); } while(0)

 float error_rate_compute(void *read_result, uint32_t writeval, int size, char access_type);
 int 	 write_test(void *start, size_t length ,char access_type, uint32_t writeval);
 int   read_test(void *start, size_t length, char access_type, uint32_t writeval);
 int   command_line_parse(int argc, char *argv[], off_t *phy_addr, size_t *length, char \
			*access_type, uint32_t *writeval);
 int   map_addr(int *fd, size_t length, off_t phy_addr, void **map_base,void **virt_addr );

int emif(void) {
	int 		fd; 			
	void 		*map_base=NULL;		 
   	void 		*virt_addr=NULL;		
	size_t 		length=0; 		
	off_t 		phy_addr=0; 		
	char 		access_type = 'b'; 	
	uint32_t 	writeval = 0;  		 
   length=8;
	phy_addr=0x60000000;
	access_type = 'h';
	if (map_addr(&fd, length, phy_addr, &map_base, &virt_addr) == -1)
		FATAL();
	if (read_test(virt_addr, length, access_type, writeval) == -1) {
		close(fd);
		FATAL();
	}
	if (munmap(map_base, length) == -1) { 
		close(fd);
		FATAL();
	}
	close(fd);
	return 0;
}

int write_test(void *start, size_t length, char access_type, uint32_t writeval) {
	float 			seconds; 
	float 			speed; 
	struct 	timeval time_start, time_stop, time_diff; 
	
	uint32_t j;
	gettimeofday(&time_start,0); 
	switch(access_type) {
			case 'b':{
				uint8_t *in = (uint8_t *)start;
				for(j=0; j < length; j++) {
					in[j] = (uint8_t)writeval; 
				}
			}
				break;
			case 'h':{
				uint16_t *in = (uint16_t *)start;
				for(j=0; j < length / sizeof(uint16_t); j++) {
					in[j] =  (uint16_t)writeval;
				}
			}
				break;
			case 'w':{
				uint32_t *in = (uint32_t *)start;
				for(j=0; j < length / sizeof(uint32_t); j++) {
					in[j] =  (uint32_t)writeval;				
				}
			}
				break;
			case 'm':{
				void * tmp = memset(start, (uint8_t)writeval, length); 
				if (tmp != start){
					fprintf(stderr, "memset failed !\n");
					return -1;//
				}
			}
				break;
			case 'd':{
				*((uint32_t *) start) = (uint32_t)writeval;
				return 0;
			}
				 break;
			default:
				fprintf(stderr, "Illegal data type '%c'.\n", access_type);
				return -1;//
		}
	gettimeofday(&time_stop,0); 
	timersub(&time_stop, &time_start, &time_diff); 	
	seconds = time_diff.tv_sec + time_diff.tv_usec / 1000000.0;
	speed = length / seconds / (float)MEGA;
	printf("%d bytes (%1.3fMB) writed, %1.3fs, %1.3fMB/s\n", 
								length, 
								length / (float)MEGA, 
								seconds, 
								speed);	
	fflush(stdout);	
	return 0;
}


int read_test(void *start, size_t length, char access_type, uint32_t writeval) {
	float seconds; 
	float speed; 
	float error_rate;
	
	void *read_result = malloc(length);
	if (NULL == read_result) {
		fprintf(stderr, "malloc failed!\n");
		return -1;
	}
	
	void *tmp_mms = memset(read_result, 1, length);
	if (tmp_mms != read_result) {
		fprintf(stderr, "memset failed!\n");
		free(read_result);
		return -1;
	}
	
	struct timeval time_start, time_stop, time_diff;  
	int j, size;
	
	gettimeofday(&time_start, 0); 
	switch(access_type) {
		case 'b':{
			size = length;
			uint8_t *in = (uint8_t *)start;
			uint8_t *out = (uint8_t *)read_result;
			for (j = 0; j < size; j++) {
				out[j] = in[j];
			}
		}
			break;
		case 'h': {
			size =length / sizeof(uint16_t);
			uint16_t *in = (uint16_t *)start;
			uint16_t *out = (uint16_t *)read_result;
			for (j = 0; j < size; j++) {
				out[j] = in[j];
			}
		}
			break;
		case 'w':{
			size = length / sizeof(uint32_t);
			uint32_t *in = (uint32_t *)start;
			uint32_t *out = (uint32_t *)read_result;
			for (j = 0; j < size; j++) {
				out[j] = in[j];
			}
		}
			break;
		case 'm':{
			size = length / sizeof(uint8_t);
			void *tmp_mmcp = memcpy(read_result, start, length);
			if (tmp_mmcp != read_result) {
				fprintf(stderr, "memcpy failed!\n");
				free(read_result);
				return -1;//
			}
			break;
		}
		case 'd':{
			uint32_t readback = *((uint32_t*)start);
			printf("Written %u; readback %u\n", writeval, readback); 
			fflush(stdout);
			free(read_result);
			return 0;
		}
			break;
		default:
			fprintf(stderr, "Illegal data type '%c'.\n", access_type);
			free(read_result);
			return -1;
	}
	gettimeofday(&time_stop, 0); 
	timersub(&time_stop, &time_start, &time_diff); 	
	seconds = time_diff.tv_sec + time_diff.tv_usec/1000000.0;
	speed = length / seconds;
	speed /= (float)MEGA;
	error_rate = error_rate_compute(read_result, writeval, size, access_type);
	printf("%d bytes (%1.3fMB) read, %1.3fs, %1.3fMB/s, error rate=%1.1f%%\n", 
										length, 
										length/(float)MEGA, 
										seconds, 
										speed, 
										error_rate);
	free(read_result);
	return 0;
}

float error_rate_compute(void *read_result, uint32_t writeval, int size, char access_type) {
	int i;
	float errors=0;
	float error_rate=0;
	
	switch(access_type) {
		case 'm':
		case 'b':{ 
			uint8_t *out = (uint8_t *)read_result;
			for (i = 0; i < size; i++) {
				if ((uint8_t)writeval != out[i]) {
					errors++;
					printf("error: Written %u, but readback %u\n", writeval, out[i]); 
				}
			}
		}

			break;
		case 'h':{
			uint16_t *out = (uint16_t *)read_result;
			for (i = 0; i < size; i++) {
				if ((uint16_t)writeval != out[i]) {
					errors++;
					printf("error: Written %u, but readback %u\n", writeval, out[i]); 
				}
			}
		}
			break;
			
		case 'w':{ 
			uint32_t *out = (uint32_t *)read_result;
			for (i = 0; i < size; i++) {
				if ((uint32_t)writeval != out[i]){
					errors++;
					printf("error: Written %u, but readback %u\n", writeval, out[i]); 
				}
			}
		}
			break;

		default:
			fprintf(stderr, "Illegal data type '%c'.\n", access_type);
			return -1;
	}
	
	error_rate = errors/size;
	error_rate *= 100.0;
	return error_rate;
}
 
 int command_line_parse(int argc, char *argv[], off_t *phy_addr, size_t *length, char *access_type, uint32_t *writeval) {
	if (argc < 5) {
		fprintf(stderr, "\nUsage:\t%s {phy_addr length type data}\n"
			"\taddress   : memory address to act upon\n"
			"\tlength 	 : the length of the mapping\n"
			"\ttype   	 : access operation type-[b]yte,[h]alfword,[w]ord,[m]emset/copy, [d]isplay\n"
			"\tdata      : data to be write, it should not be negative, and if it's above\n"
			"       	   255, it will be truncated according to the argument type\n\n", 
			argv[0]);
		return -1;
	}
	
	*phy_addr = strtoul(argv[1], 0, 0); 
	*length = strtoul(argv[2], 0, 0); 
	*access_type = tolower(argv[3][0]);
	*writeval = strtoul(argv[4], 0, 0);
	return 0;
}
 
int map_addr(int *fd, size_t length, off_t phy_addr, void **map_base, void **virt_addr) {
	fflush(stdout);
	off_t pa_offset = phy_addr & ~(sysconf(_SC_PAGE_SIZE) - 1); 
	*map_base = mmap(NULL, length + phy_addr - pa_offset, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, pa_offset);
	if (*map_base == (void *) -1) {
		close(*fd);
		return -1;
	}
 	printf("Memory mapped at address %p.\n", *map_base); 
	*virt_addr = *map_base  + phy_addr - pa_offset;
	return 0;
}


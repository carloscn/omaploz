#ifndef _ARM_GPIO_H
#define _ARM_GPIO_H

#define pin0  0
#define pin1  1
#define pin2  2
#define pin3  3
#define pin4  4
#define pin5  5
#define pin6  6
#define pin7  7
#define pin8  8
#define pin9  9
#define pin10  10
#define pin11  11
#define pin12  12
#define pin13  13
#define pin14  14
#define pin15  15

#define SDAOUT	1
#define SDAIN	2
#define GPIO_H	1
#define GPIO_L	0

#define ADC10V 	1
#define ADC5V 	2
#define FPGASDA2ARM  1   
#define ARM2FPGASDA  2	

extern void 		*emif_map_base;					
extern void 		*emif_virt_addr;				
extern void 		*pinmux_map_base;				 
extern void 		*pinmux_virt_addr;			
extern void 		*gpio_map_base;					
extern void 		*gpio_virt_addr;				
extern void 		*cecfg_map_base;				 
extern void 		*cecfg_virt_addr;				
void seti2csdadir(uint8_t out_in);
void fpga_sda_dir(uint8_t sdadir);
void setad7606rden(uint8_t high_low);
void seti2csck(uint8_t high_low);
void seti2csda(uint8_t high_low);
void setfifoad7606_profull(uint8_t high_low);
void setfifoad7606_proempty(uint8_t high_low);

uint8_t read_fifoad7606_profull(void);
uint8_t read_fifoad7606_proempty(void);
uint8_t read_armi2c_sda(void);
void i2cgpio_init(void);
void adc_start(void);
void adc_stop(void);
void fpga_reset(void);
void setadc_samfre(uint8_t fre);
void set_adc_range(uint8_t range);

extern int 		memfd;
extern volatile uint16_t 		*emifinout; 
extern uint32_t 					* gpio2_datareg;  		
extern uint32_t 					* gpio2_dirreg;			
extern uint32_t 					* gpio2_indatareg;		

#endif

EXEC = mainapp
OBJS = main.o  getcfg.o setnetinfo.o ethpro.o emif.o  systime.o arm_gpio.o rx8010sj.o	
SRC  = main.c  getcfg.c setnetinfo.c ethpro.c emif.c  systime.c arm_gpio.c rx8010sj.c	
CC = /usr/arm-2009q1/bin/arm-none-linux-gnueabi-gcc  
CFLAGS +=   -I. -I/usr/arm-2009q1/arm-none-linux-gnueabi/include  -I/root/work/myapp/omap138_app/inc
all:$(EXEC)

$(EXEC):$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) -pthread

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ -pthread

clean:
	@rm -vf $(EXEC) *.o *~



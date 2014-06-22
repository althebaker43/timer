PROJECT=trinket_sample
CC=avr-gcc
OBJCOPY=avr-objcopy
MCU=attiny85

TIMER_ROOT=../..
TIMER_SOURCE=$(TIMER_ROOT)/src/TimerDriver.c

CFLAGS=-mmcu=$(MCU)
INCLUDE_DIRS=-I. -I$(TIMER_ROOT)/include

RESIDUE= \
	 $(PROJECT).elf \
	 $(PROJECT).hex

$(PROJECT).hex : $(PROJECT).elf
	$(OBJCOPY) -O ihex $< $(PROJECT).hex

$(PROJECT).elf :
	$(CC) -o $@ $(CFLAGS) $(INCLUDE_DIRS) $(PROJECT).c $(TIMER_SOURCE)

.PHONY : clean
clean :
	rm -f $(RESIDUE)

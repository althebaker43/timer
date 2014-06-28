PROJECT=trinket_sample
CC=avr-gcc
OBJCOPY=avr-objcopy
MCU=attiny85

TIMER_ROOT=../..
TIMER_SOURCE=$(TIMER_ROOT)/src/TimerDriver.c

CFLAGS=-Wall -Werror -mmcu=$(MCU)
INCLUDE_DIRS=-I. -I$(TIMER_ROOT)/include

RESIDUE= \
	 $(PROJECT).elf \
	 $(PROJECT).hex

$(PROJECT).hex : $(PROJECT).elf
	$(OBJCOPY) -O ihex $< $(PROJECT).hex

$(PROJECT).elf : $(PROJECT).c TargetSystem.o
	$(CC) -o $@ $(CFLAGS) $(INCLUDE_DIRS) $(PROJECT).c $(TIMER_SOURCE) TargetSystem.o

TargetSystem.o : TargetSystem.c TargetSystem.h
	$(CC) -c -o $@ $(CFLAGS) $<

.PHONY : program
program : $(PROJECT).hex
	avrdude -C avrdude.conf -c usbtiny -p attiny85 -U flash:w:$<

.PHONY : clean
clean :
	rm -f $(RESIDUE)

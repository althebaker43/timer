PROJECT=launchpad_sample
CC=msp430-gcc
MCU=msp430f5529
CORE=430x

TIMER_ROOT=../..
TIMER_SOURCE=$(TIMER_ROOT)/src/TimerDriver.c

CFLAGS=-Wall -Werror -mmcu=$(MCU) -mcpu=$(CORE) -mdisable-watchdog
INCLUDE_DIRS=-I. -I$(TIMER_ROOT)/include

RESIDUE= \
	 $(PROJECT).elf \
	 TargetSystem.o

$(PROJECT).elf : $(PROJECT).c TargetSystem.o
	$(CC) -o $@ $(CFLAGS) $(INCLUDE_DIRS) $(PROJECT).c $(TIMER_SOURCE) TargetSystem.o

TargetSystem.o : TargetSystem.c TargetSystem.h
	$(CC) -c -o $@ $(CFLAGS) $<

.PHONY : program
program : $(PROJECT).hex

.PHONY : clean
clean :
	rm -f $(RESIDUE)

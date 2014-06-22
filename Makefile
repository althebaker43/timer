# Copied from TDD for Embedded C source distribution

#Set this to @ to keep the makefile quiet
SILENCE = @

#---- Outputs ----#
COMPONENT_NAME = TimerDriver

#--- Inputs ----#
UNITY_HOME = /home/allen/Development/unity/unity.framework
CPP_PLATFORM = gcc
PROJECT_HOME_DIR = /home/allen/Development/timer
PROJECT_TEST_DIR = $(PROJECT_HOME_DIR)/test
UNITY_BUILD_HOME = $(UNITY_HOME)/extras/fixture/build

UNITY_CFLAGS += -DUNITY_OUTPUT_CHAR=UnityOutputCharSpy_OutputChar
UNITY_WARNINGFLAGS = -Wall -Werror -Wswitch-default
#UNITY_WARNINGFLAGS += -Wshadow 

SRC_DIRS = $(PROJECT_HOME_DIR)/src

TEST_SRC_DIRS =\
	       $(PROJECT_TEST_DIR)\
	       $(UNITY_HOME)/unity\
	       $(UNITY_HOME)/src\
	       $(UNITY_HOME)/extras/fixture/src\
	       $(UNITY_HOME)/extras/fixture/test

MOCKS_SRC_DIRS = $(PROJECT_TEST_DIR)/mocks

INCLUDE_DIRS =\
  .\
  $(UNITY_HOME)/src\
  $(UNITY_HOME)/extras/fixture/src\
  $(UNITY_HOME)/extras/fixture/test\
  $(PROJECT_HOME_DIR)/include\
  $(PROJECT_HOME_DIR)/mocks
  
include $(UNITY_BUILD_HOME)/MakefileWorker.mk

AVR_GCC=avr-gcc

AVR_RESIDUE= \
	     avr_$(COMPONENT_NAME).o

.PHONY : avr
avr : avr_$(COMPONENT_NAME).o

avr_$(COMPONENT_NAME).o : src/$(COMPONENT_NAME).c include/$(COMPONENT_NAME).h
	$(AVR_GCC) -c -o $@ $< -Iinclude -Itest/mocks

.PHONY : avr_clean
avr_clean :
	rm -f $(AVR_RESIDUE)

SAMPLE_ROOT=samples
SAMPLES= \
	 trinket

.PHONY : samples
samples : $(SAMPLES)

.PHONY : trinket_samples
$(SAMPLES) :
	$(MAKE) -C $(SAMPLE_ROOT)/$@

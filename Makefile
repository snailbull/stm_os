################################################################################
# Common Makefile
#  $^=all depend files   $@=target   $<=first depend files
################################################################################
CROSS_COMPILE	:= 
# /usr/local/arm/gcc-3.4.5-glibc-2.3.6/bin/arm-linux-
# 4.2.2-eabi/usr/bin/arm-linux-
AS		:= $(CROSS_COMPILE)as
LD		:= $(CROSS_COMPILE)ld
CC		:= $(CROSS_COMPILE)gcc
CPP		:= $(CROSS_COMPILE)g++
AR		:= $(CROSS_COMPILE)ar
NM		:= $(CROSS_COMPILE)nm
STRIP	:= $(CROSS_COMPILE)strip
OBJCOPY	:= $(CROSS_COMPILE)objcopy
OBJDUMP	:= $(CROSS_COMPILE)objdump
TOPDIR  := $(shell pwd)

################################################################################
# user config
TARGET := app.elf

CFLAGS :=  -O2 -g -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast
CFLAGS += -I. \
	-I$(TOPDIR)/fcmd	\
	-I$(TOPDIR)/tetris	\
	-I$(TOPDIR)/stm_os

LDFLAGS := \
	-lm \
	-pthread	\
	-lc	\
	-lreadline -lncurses

obj-y += \
	fcmd/			\
	stm_os/			\
	tetris/


################################################################################
# 
export AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP TOPDIR CFLAGS LDFLAGS

all : 
	make -f $(TOPDIR)/common.mk
	$(CC) $(LDFLAGS) -o $(TARGET) built-in.o

clean:
	rm -f $(shell find -name "*.o")

distclean:
	rm -f $(shell find -name "*.o")
	rm -f $(shell find -name "*.d")
	rm -f $(TARGET)
	
	
#*******************************************************************************
# Copyright (c) 2012 Transvideo.  All Rights Reserved
#
# THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF Transvideo.
# The copyright notice above does not evidence any actual or intended
# publication of such source code.
#
# Filename              : makefile
# Author				: N. Simon
# Date					: 20/03/2012
#*******************************************************************************/

#===============================================================================


# Compiler
CROSS_COMPILE=/opt/freescale/usr/local/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain/bin/arm-none-linux-gnueabi-
CC=$(CROSS_COMPILE)gcc
#---OR 

#CC = gcc

CFLAGS +=		$(TOOLCHAIN_CFLAGS) -fPIC

DEFINES = -DCUBE_LIBRARY 
#DEFINES +=	-DDEBUG


APP			=	cubeDecrypt
APP_SIZE    =   cubeSize
LIB_BIN     =	libCubeDecrypt-$(LIB_VERSION).so
LIB_VERSION	=	1.0.0

APP_OBJECT	=	$(APP_SOURCES:.c=.o)
APP_SOURCES	= 	main.c
APP_OBJECT_LIB = $(APP_OBJECT:.o=.lib.o)

#TARGET= $(APP)
TARGET= lib


.PHONY: all install clean

all: $(TARGET)
################
$(APP): $(APP_OBJECT)
	$(CC) $(APP_OBJECT) -o $(APP)

$(APP_OBJECT): $(APP_SOURCES)
	$(CC) -I. $(DEFINES) -c $(APP_SOURCES) -o $(APP_OBJECT)
################

################
$(APP_SIZE): app_size_obj
	$(CC) size.o -o $(APP_SIZE)

app_size_obj: $(APP_SOURCES)
	$(CC) -I. $(DEFINES) -DGET_SIZE_PROGRAM -c $(APP_SOURCES) -o size.o
################

################
lib: $(APP_OBJECT_LIB)
	$(CC) -shared -Wl,-soname,$(LIB_BIN) $(APP_OBJECT_LIB) -o $(LIB_BIN) $(LDFLAGS)
	
$(APP_OBJECT_LIB): $(APP_SOURCES)
	$(CC) -I. $(DEFINES) -c $(APP_SOURCES) $(CFLAGS) -o $@
################

install:
	install -m 644 cubeheader.h $(DEV_IMAGE)/usr/include
	install -m 755 $(LIB_BIN) $(DEV_IMAGE)/usr/lib	
	ln -fs  $(LIB_BIN) $(DEV_IMAGE)/usr/lib/libCubeDecrypt.so
	#copie a default LUT for Test purpose
	#install -m 644 mylutoutput.cube $(DEV_IMAGE)/home/cubeTest.cube

clean: 
	rm -rf $(APP_OBJECT)
	rm -rf $(APP_OBJECT_LIB)


distclean:
	rm -rf $(APP)
	rm -rf $(LIB_BIN)

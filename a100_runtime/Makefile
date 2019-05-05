ifndef ROOTDIR
ROOTDIR = ../..
export ROOTDIR
endif

LIB_PATH = $(ROOTDIR)/lib/lib

LIBS+= -L$(ROOTDIR)/lib/libnvram -lpthread -lnvram -lm -lc
INCLUDES+= -I$(ROOTDIR)/lib/libnvram/ -I$(ROOTDIR)/$(LINUXDIR)/include -I$(ROOTDIR)/config

CFLAGS = -c -Wall -g -fPIC

TARGET=/opt/buildroot-gcc463/usr/bin/mipsel-linux-
ELFFILE=libzlink_runtime.so

CPP=$(TARGET)g++
CC=$(TARGET)gcc
AR=$(TARGET)ar
LD=$(TARGET)ld
STRIP=$(TARGET)strip


SOURCES = $(wildcard ./*.c) 
OBJS = $(SOURCES:.c=.o)

all: $(ELFFILE)

$(ELFFILE): $(OBJS)
	$(CC) -shared -o $(ELFFILE) $(LIBS) $(INCLUDES) $(OBJS)
	$(STRIP) $(ELFFILE)
	install -m 644 $(ELFFILE) $(LIB_PATH)

.c.o:
	$(CC) $(CFLAGS) $(LIBS) $(INCLUDES) -o $@ -c $<

clean:
	rm -rf *.o *.so
	
strip:
	$(STRIP) $(ELFFILE)

romfs:
	$(ROMFSINST) /lib/$(ELFFILE)


# Makefile

ifndef ROOTDIR
ROOTDIR = ../..
export ROOTDIR
endif

LIB_PATH = $(ROOTDIR)/lib/lib

##LIBS+= -L$(LIB_PATH) -L$(ROOTDIR)/romfs/lib/ -L$(ROOTDIR)/lib/libnvram -L $(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -lgcc -lpthread -lnvram -lm -lc
LIBS+= -L$(LIB_PATH) -L$(ROOTDIR)/romfs/lib/ -L$(ROOTDIR)/lib/libnvram -lpthread -lnvram -lm -lc 
INCLUDES+=  -I$(ROOTDIR)/user/ont_cloud/ -I$(ROOTDIR)/lib/libnvram/

LINUX_CONF_H= $(ROOTDIR)/$(LINUXDIR)/include/linux/autoconf.h

TARGET=/opt/buildroot-gcc463/usr/bin/mipsel-linux-
ELFFILE=ont_cloud

CPP=$(TARGET)g++
CC=$(TARGET)gcc
AR=$(TARGET)ar
LD=$(TARGET)ld
STRIP=$(TARGET)strip

CFLAGS += -Wall -g -O2 


ALL_OBJS += allocator.o device.o log.o ont_buffer.o ont_edp.o ont_edp_private.o ont_list.o ont_tcp_channel.o \
		packet.o parser.o persistence_util.o string_util.o tcp_util.o time_util.o ont_pkt_formatter.o sample-linux.o \
		ont_handle.o cJSON.o ont_upgrade.o ont_app.o base64.o MAC_VENDOR.o router_cfg.o utils.o aes_cbc.o aes_core.o \
		ont_aes_ecb.o md5.o



MODULE = $(shell pwd | sed "s/.*\///" )

all: $(MODULE)

$(MODULE): $(ALL_OBJS)
	$(CC) $(CFLAGS) $(ALL_OBJS) $(LIBS) -o $@
	$(CC) -o $(MODULE) $(ALL_OBJS) $(LIBS)

%.o: %.c $(LINUX_CONF_H)
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

romfs:
	$(ROMFSINST) /bin/ont_cloud
clean:
	rm $(MODULE) $(ALL_OBJS)

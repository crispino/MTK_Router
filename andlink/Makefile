TARGET=/opt/buildroot-gcc463/usr/bin/mipsel-linux-
ELFFILE=andlink

CPP=$(TARGET)g++
CC=$(TARGET)gcc
AR=$(TARGET)ar
LD=$(TARGET)ld
STRIP=$(TARGET)strip
OBJS+=qlink_demo.o
HEADERS+=Qlink_API.h

LDFLAGS = -L ./libcoap-4.1.1 -lcoap -pthread -lm
LDFLAGS += -L$(ROOTDIR)/lib/libnvram -L. -lnvram

CFLAGS = -I$(ROOTDIR)/$(LINUXDIR)/include -I$(ROOTDIR)/lib/libnvram -I$(ROOTDIR)/config

$(ELFFILE) : $(OBJS)
	cd ./libcoap-4.1.1;make -f Makefile_QLink CC=$(CC) AR=$(AR) LD=$(LD) BUILD_SO=1
	$(CC) -O2 -o $@ $^ $(LDFLAGS)
	#cp $(ELFFILE) $(ROMFSDIR)/bin

qlink_demo.o : qlink_demo.c  Qlink_API.h
	$(CC) $(INCLUDE) $(CFLAGS) $(LDFLAGS) -c -o $@ $<
strip:
	$(STRIP) $(ELFFILE)

romfs:
	$(ROMFSINST) /bin/$(ELFFILE)

.PHONY: clean 
clean:
	-rm -f $(ELFFILE) *.o
	cd ./libcoap-4.1.1;make -f Makefile_QLink clean

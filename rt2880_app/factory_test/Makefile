EXEC = factory
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/char
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/raeth
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/include/linux/autoconf.h

all: $(EXEC)

$(EXEC): $(EXEC).c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o


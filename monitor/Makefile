EXE = monitor
OBJS = monitor.o clientFlow.o

#stdlibc++ known issue: 
#If your program  needs libstdc++, please turn off code optimization
CFLAGS += -O0 -I$(ROOTDIR)/$(LINUXDIR)/drivers/char
LIBS+= -L$(LIB_PATH) -L$(ROOTDIR)/romfs/lib/ -L$(ROOTDIR)/lib/libnvram -lpthread -lnvram -lm -lc
INCLUDES+= -I$(ROOTDIR)/lib/libnvram/

all: $(OBJS)
	$(CXX) -o $(EXE) $(OBJS) $(LIBS) $(LDFLAGS)

.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< 

romfs:
	$(ROMFSINST) /bin/$(EXE)
	$(ROMFSINST) /sbin/client_flow_stats.sh
	chmod 775 $(ROMFSDIR)/sbin/client_flow_stats.sh
	$(ROMFSINST) /etc_ro/client_flow_stats.txt
	chmod 775 $(ROMFSDIR)/etc_ro/client_flow_stats.txt
	
clean:
	@rm -rf $(EXE)  *.o


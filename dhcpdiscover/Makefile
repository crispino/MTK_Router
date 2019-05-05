EXE = dhcpdiscover
OBJS = dhcpdiscover.o

#stdlibc++ known issue: 
#If your program  needs libstdc++, please turn off code optimization
CFLAGS += -O0
LIBS+= -L$(LIB_PATH) -L$(ROOTDIR)/romfs/lib/ -L$(ROOTDIR)/lib/libnvram -lpthread -lnvram -lm -lc
INCLUDES+= -I$(ROOTDIR)/lib/libnvram/

all: $(OBJS)
	$(CXX) -o $(EXE) $(OBJS) $(LIBS) $(LDFLAGS)

.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< 

romfs:
	$(ROMFSINST) /bin/$(EXE)

clean:
	@rm -rf $(EXE)  *.o


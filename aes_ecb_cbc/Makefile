TARGET=/opt/buildroot-gcc463/usr/bin/mipsel-linux-

LIBSO=libaes.so
LIBA=libaes.a
EXE=Aes_ecb_cbc

CPP=$(TARGET)g++
CC=$(TARGET)gcc
AR=$(TARGET)ar
LD=$(TARGET)ld
STRIP=$(TARGET)strip

LDFLAGS =

CFLAGS = -I./

SOURCES:= aes_ecb_cbc_test.c aes_cbc.c aes_core.c  aes_ecb.c aes_ige.c aes_misc.c aes_wrap.c \
		cbc128.c cryptlib.c stack.c err.c buf_str.c mem_clr.c mem.c lhash.c mem_dbg.c \
		obj_dat.c o_init.c b_print.c bio_lib.c bss_file.c a_object.c obj_lib.c asn1_lib.c \
		bn_word.c bn_lib.c bn_shift.c bn_print.c bn_asm.c ex_data.c base64.c

OBJECTS:= $(patsubst %.c, %.o, $(SOURCES))

#all: $(LIBSO) $(LIBA) $(EXE) strip
all: $(EXE)

$(LIBSO): $(OBJECTS)
	$(CC) -fPIC -shared -o $^

$(LIBA): $(OBJECTS)
	$(AR) $(ARFLAGS) $^

$(EXE) : $(OBJECTS)
	$(CC) -O2 -o $@ $^ $(LDFLAGS)

strip:
	$(STRIP) $(LIBSO)
	$(STRIP) $(LIBA)

romfs install:


.PHONY: clean 
clean:
	-rm -f $(EXE) *.o *.so *.a

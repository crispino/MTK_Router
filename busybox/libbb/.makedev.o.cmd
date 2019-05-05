cmd_libbb/makedev.o := /opt/buildroot-gcc463/usr/bin/mipsel-linux-gcc -Wp,-MD,libbb/.makedev.o.d   -std=gnu99 -Iinclude -Ilibbb  -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG  -D"BB_VER=KBUILD_STR(1.12.1)" -DBB_BT=AUTOCONF_TIMESTAMP -O2 -fomit-frame-pointer -pipe  -Dlinux -D__linux__ -Dunix -DEMBED -I/home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include -I/home/xiaoning/CMCC_CY/source/lib/include -DCONFIG_UCLIBC_0_9_33_2 -I/home/xiaoning/CMCC_CY/source  -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement -Wold-style-definition -fno-builtin-strlen -finline-limit=0 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -Os -Dlinux    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(makedev)"  -D"KBUILD_MODNAME=KBUILD_STR(makedev)" -c -o libbb/makedev.o libbb/makedev.c

deps_libbb/makedev.o := \
  libbb/makedev.c \
  include/platform.h \
    $(wildcard include/config/werror.h) \
    $(wildcard include/config///.h) \
    $(wildcard include/config/nommu.h) \
    $(wildcard include/config//nommu.h) \
    $(wildcard include/config//mmu.h) \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/byteswap.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/byteswap.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/byteswap-common.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/endian.h \
    $(wildcard include/config/.h) \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/features.h \
    $(wildcard include/config/c99.h) \
    $(wildcard include/config/c95.h) \
    $(wildcard include/config/ix.h) \
    $(wildcard include/config/ix2.h) \
    $(wildcard include/config/ix199309.h) \
    $(wildcard include/config/ix199506.h) \
    $(wildcard include/config/en.h) \
    $(wildcard include/config/en/extended.h) \
    $(wildcard include/config/x98.h) \
    $(wildcard include/config/en2k.h) \
    $(wildcard include/config/en2k8.h) \
    $(wildcard include/config/gefile.h) \
    $(wildcard include/config/gefile64.h) \
    $(wildcard include/config/e/offset64.h) \
    $(wildcard include/config/d.h) \
    $(wildcard include/config/c.h) \
    $(wildcard include/config/ile.h) \
    $(wildcard include/config/ntrant.h) \
    $(wildcard include/config/tify/level.h) \
    $(wildcard include/config/i.h) \
    $(wildcard include/config/ern/inlines.h) \
    $(wildcard include/config/ix/implicitly.h) \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/uClibc_config.h \
    $(wildcard include/config/mips/o32/abi//.h) \
    $(wildcard include/config/mips/n32/abi//.h) \
    $(wildcard include/config/mips/n64/abi//.h) \
    $(wildcard include/config/mips/isa/1//.h) \
    $(wildcard include/config/mips/isa/2//.h) \
    $(wildcard include/config/mips/isa/3//.h) \
    $(wildcard include/config/mips/isa/4//.h) \
    $(wildcard include/config/mips/isa/mips32//.h) \
    $(wildcard include/config/mips/isa/mips32r2//.h) \
    $(wildcard include/config/mips/isa/mips64//.h) \
    $(wildcard include/config//.h) \
    $(wildcard include/config/link//.h) \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/cdefs.h \
    $(wildcard include/config/espaces.h) \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/endian.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/arpa/inet.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/netinet/in.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/stdint.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/wchar.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/wordsize.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/socket.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/uio.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/types.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/types.h \
  /opt/buildroot-gcc463/usr/lib/gcc/mipsel-buildroot-linux-uclibc/4.6.3/include/stddef.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/typesizes.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/pthreadtypes.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/sched.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/time.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/select.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/select.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/sigset.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/time.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/sysmacros.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/uio.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/socket.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/limits.h \
  /home/xiaoning/CMCC_CY/source/lib/include/limits.h \
  /opt/buildroot-gcc463/usr/lib/gcc/mipsel-buildroot-linux-uclibc/4.6.3/include-fixed/limits.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/posix1_lim.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/local_lim.h \
  /home/xiaoning/CMCC_CY/source/lib/include/linux/limits.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/uClibc_local_lim.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/posix2_lim.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/xopen_lim.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/stdio_lim.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/sockaddr.h \
  /home/xiaoning/CMCC_CY/source/lib/include/asm/socket.h \
  /home/xiaoning/CMCC_CY/source/lib/include/asm/sockios.h \
  /home/xiaoning/CMCC_CY/source/lib/include/asm/ioctl.h \
  /home/xiaoning/CMCC_CY/source/lib/include/asm-generic/ioctl.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/in.h \
  /opt/buildroot-gcc463/usr/lib/gcc/mipsel-buildroot-linux-uclibc/4.6.3/include/stdbool.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/mount.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/ioctl.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/ioctls.h \
  /home/xiaoning/CMCC_CY/source/lib/include/asm/ioctls.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/ioctl-types.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/ttydefaults.h \

libbb/makedev.o: $(deps_libbb/makedev.o)

$(deps_libbb/makedev.o):

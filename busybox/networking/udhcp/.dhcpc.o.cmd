cmd_networking/udhcp/dhcpc.o := /opt/buildroot-gcc463/usr/bin/mipsel-linux-gcc -Wp,-MD,networking/udhcp/.dhcpc.o.d   -std=gnu99 -Iinclude -Ilibbb  -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG  -D"BB_VER=KBUILD_STR(1.12.1)" -DBB_BT=AUTOCONF_TIMESTAMP -O2 -fomit-frame-pointer -pipe  -Dlinux -D__linux__ -Dunix -DEMBED -I/home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include -I/home/xiaoning/CMCC_CY/source/lib/include -DCONFIG_UCLIBC_0_9_33_2 -I/home/xiaoning/CMCC_CY/source  -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement -Wold-style-definition -fno-builtin-strlen -finline-limit=0 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -Os -Dlinux    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(dhcpc)"  -D"KBUILD_MODNAME=KBUILD_STR(dhcpc)" -c -o networking/udhcp/dhcpc.o networking/udhcp/dhcpc.c

deps_networking/udhcp/dhcpc.o := \
  networking/udhcp/dhcpc.c \
    $(wildcard include/config/feature/pidfile.h) \
    $(wildcard include/config/ture/udhcp/port.h) \
    $(wildcard include/config/getopt/long.h) \
    $(wildcard include/config/ture/udhcpc/arping.h) \
    $(wildcard include/config//mmu.h) \
    $(wildcard include/config/opt/long.h) \
    $(wildcard include/config/feature/udhcp/port.h) \
    $(wildcard include/config/feature/udhcpc/arping.h) \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/syslog.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/syslog.h \
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
    $(wildcard include/config///.h) \
    $(wildcard include/config//.h) \
    $(wildcard include/config/link//.h) \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/cdefs.h \
    $(wildcard include/config/espaces.h) \
  /opt/buildroot-gcc463/usr/lib/gcc/mipsel-buildroot-linux-uclibc/4.6.3/include/stdarg.h \
  networking/udhcp/common.h \
    $(wildcard include/config/dhcpc/default/script.h) \
    $(wildcard include/config/udhcpc/slack/for/buggy/servers.h) \
    $(wildcard include/config/feature/udhcp/debug.h) \
  include/libbb.h \
    $(wildcard include/config/selinux.h) \
    $(wildcard include/config/locale/support.h) \
    $(wildcard include/config/use/bb/pwd/grp.h) \
    $(wildcard include/config/feature/shadowpasswds.h) \
    $(wildcard include/config/use/bb/shadow.h) \
    $(wildcard include/config/lfs.h) \
    $(wildcard include/config/feature/buffers/go/on/stack.h) \
    $(wildcard include/config/buffer.h) \
    $(wildcard include/config/ubuffer.h) \
    $(wildcard include/config/feature/buffers/go/in/bss.h) \
    $(wildcard include/config/inux.h) \
    $(wildcard include/config/feature/ipv6.h) \
    $(wildcard include/config/feature/check/names.h) \
    $(wildcard include/config/feature/prefer/applets.h) \
    $(wildcard include/config/busybox/exec/path.h) \
    $(wildcard include/config/feature/syslog.h) \
    $(wildcard include/config/feature/individual.h) \
    $(wildcard include/config/o.h) \
    $(wildcard include/config/ntf.h) \
    $(wildcard include/config/t.h) \
    $(wildcard include/config/l.h) \
    $(wildcard include/config/wn.h) \
    $(wildcard include/config/ktop.h) \
    $(wildcard include/config/route.h) \
    $(wildcard include/config/feature/hwib.h) \
    $(wildcard include/config/debug/crond/option.h) \
    $(wildcard include/config/use/bb/crypt.h) \
    $(wildcard include/config/ioctl/hex2str/error.h) \
    $(wildcard include/config/feature/editing.h) \
    $(wildcard include/config/feature/editing/history.h) \
    $(wildcard include/config/ture/editing/savehistory.h) \
    $(wildcard include/config/feature/editing/savehistory.h) \
    $(wildcard include/config/feature/tab/completion.h) \
    $(wildcard include/config/feature/username/completion.h) \
    $(wildcard include/config/feature/editing/vi.h) \
    $(wildcard include/config/feature/topmem.h) \
    $(wildcard include/config/feature/top/smp/process.h) \
    $(wildcard include/config/pgrep.h) \
    $(wildcard include/config/pkill.h) \
    $(wildcard include/config/pidof.h) \
    $(wildcard include/config/feature/devfs.h) \
  include/platform.h \
    $(wildcard include/config/werror.h) \
    $(wildcard include/config/nommu.h) \
    $(wildcard include/config//nommu.h) \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/byteswap.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/byteswap.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/byteswap-common.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/endian.h \
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
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/ctype.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/uClibc_touplow.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/dirent.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/dirent.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/errno.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/errno.h \
  /home/xiaoning/CMCC_CY/source/lib/include/linux/errno.h \
  /home/xiaoning/CMCC_CY/source/lib/include/asm/errno.h \
  /home/xiaoning/CMCC_CY/source/lib/include/asm-generic/errno-base.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/syscall.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/sysnum.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/fcntl.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/fcntl.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sgidefs.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/stat.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/stat.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/inttypes.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/netdb.h \
    $(wildcard include/config/3/ascii/rules.h) \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/rpc/netdb.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/siginfo.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/netdb.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/setjmp.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/setjmp.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/signal.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/signum.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/sigaction.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/sigcontext.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/sigstack.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/ucontext.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/sigthread.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/stdio.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/uClibc_stdio.h \
    $(wildcard include/config/io/futexes//.h) \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/wchar.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/uClibc_mutex.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/pthread.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sched.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/uClibc_clk_tck.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/initspin.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/uClibc_pthread.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/stdlib.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/waitflags.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/waitstatus.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/alloca.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/string.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/poll.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/poll.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/mman.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/mman.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/time.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/wait.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/resource.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/resource.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/termios.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/termios.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/unistd.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/posix_opt.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/environments.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/confname.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/getopt.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/utime.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/param.h \
  /home/xiaoning/CMCC_CY/source/lib/include/linux/param.h \
  /home/xiaoning/CMCC_CY/source/lib/include/asm/param.h \
  /home/xiaoning/CMCC_CY/source/lib/include/asm-generic/param.h \
    $(wildcard include/config/hz.h) \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/mntent.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/paths.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/sys/statfs.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/bits/statfs.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/pwd.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/grp.h \
  include/xatonum.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/netinet/udp.h \
  /home/xiaoning/CMCC_CY/source/uClibc-0.9.33.2/app_headers/include/netinet/ip.h \
  networking/udhcp/dhcpd.h \
    $(wildcard include/config/dhcpd/leases/file.h) \
  networking/udhcp/dhcpc.h \
  networking/udhcp/options.h \
    $(wildcard include/config/feature/rfc3397.h) \

networking/udhcp/dhcpc.o: $(deps_networking/udhcp/dhcpc.o)

$(deps_networking/udhcp/dhcpc.o):

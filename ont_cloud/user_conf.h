/*
 * Automatically generated C config: don't edit
 */
#define AUTOCONF_INCLUDED

/*
 * Library Configuration
 */

/*
 * Force build (Normally built when required)
 */
#undef  CONFIG_LIB_LIBGMP_FORCE
#undef  CONFIG_LIB_LIBPTHREAD_FORCE
#define CONFIG_LIB_LIBNVRAM_FORCE 1
#undef  CONFIG_LIB_LIBNTFS3G_FORCE
#undef  CONFIG_LIB_LIBBT_FORCE
#undef  CONFIG_LIB_READLINE_FORCE
#undef  CONFIG_LIB_NL_FORCE
#undef  CONFIG_LIB_LIBPCAP_FORCE
#undef  CONFIG_LIB_LIBEVENT_FORCE
#undef  CONFIG_LIB_NCURSE_FORCE
#define CONFIG_LIB_FLEX_FORCE 1
#undef  CONFIG_LIB_LIBUPNP_FORCE
#undef  CONFIG_LIB_LIBDLNA_FORCE
#undef  CONFIG_LIB_LIBFFMPEG_FORCE
#undef  CONFIG_LIB_LIBGDBM_FORCE
#undef  CONFIG_LIB_LIBID3TAG_FORCE
#undef  CONFIG_LIB_LIBSQLITE3_FORCE
#undef  CONFIG_LIB_LIBEXIF_FORCE
#undef  CONFIG_LIB_LIBOGG_FORCE
#undef  CONFIG_LIB_LIBVORBIS_FORCE
#undef  CONFIG_LIB_LIBFLAC_FORCE
#undef  CONFIG_LIB_LIBJPEG_FORCE
#undef  CONFIG_LIB_LIBDB_FORCE
#undef  CONFIG_LIB_USB0112_FORCE
#undef  CONFIG_LIB_USB100_FORCE
#define CONFIG_LIB_PCRE_FORCE 1
#undef  CONFIG_LIB_VSTR_FORCE
#define CONFIG_LIB_ZLIB_FORCE 1
#undef  CONFIG_LIB_LZO_FORCE
#undef  CONFIG_LIB_E2FSPROGS_FORCE
#undef  CONFIG_LIB_LIBSTDC_PLUS_FORCE
#undef  CONFIG_USER_HELLO_WORLD

/*
 * MTD utils
 */
#undef  CONFIG_USER_MTDUTILS
#undef  CONFIG_USER_MTDUTILS_DOCFDISK
#undef  CONFIG_USER_MTDUTILS_DOC_LOADBIOS
#undef  CONFIG_USER_MTDUTILS_FLASHCP
#undef  CONFIG_USER_MTDUTILS_FLASH_ERASE
#undef  CONFIG_USER_MTDUTILS_FLASH_ERASEALL
#undef  CONFIG_USER_MTDUTILS_FLASH_LOCK
#undef  CONFIG_USER_MTDUTILS_FLASH_OTP_DUMP
#undef  CONFIG_USER_MTDUTILS_FLASH_OTP_INFO
#undef  CONFIG_USER_MTDUTILS_FLASH_UNLOCK
#undef  CONFIG_USER_MTDUTILS_FTL_CHECK
#undef  CONFIG_USER_MTDUTILS_FTL_FORMAT
#undef  CONFIG_USER_MTDUTILS_JFFS2DUMP
#undef  CONFIG_USER_MTDUTILS_JFFS2READER
#undef  CONFIG_USER_MTDUTILS_MKFSJFFS2
#undef  CONFIG_USER_MTDUTILS_MTD_DEBUG
#undef  CONFIG_USER_MTDUTILS_NANDDUMP
#undef  CONFIG_USER_MTDUTILS_NANDTEST
#undef  CONFIG_USER_MTDUTILS_NANDWRITE
#undef  CONFIG_USER_MTDUTILS_NFTLDUMP
#undef  CONFIG_USER_MTDUTILS_NFTL_FORMAT
#undef  CONFIG_USER_MTDUTILS_RECV_IMAGE
#undef  CONFIG_USER_MTDUTILS_RFDDUMP
#undef  CONFIG_USER_MTDUTILS_RFDFORMAT
#undef  CONFIG_USER_MTDUTILS_SERVE_IMAGE
#undef  CONFIG_USER_MTDUTILS_SUMTOOL
#undef  CONFIG_USER_MTDUTILS_MKFSUBIFS
#undef  CONFIG_USER_MTDUTILS_MTDINFO
#undef  CONFIG_USER_MTDUTILS_UBIATTACH
#undef  CONFIG_USER_MTDUTILS_UBICRC32
#undef  CONFIG_USER_MTDUTILS_UBIDETACH
#undef  CONFIG_USER_MTDUTILS_UBIFORMAT
#undef  CONFIG_USER_MTDUTILS_UBIMKVOL
#undef  CONFIG_USER_MTDUTILS_UBINFO
#undef  CONFIG_USER_MTDUTILS_UBINIZE
#undef  CONFIG_USER_MTDUTILS_UBIRENAME
#undef  CONFIG_USER_MTDUTILS_UBIRMVOL
#undef  CONFIG_USER_MTDUTILS_UBIRSVOL
#undef  CONFIG_USER_MTDUTILS_UBIUPDATEVOL

/*
 * Network Applications
 */
#undef  CONFIG_USER_3G
#define CONFIG_USER_ACCEL_PPTP 1
#define CONFIG_USER_PPPD_WITH_PPTP 1
#undef  CONFIG_USER_ARPTABLES
#undef  CONFIG_USER_BRCTL_BRCTL
#undef  CONFIG_USER_BIGPOND
#undef  CONFIG_USER_BT_UTILS
#undef  CONFIG_USER_BT_AVINFO
#undef  CONFIG_USER_BT_BCCMD
#undef  CONFIG_USER_BT_CIPTOOL
#undef  CONFIG_USER_BT_HCICONFIG
#undef  CONFIG_USER_BT_HCIATTACH
#undef  CONFIG_USER_BT_HCISECFILTER
#undef  CONFIG_USER_BT_HCITOOL
#undef  CONFIG_USER_BT_L2PING
#undef  CONFIG_USER_BT_PPPORC
#undef  CONFIG_USER_BT_SDPTOOL
#undef  CONFIG_USER_CTORRENT
#undef  CONFIG_USER_CPU
#undef  CONFIG_USER_DHCP6
#undef  CONFIG_USER_DHCP6_SERVER
#undef  CONFIG_USER_DHCP6_CLIENT
#undef  CONFIG_USER_WIDE_DHCPV6
#undef  CONFIG_USER_ISC_DHCP
#undef  CONFIG_USER_DROPBEAR
#undef  CONFIG_USER_DNSMASQ
#undef  CONFIG_USER_DISKTYPE
#undef  CONFIG_USER_ECHO_SERVER
#undef  CONFIG_USER_EBTABLES
#undef  CONFIG_USER_HOSTAPD
#define CONFIG_USER_STORAGE 1
#undef  CONFIG_USER_PROFTPD
#define CONFIG_USER_SAMBA 1
#define CONFIG_SAMBA_3_0_2 1
#undef  CONFIG_SAMBA_4_0_24
#define CONFIG_LIB_NSL_FORCE 1
#undef  CONFIG_USER_NMBD_ONLY
#define CONFIG_USER_SAMBA_ALL 1
#undef  CONFIG_USER_MINIDLNA
#define CONFIG_USER_LIGHTY 1
#define CONFIG_LIB_PCRE_FORCE 1
#define CONFIG_LIB_ZLIB_FORCE 1
#undef  CONFIG_USER_ECMH
#define CONFIG_USER_IGMP_PROXY 1
#undef  CONFIG_USER_INADYN
#define CONFIG_USER_IPTABLES_IPTABLES 1
#define CONFIG_LIB_LIBM_FORCE 1
#undef  CONFIG_USER_IPTABLES_IP6TABLES
#undef  CONFIG_USER_IPROUTE2
#undef  CONFIG_USER_IPROUTE2_SS
#undef  CONFIG_USER_IPROUTE2_ARPD
#undef  CONFIG_USER_IPROUTE2_NSTAT
#undef  CONFIG_USER_IPROUTE2_IFSTAT
#undef  CONFIG_USER_IPROUTE2_RTACCT
#undef  CONFIG_USER_IPROUTE2_LNSTAT
#undef  CONFIG_USER_IPROUTE2_IP
#undef  CONFIG_USER_IPROUTE2_RTMON
#undef  CONFIG_USER_IPROUTE2_TC
#undef  CONFIG_USER_MATRIXSSL
#define CONFIG_USER_MINIUPNPD 1
#undef  CONFIG_USER_MKDOSFS
#define CONFIG_USER_NTPCLIENT_NTPCLIENT 1
#undef  CONFIG_USER_NTPCLIENT_ADJTIMEX
#undef  CONFIG_USER_OPENSSL
#define CONFIG_USER_OPENL2TP 1
#define CONFIG_USER_PPPD_WITH_L2TP 1
#define CONFIG_LIB_FLEX_FORCE 1
#undef  CONFIG_USER_IPSEC_TOOLS
#undef  CONFIG_USER_OPENSWAN
#undef  CONFIG_USER_P910ND
#define CONFIG_USER_PPPD 1
#define CONFIG_USER_PPPD_WITH_PPPOE 1
#define CONFIG_USER_PPPD_WITH_L2TP 1
#define CONFIG_USER_PPPD_WITH_PPTP 1
#undef  CONFIG_USER_PPPD_PPPSTATS
#undef  CONFIG_USER_PPPD_PPPDUMP
#undef  CONFIG_USER_PPPD_PPPCHAT
#undef  CONFIG_USER_RPPPPOE_RELAY
#undef  CONFIG_USER_RPPPPOE_SNIFF
#undef  CONFIG_USER_PPPOECD
#undef  CONFIG_USER_PPTP_CLIENT
#undef  CONFIG_USER_ZEBRA
#undef  CONFIG_LIB_LIBZEBRA_FORCE
#undef  CONFIG_USER_RP_L2TP
#define CONFIG_USER_RADVD 1
#undef  CONFIG_USER_RADVD_DUMP
#define CONFIG_USER_802_1X 1
#undef  CONFIG_USER_RT2561_802_1X
#undef  CONFIG_USER_SNORT
#undef  CONFIG_USER_SNMPD
#undef  CONFIG_USER_UVC_STREAM
#undef  CONFIG_USER_UPNP_IGD
#undef  CONFIG_USER_WPA_SUPPLICANT
#undef  CONFIG_USER_WSC

/*
 * Wireless-tools
 */
#define CONFIG_USER_WIRELESS_TOOLS 1
#define CONFIG_USER_WIRELESS_TOOLS_IWCONFIG 1
#undef  CONFIG_USER_WIRELESS_TOOLS_IWGETID
#undef  CONFIG_USER_WIRELESS_TOOLS_IWLIST
#define CONFIG_USER_WIRELESS_TOOLS_IWPRIV 1
#undef  CONFIG_USER_WIRELESS_TOOLS_IWSPY
#undef  CONFIG_USER_WIRELESS_TOOLS_IWEVENT

/*
 * Miscellaneous Applications
 */
#define CONFIG_USER_BUSYBOX_BUSYBOX 1
#undef  CONFIG_USER_BONNIE
#undef  CONFIG_USER_ETHTOOL
#undef  CONFIG_DHRYSTONE
#undef  CONFIG_DHCPFWD
#undef  CONFIG_USER_GDBREPLAY
#undef  CONFIG_USER_GDBSERVER
#undef  CONFIG_USER_IXIA_ENDPOINT
#define CONFIG_USER_IPERF 1
#define CONFIG_LIB_LIBM_FORCE 1
#undef  CONFIG_USER_I2C_TOOLS
#define CONFIG_USER_MTD_WRITE 1
#undef  CONFIG_USER_MPSTAT
#undef  CONFIG_USER_NETSTAT_NAT
#undef  CONFIG_USER_NTFS3G
#undef  CONFIG_USER_NTFSPROGS
#undef  CONFIG_LIB_LIBNTFS3G_FORCE
#undef  CONFIG_USER_STRACE_STRACE
#undef  CONFIG_USER_TCPDUMP
#undef  CONFIG_USER_TASKSET
#define CONFIG_USER_PCIUTIL_LSPCI 1
#undef  CONFIG_USER_PCIUTIL_SETPCI
#define CONFIG_LIB_ZLIB_FORCE 1
#undef  CONFIG_USER_LSUSB
#undef  CONFIG_USER_USBMODESWITCH
#undef  CONFIG_USER_COMGT
#undef  CONFIG_USER_SDPARM
#undef  CONFIG_USER_WATCHDOG
#undef  CONFIG_USER_VMSTAT

/*
 * Proprietary Application
 */
#define CONFIG_RALINKAPP 1
#define CONFIG_RALINKAPP_ATED 1
#define CONFIG_RALINKAPP_REG_RW 1
#undef  CONFIG_RALINKAPP_CSR
#undef  CONFIG_RALINKAPP_FLASH
#undef  CONFIG_FLASH_TOOL
#undef  CONFIG_ETHMAC_TOOL
#undef  CONFIG_RALINKAPP_HWNAT
#define CONFIG_RALINKAPP_MII_MGR 1
#define CONFIG_RALINKAPP_NVRAM 1
#define CONFIG_LIB_LIBNVRAM_FORCE 1
#undef  CONFIG_RALINKAPP_L2_MANAGE
#define CONFIG_RALINKAPP_GPIO 1
#undef  CONFIG_RALINKAPP_SPI
#undef  CONFIG_RALINKAPP_I2C
#undef  CONFIG_RALINKAPP_MEMORY_USAGE
#undef  CONFIG_USER_QOS
#undef  CONFIG_RALINKAPP_SWQOS
#undef  CONFIG_RALINKAPP_SUPERDMZ
#define CONFIG_RALINKAPP_SWITCH 1
#define CONFIG_RALINKAPP_LONG_LOOP 1
#undef  CONFIG_RALINKAPP_I2S
#undef  CONFIG_RALINKAPP_PCM
#undef  CONFIG_RALINKAPP_SPDIF
#undef  CONFIG_RALINKAPP_PCIE_PHY
#undef  CONFIG_RALINKAPP_CLS
#define CONFIG_RALINKAPP_SCRIPTS 1
#define CONFIG_RALINKAPP_ETHPHY 1
#undef  CONFIG_RALINKAPP_KPROFILE
#undef  CONFIG_USER_NFCSD
#undef  CONFIG_USER_NFCHOD

/*
 * Windows Rally Program
 */
#define CONFIG_USER_LLTD 1

/*
 * Phone Management
 */
#undef  CONFIG_MTK_VOIP

/*
 * Remote Management
 */
#undef  CONFIG_MTK_TR069
#define CONFIG_VENDOR "Ralink"
#define CONFIG_PRODUCT "MT7628"
#define CONFIG_LINUXDIR "linux-2.6.36.x"
#define CONFIG_LIBCDIR ""
#define CONFIG_LANGUAGE ""
#define VENDORS_AUTOCONF_INCLUDED
#undef AUTOCONF_INCLUDED

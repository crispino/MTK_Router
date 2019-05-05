#include "utils.h"
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <linux/wireless.h>
#include "oid.h"
#include "user_conf.h"
#include <sys/socket.h>
#include <linux/netlink.h>
#if defined CONFIG_MTK_VOIP
#include "openssl/pem.h"
#include "openssl/x509.h"
#include "openssl/evp.h"
#endif
#if defined CONFIG_USER_STORAGE
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif


#define CMD_NVRAM_GET 		0x1
#define CMD_BUILD_GET 		0x2
#define CMD_WIFI_GET 		0x3
#define CMD_SYS_GET 		0x4
#define CMD_FIREWALL_GET	0x5
#define CMD_VOIP_GET	0x6
#define CMD_TR069_GET	0x7
#define CMD_AUTO_WAN_GET	0x8
#define CMD_TEST_GET		0x9
#define CMD_QOS_GET		0xA


#define TXBYTE          	0
#define TXPACKET        	1
#define RXBYTE          	2
#define RXPACKET        	3
#define PROTO_UNKNOWN   	0
#define PROTO_TCP               1
#define PROTO_UDP               2
#define PROTO_TCP_UDP   	3
#define PROTO_ICMP              4
#define PROTO_NONE              5
#define ACTION_DROP             0
#define ACTION_ACCEPT   	1

#define WLAN1_CONF		"2860"
#define WLAN2_CONF 		"rtdev"
#define PROC_MEM_STATISTIC 	"/proc/meminfo"
#define PROC_IF_STATISTIC       "/proc/net/dev"

#if defined CONFIG_MT7628_APCLI_SUPPORT
#define RT2860_APCLI_SUPPORT
#endif

#define IPPORT_FILTER_CHAIN	"macipport_filter"
char l7name[8192];						// export it for internet.c qos

										// (The actual string is about 7200 bytes.)
void get_usage(char *aout)
{
	int i;

	DBG_MSG("Usage: ");
	for (i = 0; i < getNvramNum(); i++){
		DBG_MSG("\t%s %s nvram lan_ipaddr", aout, getNvramName(i));
	}
}

int ap_oid_query_info(unsigned long OidQueryCode, int socket_id, char *DeviceName, void *ptr, unsigned long PtrLength)
{
	struct iwreq wrq;

	strcpy(wrq.ifr_name, DeviceName);
	wrq.u.data.length = PtrLength;
	wrq.u.data.pointer = (caddr_t) ptr;
	wrq.u.data.flags = OidQueryCode;

	return (ioctl(socket_id, RT_PRIV_IOCTL, &wrq));
}
static void getAutoChannel(int nvram_id,char* value)
{
	char buf[128];
	char *ptr = NULL;
	int ret = 1; //default
	FILE *pp=NULL;

	if(nvram_id == 0)
		pp = popen("iwconfig ra0", "r");
	else
		pp = popen("iwconfig rai0", "r");

	if (pp)
	{
		while(fgets(buf, sizeof(buf), pp))
		{
			if (strlen(buf))
			{
				ptr = strstr(buf, "Channel");
				if (ptr)
				{
					ptr += strlen("Channel") + 1; //skip '='
					value = ptr;
					break;
				}
			}
		}

		pclose(pp);
	}
	while(*ptr != ' ')
		{
				*ptr ++;
		}

	*ptr = '\0';

	printf("%s",value);
	
}

/* 
 * NVRAM Query
 */
void WebNvramGet(char *argv[])
{
	char *key, tmp[1024], *p, *block_name;
	const char *value;
	int nvram_id;

	key = argv[3];

	if ((!strcmp(argv[1], "voip")) || (!strcmp(argv[1], "tr069")) || (!strcmp(argv[1], "config2")) ){
#if defined CONFIG_CONFIG_SHRINK
		block_name = FB_2860_BLOCK_NAME;
#else
		block_name = FB_CONFIG2_BLOCK_NAME;
#endif
	}
	else{
		block_name = argv[1];
	}
	if ((nvram_id = getNvramIndex(block_name)) == -1) {
		DBG_MSG("%s: Error: \"%s\" flash zone not existed", argv[0], argv[1]);
		get_usage(argv[0]);
		return;
	}

	nvram_init(nvram_id);
	if (!strcmp(argv[3], "Channel")) {
		/*
		value = nvram_bufget(nvram_id, "AutoChannelSelect");
		if (!strcmp(value, "0"))
			value = nvram_bufget(nvram_id, key);
		else*/
			getAutoChannel(nvram_id,value);
		return;
	} else
		value = nvram_bufget(nvram_id, key);

	if (!strcmp(argv[3], "HeMuChannel")) {
		value = nvram_bufget(nvram_id, "Channel");
	}
	
	memset(tmp, 0, 129);
	for (; *value != '\0'; value++) {
		switch(*value) {
		case '\\':
		case '\'':
			case '\"':
				sprintf(tmp, "%s\\", tmp);
		}
		sprintf(tmp, "%s%c", tmp, *value);
	}
	printf("%s", tmp);
	nvram_close(nvram_id);
}

void WebBuildGet(char *argv[])
{
	int build = 0;

#if defined CONFIG_RA_NAT_HW
	if (!strcmp(argv[3], "hnat"))
		build = 1;
#endif
#if defined CONFIG_RT2860V2_STA_DPB || defined CONFIG_RT2860V2_STA_ETH_CONVERT
	if (!strcmp(argv[3], "staDPB"))
		build = 1;
#endif
#if defined CONFIG_RTDEV_MII
	if (!strcmp(argv[3], "miiInic"))
		build = 1;
#endif
#if defined CONFIG_LAN_WAN_SUPPORT || defined CONFIG_MAC_TO_MAC_MODE || \
	(defined CONFIG_GE1_RGMII_AN && defined CONFIG_GE2_RGMII_AN) || \
	(defined CONFIG_GE1_RGMII_FORCE_1000 && defined CONFIG_GE2_INTERNAL_GPHY) || \
	(defined CONFIG_GE1_TRGMII_FORCE_1200 && defined CONFIG_GE2_INTERNAL_GPHY)
	if (!strcmp(argv[3], "gateway"))
		build = 1;
#endif
#if defined (CONFIG_RT2860V2_STA) || defined (CONFIG_RT2860V2_STA_MODULE)
	if (!strcmp(argv[3], "sta"))
		build = 1;
#endif
#if defined CONFIG_USER_3G
	if (!strcmp(argv[3], "3G"))
		build = 1;
#endif
#if defined CONFIG_HOSTNAME
	if (!strcmp(argv[3], "hostName"))
		build = 1;
#endif
#if defined CONFIG_USER_LLTD
	if (!strcmp(argv[3], "lltd"))
		build = 1;
#endif
#if defined CONFIG_USER_UPNP_IGD || defined CONFIG_USER_MINIUPNPD
	if (!strcmp(argv[3], "upnp"))
		build = 1;
#endif
#if defined CONFIG_USER_RADVD
	if (!strcmp(argv[3], "radvd"))
		build = 1;
#endif
#if defined CONFIG_USER_RPPPPOE_RELAY
	if (!strcmp(argv[3], "pppoeRelay"))
		build = 1;
#endif
#if defined CONFIG_USER_IGMP_PROXY
	if (!strcmp(argv[3], "igmpProxy"))
		build = 1;
#endif
#if defined CONFIG_USER_DNSMASQ
	if (!strcmp(argv[3], "dnsMasq"))
		build = 1;
#endif
#if defined CONFIG_USER_ZEBRA
	if (!strcmp(argv[3], "zebra"))
		build = 1;
#endif
#if defined CONFIG_NF_CONNTRACK_PPTP || defined CONFIG_NF_CONNTRACK_PPTP_MODULE || \
    defined CONFIG_IP_NF_PPTP        || defined CONFIG_IP_NF_PPTP_MODULE
	if (!strcmp(argv[3], "vpnpassthr"))
		build = 1;
#endif
#if defined CONFIG_RALINK_RT3052 || defined CONFIG_RALINK_RT3352 || defined CONFIG_RALINK_RT5350 || defined CONFIG_RALINK_RT6855 || defined CONFIG_RALNK_MT7620 || defined CONFIG_RALINK_MT7621
	if (!strcmp(argv[3], "QoSisPortBasedQoSSupport"))
		build = 1;
#endif

#if defined CONFIG_RALINKAPP_SWQOS || defined CONFIG_RALINKAPP_HWQOS
	if (!strcmp(argv[3], "qos"))
		build = 1;
#elif CONFIG_CUSTOM_QOS
	if (!strcmp(argv[3], "qos"))
		build = 2;
#endif

#if defined CONFIG_IPV6
	if (!strcmp(argv[3], "ipv6"))
		build = 1;
#endif
#if defined CONFIG_IPV6_SIT_6RD
	if (!strcmp(argv[3], "ipv6rd"))
		build = 1;
#endif
#if defined CONFIG_IPV6_TUNNEL
	if (!strcmp(argv[3], "ipv6dslite"))
		build = 1;
#endif
#if defined (RT2860_MBSS_SUPPORT)
	if (!strcmp(argv[3], "mBssid") && !strcmp(argv[1], WLAN1_CONF))
		build = 1;
#if defined (RT2860_NEW_MBSS_SUPPORT)
	if (!strcmp(argv[3], "mBssid16") && !strcmp(argv[1], WLAN1_CONF))
		build = 1;
#endif
#endif
#if defined (RTDEV_MBSS_SUPPORT)
	if (!strcmp(argv[3], "mBssid") && !strcmp(argv[1], WLAN2_CONF))
		build = 1;
#if defined (RTDEV_NEW_MBSS_SUPPORT)
	if (!strcmp(argv[3], "mBssid16") && !strcmp(argv[1], WLAN2_CONF))
		build = 1;
#endif
#endif
#if defined (RT2860_APCLI_SUPPORT)
	if (!strcmp(argv[3], "apClient"))
		build = 1;
#endif
#if defined (CONFIG_MT7628_APCLI_SUPPORT)
	if (!strcmp(argv[3], "apClient"))
		build = 1;
#endif
//#if defined (RT2860_11NDRAFT3_SUPPORT)
	if (!strcmp(argv[3], "11nDraft3") && !strcmp(argv[1], WLAN1_CONF))
		build = 1;
//#endif
//#if defined (RTDEV_11NDRAFT3_SUPPORT)
	if (!strcmp(argv[3], "11nDraft3") && !strcmp(argv[1], WLAN2_CONF))
		build = 1;
//#endif
#if defined (RT2860_DLS_SUPPORT)
	if (!strcmp(argv[3], "dls") && !strcmp(argv[1], WLAN1_CONF))
		build = 1;
#endif
#if defined (RTDEV_DLS_SUPPORT)
	if (!strcmp(argv[3], "dls") && !strcmp(argv[1], WLAN2_CONF))
		build = 1;
#endif
#if defined (RT2860_IGMPSNOOP_SUPPORT)
	if (!strcmp(argv[3], "igmpSnooping") && !strcmp(argv[1], WLAN1_CONF))
		build = 1;
#endif
#if defined (RTDEV_IGMPSNOOP_SUPPORT)
	if (!strcmp(argv[3], "igmpSnooping") && !strcmp(argv[1], WLAN2_CONF))
		build = 1;
#endif
#if defined (CONFIG_MT7628_IGMP_SNOOP_SUPPORT)
	if (!strcmp(argv[3], "igmpSnooping") && !strcmp(argv[1], WLAN2_CONF))
		build = 1;
#endif
#if defined (RT2860_WDS_SUPPORT)
	if (!strcmp(argv[3], "wds") && !strcmp(argv[1], WLAN1_CONF))
		build = 1;
#endif
#if defined (RTDEV_WDS_SUPPORT)
	if (!strcmp(argv[3], "wds") && !strcmp(argv[1], WLAN2_CONF))
		build = 1;
#endif
#if defined (RT2860_WSC_SUPPORT)
	if (!strcmp(argv[3], "wsc") && !strcmp(argv[1], WLAN1_CONF))
		build = 1;
#endif
#if defined (RTDEV_WSC_SUPPORT)
	if (!strcmp(argv[3], "wsc") && !strcmp(argv[1], WLAN2_CONF))
		build = 1;
#endif
#if defined (RT2860_ABAND_SUPPORT)
	if (!strcmp(argv[3], "aBand") && !strcmp(argv[1], WLAN1_CONF))
		build = 1;
#endif
#if defined (RTDEV_ABAND_SUPPORT)
	if (!strcmp(argv[3], "aBand") && !strcmp(argv[1], WLAN2_CONF))
		build = 1;
#endif
#if defined (RT2860_GBAND_SUPPORT)
	if (!strcmp(argv[3], "gBand") && !strcmp(argv[1], WLAN1_CONF))
		build = 1;
#endif
#if defined (RTDEV_GBAND_SUPPORT)
	if (!strcmp(argv[3], "gBand") && !strcmp(argv[1], WLAN2_CONF))
		build = 1;
#endif
#if defined (RT2860_AC_SUPPORT)
	if (!strcmp(argv[3], "vht") && !strcmp(argv[1], WLAN1_CONF))
		build = 1;
#endif
#if defined (RTDEV_AC_SUPPORT)
	if (!strcmp(argv[3], "vht") && !strcmp(argv[1], WLAN2_CONF))
		build = 1;
#endif
#if defined (RT2860_WAPI_SUPPORT)
	if (!strcmp(argv[3], "wapi") && !strcmp(argv[1], WLAN1_CONF))
		build = 1;
#endif
#if defined (RTDEV_WAPI_SUPPORT)
	if (!strcmp(argv[3], "wapi") && !strcmp(argv[1], WLAN2_CONF))
		build = 1;
#endif
#if defined (RT2860_DFS_SUPPORT)
	if (!strcmp(argv[3], "dfs") && !strcmp(argv[1], WLAN1_CONF))
		build = 1;
#endif
#if defined (RTDEV_DFS_SUPPORT)
	if (!strcmp(argv[3], "dfs") && !strcmp(argv[1], WLAN2_CONF))
		build = 1;
#endif
#if defined (RT2860_CARRIER_SUPPORT)
	if (!strcmp(argv[3], "carrier") && !strcmp(argv[1], WLAN1_CONF))
		build = 1;
#endif
#if defined (RTDEV_CARRIER_SUPPORT)
	if (!strcmp(argv[3], "carrier") && !strcmp(argv[1], WLAN2_CONF))
		build = 1;
#endif
#if defined (CONFIG_EXT_CHANNEL_LIST)
	if (!strcmp(argv[3], "extchlist") && !strcmp(argv[1], WLAN1_CONF))
		build = 1;
#endif
#if defined (RT2860_HS_SUPPORT)
	if (!strcmp(argv[3], "hotspot") && !strcmp(argv[1], WLAN1_CONF))
		build = 1;
#endif
#if defined (RTDEV_HS_SUPPORT)
	if (!strcmp(argv[3], "hotspot") && !strcmp(argv[1], WLAN2_CONF))
		build = 1;
#endif
#if defined CONFIG_RA_CLASSIFIER_MODULE && \
    defined CONFIG_RT2860V2_AP_VIDEO_TURBINE && \
    defined (RT2860_TXBF_SUPPORT)
	if (!strcmp(argv[3], "rvt") && !strcmp(argv[1], WLAN1_CONF))
		build = 1;
#endif
#if defined (RT2860_TXBF_SUPPORT)
	if (!strcmp(argv[3], "txbf") && !strcmp(argv[1], WLAN1_CONF))
		build = 1;
#endif
#if defined (RTDEV_TXBF_SUPPORT)
	if (!strcmp(argv[3], "txbf") && !strcmp(argv[1], ))
		build = 1;
#endif
#if defined CONFIG_RT2860V2_AUTO_PROVISION
	if (!strcmp(argv[3], "autoProvision"))
		build = 1;
#endif
#if defined (RT2860_MESH_SUPPORT)
	if (!strcmp(argv[3], "mesh") && !strcmp(argv[1], WLAN1_CONF))
		build = 1;
#endif
#if defined (RTDEV_MESH_SUPPORT)
	if (!strcmp(argv[3], "mesh") && !strcmp(argv[1], WLAN2_CONF))
		build = 1;
#endif
#if defined CONFIG_IP_NF_FILTER
	if (!strcmp(argv[3], "pktFilter"))
		build = 1;
#endif
#if defined CONFIG_USER_STORAGE && (defined CONFIG_USB || defined CONFIG_MMC)
	if (!strcmp(argv[3], "nas"))
		build = 1;
#endif
#if defined CONFIG_USER_PROFTPD
	if (!strcmp(argv[3], "ftp"))
		build = 1;
#endif
#if defined CONFIG_USER_SAMBA
	if (!strcmp(argv[3], "samba"))
		build = 1;
#endif
#if defined CONFIG_USER_GREENAP && defined CONFIG_CROND && defined CONFIG_CRONTAB
	if (!strcmp(argv[3], "greenAP"))
		build = 1;
#endif
#if defined CONFIG_DATE
	if (!strcmp(argv[3], "cmd_date"))
		build = 1;
#endif
#if defined CONFIG_USER_INADYN
	if (!strcmp(argv[3], "ddns"))
		build = 1;
#endif
#if (defined CONFIG_RALINK_WATCHDOG || defined CONFIG_RALINK_WATCHDOG_MODULE) && defined CONFIG_USER_WATCHDOG
	if (!strcmp(argv[3], "wdg"))
		build = 1;
#endif
#if defined CONFIG_LOGREAD && defined CONFIG_KLOGD
	if (!strcmp(argv[3], "ethtool"))
		build = 1;
#endif

#if defined CONFIG_RALINKAPP_SUPERDMZ
	if (!strcmp(argv[3], "superDMZ"))
		build = 1;
#endif
#if defined CONFIG_USB && defined CONFIG_USER_UVC_STREAM
	if (!strcmp(argv[3], "webcam"))
		build = 1;
#endif
#if defined CONFIG_USB && defined CONFIG_USER_P910ND
	if (!strcmp(argv[3], "printersrv"))
		build = 1;
#endif
#if defined CONFIG_USB && defined CONFIG_USER_MTDAAPD
	if (!strcmp(argv[3], "itunes"))
		build = 1;
#endif
#if defined CONFIG_USER_STORAGE && defined CONFIG_USER_MINIDLNA
	if (!strcmp(argv[3], "media"))
		build = 1;
#endif
#if defined CONFIG_MTK_VOIP
  if (!strcmp(argv[3], "voip"))
		build = 1;
#endif

#if defined CONFIG_RALINKAPP_AUTO_WAN
  if (!strcmp(argv[3], "auto_wan"))
		build = 1;
#endif


#if defined (CONFIG_RALINK_MT7628)
  if (!strcmp(argv[3], "one_button_through_wall"))
		build = 1;
#endif
	printf("%d", build);
}

/*
 * WiFi Query
 */
void GetWifiVersion(char *argv[])
{
	unsigned char DriverVersionInfo[16];
	int s;
	char wif[5];

	memset(DriverVersionInfo, 0, 16);
	if (!strcmp(argv[1], WLAN2_CONF))
		strcpy(wif, "rai0");
	else
		strcpy(wif, "ra0");
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (ap_oid_query_info(RT_OID_VERSION_INFO, s, wif, &DriverVersionInfo, sizeof(DriverVersionInfo)) >= 0)
		printf("%s", DriverVersionInfo);
	else
		printf("&nbsp;");
	close(s);
}

/*
 *  * description: write 802.11a channels in <select> tag
 */
void GetAChannels(char *argv[])
{
	int  nvram_id, idx = 0, channel;
	const char *value, *channel_s;

	if ((nvram_id = getNvramIndex(argv[1])) == -1) {
		DBG_MSG("%s: Error: \"%s\" flash zone not existed", argv[0], argv[1]);
		get_usage(argv[0]);
		return;
	}
	nvram_init(nvram_id);
	value = nvram_bufget(nvram_id,"CountryRegionABand");
	channel_s = nvram_bufget(nvram_id, "Channel");

	channel = (channel_s == NULL)? 0 : strtoul(channel_s, NULL, 10);
#if ! defined CONFIG_EXT_CHANNEL_LIST
	if ((value == NULL) || (strcmp(value, "") == 0) ||
			(strcmp(value, "7") == 0)) {
		/* 36~64 */
		for (idx = 0; idx < 8; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 100~140 */
		for (idx = 16; idx < 27; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 149~173 */
		for (idx = 28; idx < 35; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=",
					36+4*idx+1, (36+4*idx+1 == channel)? "selected" : "",
					5180+20*idx+5, "MHz (Channel ", 36+4*idx+1, ")</option>");
	} else if (strcmp(value, "0") == 0) {
		/* 36~64 */
		for (idx = 0; idx < 8; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 149~165 */
		for (idx = 28; idx < 33; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=",
					36+4*idx+1, (36+4*idx+1 == channel)? "selected" : "",
					5180+20*idx+5, "MHz (Channel ", 36+4*idx+1, ")</option>");
	} else if (strcmp(value, "1") == 0) {
		/* 36~64 */
		for (idx = 0; idx < 8; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 100~140 */
		for (idx = 16; idx < 27; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
	} else if (strcmp(value, "2") == 0) {
		/* 36~64 */
		for (idx = 0; idx < 8; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
	} else if (strcmp(value, "3") == 0) {
		/* 52~64 */
		for (idx = 4; idx < 8; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 149~161 */
		for (idx = 28; idx < 32; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=",
					36+4*idx+1, (36+4*idx+1 == channel)? "selected" : "",
					5180+20*idx+5, "MHz (Channel ", 36+4*idx+1, ")</option>");
	} else if (strcmp(value, "4") == 0) {
		/* 149~165 */
		for (idx = 28; idx < 33; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=",
					36+4*idx+1, (36+4*idx+1 == channel)? "selected" : "",
					5180+20*idx+5, "MHz (Channel ", 36+4*idx+1, ")</option>");
	} else if (strcmp(value, "5") == 0) {
		/* 149~161 */
		for (idx = 28; idx < 32; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=",
					36+4*idx+1, (36+4*idx+1 == channel)? "selected" : "",
					5180+20*idx+5, "MHz (Channel ", 36+4*idx+1, ")</option>");
	} else if (strcmp(value, "6") == 0) {
		/* 36~48 */
		for (idx = 0; idx < 4; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
	} else if (strcmp(value, "8") == 0) {
		/* 52~64 */
		for (idx = 4; idx < 8; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
	} else if (strcmp(value, "9") == 0) {
		/* 36~64 */
		for (idx = 0; idx < 8; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 100~116 */
		for (idx = 16; idx < 21; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 132~140 */
		for (idx = 24; idx < 27; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 149~165 */
		for (idx = 28; idx < 33; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=",
					36+4*idx+1, (36+4*idx+1 == channel)? "selected" : "",
					5180+20*idx+5, "MHz (Channel ", 36+4*idx+1, ")</option>");
	} else if (strcmp(value, "10") == 0) {
		/* 36~48 */
		for (idx = 0; idx < 4; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 149~165 */
		for (idx = 28; idx < 33; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=",
					36+4*idx+1, (36+4*idx+1 == channel)? "selected" : "",
					5180+20*idx+5, "MHz (Channel ", 36+4*idx+1, ")</option>");
	} else if (strcmp(value, "11") == 0) {
		/* 36~64 */
		for (idx = 0; idx < 8; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 100~120 */
		for (idx = 16; idx < 22; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 149~161 */
		for (idx = 28; idx < 32; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=",
					36+4*idx+1, (36+4*idx+1 == channel)? "selected" : "",
					5180+20*idx+5, "MHz (Channel ", 36+4*idx+1, ")</option>");
	} else if (strcmp(value, "12") == 0) {
		/* 36~64 */
		for (idx = 0; idx < 8; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 100~140 */
		for (idx = 16; idx < 27; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
	} else if (strcmp(value, "13") == 0) {
		/* 52~64 */
		for (idx = 4; idx < 8; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 100~140 */
		for (idx = 16; idx < 27; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 149~161 */
		for (idx = 28; idx < 32; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=",
					36+4*idx+1, (36+4*idx+1 == channel)? "selected" : "",
					5180+20*idx+5, "MHz (Channel ", 36+4*idx+1, ")</option>");
	} else if (strcmp(value, "14") == 0) {
		/* 36~64 */
		for (idx = 0; idx < 8; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 100~116 */
		for (idx = 16; idx < 21; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 136~140 */
		for (idx = 25; idx < 27; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 149~165 */
		for (idx = 28; idx < 33; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=",
					36+4*idx+1, (36+4*idx+1 == channel)? "selected" : "",
					5180+20*idx+5, "MHz (Channel ", 36+4*idx+1, ")</option>");
	} else if (strcmp(value, "15") == 0) {
		/* 149~173 */
		for (idx = 28; idx < 35; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=",
					36+4*idx+1, (36+4*idx+1 == channel)? "selected" : "",
					5180+20*idx+5, "MHz (Channel ", 36+4*idx+1, ")</option>");
	} else if (strcmp(value, "16") == 0) {
		/* 52~64 */
		for (idx = 4; idx < 8; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 149~165 */
		for (idx = 28; idx < 33; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=",
					36+4*idx+1, (36+4*idx+1 == channel)? "selected" : "",
					5180+20*idx+5, "MHz (Channel ", 36+4*idx+1, ")</option>");
	} else if (strcmp(value, "17") == 0) {
		/* 36~48 */
		for (idx = 0; idx < 4; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 149~161 */
		for (idx = 28; idx < 32; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=",
					36+4*idx+1, (36+4*idx+1 == channel)? "selected" : "",
					5180+20*idx+5, "MHz (Channel ", 36+4*idx+1, ")</option>");
	} else if (strcmp(value, "18") == 0) {
		/* 36~64 */
		for (idx = 0; idx < 8; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 100~116 */
		for (idx = 16; idx < 21; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 132~140 */
		for (idx = 24; idx < 27; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
	} else if (strcmp(value, "19") == 0) {
		/* 56~64 */
		for (idx = 5; idx < 8; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 100~140 */
		for (idx = 16; idx < 27; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 149~161 */
		for (idx = 28; idx < 32; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=",
					36+4*idx+1, (36+4*idx+1 == channel)? "selected" : "",
					5180+20*idx+5, "MHz (Channel ", 36+4*idx+1, ")</option>");
	} else if (strcmp(value, "20") == 0) {
		/* 36~64 */
		for (idx = 0; idx < 8; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 100~124 */
		for (idx = 16; idx < 23; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 149~161 */
		for (idx = 28; idx < 32; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=",
					36+4*idx+1, (36+4*idx+1 == channel)? "selected" : "",
					5180+20*idx+5, "MHz (Channel ", 36+4*idx+1, ")</option>");
	} else if (strcmp(value, "21") == 0) {
		/* 36~64 */
		for (idx = 0; idx < 8; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 100~140 */
		for (idx = 16; idx < 27; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
		/* 149~161 */
		for (idx = 28; idx < 32; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=",
					36+4*idx+1, (36+4*idx+1 == channel)? "selected" : "",
					5180+20*idx+5, "MHz (Channel ", 36+4*idx+1, ")</option>");
	}
#else
	/* 36~64 */
	for (idx = 0; idx < 8; idx++)
		printf("%s%d %s>%d%s%d%s", "<option value=", 36+4*idx,
				(36+4*idx == channel)? "selected" : "", 5180+20*idx,
				"MHz (Channel ", 36+4*idx, ")</option>");
	/* 100~140 */
	for (idx = 16; idx < 27; idx++)
		printf("%s%d %s>%d%s%d%s", "<option value=", 36+4*idx,
				(36+4*idx == channel)? "selected" : "", 5180+20*idx,
				"MHz (Channel ", 36+4*idx, ")</option>");
	/* 149~173 */
	for (idx = 28; idx < 35; idx++)
		printf("%s%d %s>%d%s%d%s", "<option value=",
				36+4*idx+1, (36+4*idx+1 == channel)? "selected" : "",
				5180+20*idx+5, "MHz (Channel ", 36+4*idx+1, ")</option>");
#endif
	nvram_close(nvram_id);
}

void GetGChannels(char *argv[])
{
	int  nvram_id, idx = 0, channel;
	const char *value, *channel_s;

	if ((nvram_id = getNvramIndex(argv[1])) == -1) {
		DBG_MSG("%s: Error: \"%s\" flash zone not existed", argv[0], argv[1]);
		get_usage(argv[0]);
		return;
	}
	nvram_init(nvram_id);
	value = nvram_bufget(nvram_id,"CountryRegion");
	channel_s = nvram_bufget(nvram_id, "Channel");

	channel = (channel_s == NULL)? 0 : strtoul(channel_s, NULL, 10);
#if ! defined CONFIG_EXT_CHANNEL_LIST
	if ((value == NULL) || (strcmp(value, "") == 0) ||
			(strcmp(value, "5") == 0)) {
		/* ch1 ~ ch14 */
		for (idx = 0; idx < 14; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", idx+1,
					(idx+1 == channel)? "selected" : "", 2412+5*idx,
					"MHz (Channel ", idx+1, ")</option>");
	} else if (strcmp(value, "0") == 0) {
		/* ch1 ~ ch11 */
		for (idx = 0; idx < 11; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", idx+1,
					(idx+1 == channel)? "selected" : "", 2412+5*idx,
					"MHz (Channel ", idx+1, ")</option>");
	} else if (strcmp(value, "1") == 0) {
		/* ch1 ~ ch13 */
		for (idx = 0; idx < 13; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", idx+1,
					(idx+1 == channel)? "selected" : "", 2412+5*idx,
					"MHz (Channel ", idx+1, ")</option>");
	} else if (strcmp(value, "2") == 0) {
		/* ch10 ~ ch11 */
		for (idx = 9; idx < 11; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", idx+1,
					(idx+1 == channel)? "selected" : "", 2412+5*idx,
					"MHz (Channel ", idx+1, ")</option>");
	} else if (strcmp(value, "3") == 0) {
		/* ch10 ~ ch13 */
		for (idx = 9; idx < 13; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", idx+1,
					(idx+1 == channel)? "selected" : "", 2412+5*idx,
					"MHz (Channel ", idx+1, ")</option>");
	} else if (strcmp(value, "4") == 0) {
		/* ch14 */
		printf("<option value=14 %s>2484MHz (Channel 14)</option>\n", 
				(14 == channel)? "selected" : "");
	} else if (strcmp(value, "6") == 0) {
		/* ch3 ~ ch9 */
		for (idx = 2; idx < 9; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", idx+1,
					(idx+1 == channel)? "selected" : "", 2412+5*idx,
					"MHz (Channel ", idx+1, ")</option>");
	} else if (strcmp(value, "7") == 0) {
		/* ch5 ~ ch13 */
		for (idx = 4; idx < 13; idx++)
			printf("%s%d %s>%d%s%d%s", 
					"<option value=", idx+1,
					(idx+1 == channel)? "selected" : "", 2412+5*idx,
					"MHz (Channel ", idx+1, ")</option>");
	} else if (strcmp(value, "31") == 0) {
		/* ch1 ~ ch11 */
		for (idx = 0; idx < 11; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", idx+1,
					(idx+1 == channel)? "selected" : "", 2412+5*idx,
					"MHz (Channel ", idx+1, ")</option>");
#if 0
		/* ch12 ~ ch14 */
		for (idx = 11; idx < 14; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", idx+1,
					(idx+1 == channel)? "selected" : "", 2412+5*idx,
					"MHz (Channel ", idx+1, ")</option>");
#endif
	} else if (strcmp(value, "32") == 0) {
		/* ch1 ~ ch11 */
		for (idx = 0; idx < 11; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", idx+1,
					(idx+1 == channel)? "selected" : "", 2412+5*idx,
					"MHz (Channel ", idx+1, ")</option>");
#if 0
		/* ch12 ~ ch13 */
		for (idx = 11; idx < 13; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", idx+1,
					(idx+1 == channel)? "selected" : "", 2412+5*idx,
					"MHz (Channel ", idx+1, ")</option>");
#endif
	} else if (strcmp(value, "33") == 0) {
		/* ch1 ~ ch14 */
		for (idx = 0; idx < 14; idx++)
			printf("%s%d %s>%d%s%d%s", "<option value=", idx+1,
					(idx+1 == channel)? "selected" : "", 2412+5*idx,
					"MHz (Channel ", idx+1, ")</option>");
	}
#else
	/* ch1 ~ ch14 */
	for (idx = 0; idx < 14; idx++)
		printf("%s%d %s>%d%s%d%s", "<option value=", idx+1,
				(idx+1 == channel)? "selected" : "", 2412+5*idx,
				"MHz (Channel ", idx+1, ")</option>");
#endif
	nvram_close(nvram_id);
}

void StaInfo(char *argv[])
{
	int i, s;
	struct iwreq iwr;
	RT_802_11_MAC_TABLE table = {0};
#if defined (RT2860_TXBF_SUPPORT) || defined (RTDEV_TXBF_SUPPORT)
	char tmpBuff[32];
	char *phyMode[4] = {"CCK", "OFDM", "MM", "GF"};
#endif
	char ifname[5];

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (!strcmp(argv[1], WLAN2_CONF))
		strncpy(iwr.ifr_name, "rai0", IFNAMSIZ);
	else
		strncpy(iwr.ifr_name, "ra0", IFNAMSIZ);
	iwr.u.data.pointer = (caddr_t) &table;

	if (s < 0) {
		DBG_MSG("open socket fail!");
		return;
	}

	if (ioctl(s, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &iwr) < 0) {
		DBG_MSG("ioctl -> RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT fail!");
		close(s);
		return;
	}

	close(s);
#if defined (RT2860_TXBF_SUPPORT)
	if (!strcmp(argv[1], WLAN1_CONF))
		goto advance;
#endif
#if defined (RT2860_TXBF_SUPPORT)
	if (!strcmp(argv[1], WLAN2_CONF))
		goto advance;
#endif
#if ! defined (RT2860_TXBF_SUPPORT) || ! defined (RTDEV_TXBF_SUPPORT)
	for (i = 0; i < table.Num; i++) {
		printf("<tr><td>%02X:%02X:%02X:%02X:%02X:%02X</td>",
				table.Entry[i].Addr[0], table.Entry[i].Addr[1],
				table.Entry[i].Addr[2], table.Entry[i].Addr[3],
				table.Entry[i].Addr[4], table.Entry[i].Addr[5]);
		printf("<td>%d</td><td>%d</td><td>%d</td>",
				table.Entry[i].Aid, table.Entry[i].Psm, table.Entry[i].MimoPs);
		printf("<td>%d</td><td>%s</td><td>%d</td><td>%d</td></tr>",
				table.Entry[i].TxRate.field.MCS,
				(table.Entry[i].TxRate.field.BW == 0)? "20M":"40M",
				table.Entry[i].TxRate.field.ShortGI, table.Entry[i].TxRate.field.STBC);
	}
	return;
#endif
#if defined (RT2860_TXBF_SUPPORT) || defined (RTDEV_TXBF_SUPPORT)
advance:
	for (i = 0; i < table.Num; i++) {
		RT_802_11_MAC_ENTRY *pe = &(table.Entry[i]);
		unsigned int lastRxRate = pe->LastRxRate;
		unsigned int mcs = pe->LastRxRate & 0x7F;
		int hr, min, sec;

		hr = pe->ConnectedTime/3600;
		min = (pe->ConnectedTime % 3600)/60;
		sec = pe->ConnectedTime - hr*3600 - min*60;

		// MAC Address
		printf("<tr><td>%02X:%02X:%02X:<br>%02X:%02X:%02X</td>",
				pe->Addr[0], pe->Addr[1], pe->Addr[2], pe->Addr[3],
				pe->Addr[4], pe->Addr[5]);

		// AID, Power Save mode, MIMO Power Save
		printf("<td align=\"center\">%d</td><td align=\"center\">%d</td><td align=\"center\">%d</td>",
				pe->Aid, pe->Psm, pe->MimoPs);

		// TX Rate
		printf("<td>MCS %d<br>%2dM, %cGI<br>%s%s</td>",
				pe->TxRate.field.MCS,
				pe->TxRate.field.BW? 40: 20,
				pe->TxRate.field.ShortGI? 'S': 'L',
				phyMode[pe->TxRate.field.MODE],
				pe->TxRate.field.STBC? ", STBC": " ");

		// TxBF configuration
		printf("<td align=\"center\">%c %c</td>",
				pe->TxRate.field.iTxBF? 'I': '-',
				pe->TxRate.field.eTxBF? 'E': '-');

		// RSSI
		printf("<td align=\"center\">%d<br>%d<br>%d</td>",
				(int)(pe->AvgRssi0), (int)(pe->AvgRssi1), (int)(pe->AvgRssi2));

		// Per Stream SNR
		snprintf(tmpBuff, sizeof(tmpBuff), "%0.1f", pe->StreamSnr[0]*0.25);
		printf("<td>%s<br>", tmpBuff);
		snprintf(tmpBuff, sizeof(tmpBuff), "%0.1f", pe->StreamSnr[1]*0.25); //mcs>7? pe->StreamSnr[1]*0.25: 0.0);
		printf("%s<br>", tmpBuff);
		snprintf(tmpBuff, sizeof(tmpBuff), "%0.1f", pe->StreamSnr[2]*0.25); //mcs>15? pe->StreamSnr[2]*0.25: 0.0);
		printf("%s</td>", tmpBuff);

		// Sounding Response SNR
		if (pe->TxRate.field.eTxBF) {
			snprintf(tmpBuff, sizeof(tmpBuff), "%0.1f", pe->SoundingRespSnr[0]*0.25);
			printf("<td>%s<br>", tmpBuff);
			snprintf(tmpBuff, sizeof(tmpBuff), "%0.1f", pe->SoundingRespSnr[1]*0.25);
			printf("%s<br>", tmpBuff);
			snprintf(tmpBuff, sizeof(tmpBuff), "%0.1f", pe->SoundingRespSnr[2]*0.25);
			printf("%s</td>", tmpBuff);
		}
		else {
			printf("<td align=\"center\">-<br>-<br>-</td>");
		}

		// Last RX Rate
		printf("<td>MCS %d<br>%2dM, %cGI<br>%s%s</td>",
				mcs,  ((lastRxRate>>7) & 0x1)? 40: 20,
				((lastRxRate>>8) & 0x1)? 'S': 'L',
				phyMode[(lastRxRate>>14) & 0x3],
				((lastRxRate>>9) & 0x3)? ", STBC": " ");

		// Connect time
		//printf("<td align=\"center\">%02d:%02d:%02d<br>PER=%d%%</td>", hr, min, sec, pe->TxPER);
		printf("<td align=\"center\">%02d:%02d:%02d</td>", hr, min, sec);
		printf("</tr>");
	}
#endif
}

void ApStats(char *argv[])
{
	NDIS_802_11_STATISTICS stat;
	int s, ret;
	float txCount;
#ifdef ENHANCED_AP_STATUS_INFO
	char  tmpStatistics[256];
#endif

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (!strcmp(argv[1], WLAN2_CONF))
		ret = ap_oid_query_info(OID_802_11_STATISTICS, s, "rai0", &stat, sizeof(stat));
	else
		ret = ap_oid_query_info(OID_802_11_STATISTICS, s, "ra0", &stat, sizeof(stat));
	close(s);
	if (ret >= 0) {
		txCount = stat.TransmittedFragmentCount.QuadPart;

		if (!strncmp(argv[4], "TxSucc", 7)) {
			printf("%ld", stat.TransmittedFragmentCount.QuadPart);
#ifdef ENHANCED_AP_STATUS_INFO
		} else if (!strncmp(argv[4], "TxRetry", 8)) {
			sprintf(tmpStatistics, "%lld,  PER=%0.1f%%", stat.RetryCount.QuadPart,
					txCount==0? 0.0: 100.0f*(stat.RetryCount.QuadPart+stat.FailedCount.QuadPart)/(stat.RetryCount.QuadPart+stat.FailedCount.QuadPart+txCount));
			printf("%s", tmpStatistics);
		} else if (!strncmp(argv[4], "TxFail", 7)) {
			sprintf(tmpStatistics, "%lld,  PLR=%0.1e", stat.FailedCount.QuadPart,
					txCount==0? 0.0: (float)stat.FailedCount.QuadPart/(stat.FailedCount.QuadPart+txCount));
			printf("%s", tmpStatistics);
#else
		} else if (!strncmp(argv[4], "TxRetry", 8)) {
			printf("%ld", stat.RetryCount.QuadPart);
		} else if (!strncmp(argv[4], "TxFail", 7)) {
			printf("%ld", stat.FailedCount.QuadPart);
#endif
		} else if (!strncmp(argv[4], "RTSSucc", 8)) {
			printf("%ld", stat.RTSSuccessCount.QuadPart);
		} else if (!strncmp(argv[4], "RTSFail", 8)) {
			printf("%ld", stat.RTSFailureCount.QuadPart);
		} else if (!strncmp(argv[4], "RxSucc", 7)) {
			printf("%ld", stat.ReceivedFragmentCount.QuadPart);
		} else if (!strncmp(argv[4], "FCSError", 9)) {
#ifdef ENHANCED_AP_STATUS_INFO
			sprintf(tmpStatistics, "%lld, PER=%0.1f%%", stat.FCSErrorCount.QuadPart,
					stat.ReceivedFragmentCount.QuadPart==0?
					0.0: 100.0f*stat.FCSErrorCount.QuadPart/(stat.FCSErrorCount.QuadPart+stat.ReceivedFragmentCount.QuadPart));
			printf("%s", tmpStatistics);
#else
			printf("%ld", stat.FCSErrorCount.QuadPart);
#endif
		} else {
			printf("type not supported");
		}
	} else {
		printf("ioctl %d", ret);
	}
}

void ApSNR(char *argv[])
{
	int s, n, ret;
	unsigned long SNR;
	char wif[5];

	if (!strcmp(argv[1], WLAN2_CONF))
		strncpy(wif, "rai0", IFNAMSIZ);
	else
		strncpy(wif, "ra0", IFNAMSIZ);
	n = strtoul(argv[4], NULL, 10);
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (n == 0)
		ret = ap_oid_query_info(RT_OID_802_11_SNR_0, s, wif, &SNR, sizeof(SNR));
	else if (n == 1)
		ret = ap_oid_query_info(RT_OID_802_11_SNR_1, s, wif, &SNR, sizeof(SNR));
	else if (n == 2)
		ret = ap_oid_query_info(RT_OID_802_11_SNR_2, s, wif, &SNR, sizeof(SNR));
	else
		ret = -1;
	close(s);

	if (ret < 0) {
		printf("n/a");
	} else {
		printf("%ld", SNR);
	}

}

#if defined (RT2860_TXBF_SUPPORT) || defined (RTDEV_TXBF_SUPPORT)
void ApTxBfStats(char *argv[])
{
	int s, ret, i;
	RT_802_11_TXBF_TABLE table = {0};
	RT_COUNTER_TXBF *pCnt;
	int hdr = 0;
	char  tmpStatistics[256];
	unsigned long totalNBF, totalEBF, totalIBF, totalTx, totalRetry, totalSuccess;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (!strcmp(argv[1], WLAN2_CONF))
		ret = ap_oid_query_info(RT_OID_802_11_QUERY_TXBF_TABLE, s, "rai0", &table, sizeof(table));
	else
		ret = ap_oid_query_info(RT_OID_802_11_QUERY_TXBF_TABLE, s, "ra0", &table, sizeof(table));
	close(s);
	if (ret < 0) {
		DBG_MSG("ioctl error %d", ret);
		return;
	}

	for (i = 0; i < table.Num; i++) {
		pCnt = &(table.Entry[i]);
		totalNBF = pCnt->TxSuccessCount + pCnt->TxFailCount;
		totalEBF = pCnt->ETxSuccessCount + pCnt->ETxFailCount;
		totalIBF = pCnt->ITxSuccessCount + pCnt->ITxFailCount;
		totalTx = totalNBF + totalEBF + totalIBF;
		totalRetry = pCnt->TxRetryCount + pCnt->ETxRetryCount + pCnt->ITxRetryCount;
		totalSuccess = pCnt->TxSuccessCount + pCnt->ETxSuccessCount + pCnt->ITxSuccessCount;

		if (totalTx == 0)
			continue;
		if (hdr == 0) {
			printf("<tr><td class=\"title\" colspan=\"2\">Detailed TX Statistics (Retry count is approximate)</td></tr>");
			hdr = 1;
		}

		printf("<tr><td class=\"head2\">%d</td><td><kbd>", i);
		strcpy(tmpStatistics, "              Success    Retry/PER    Fail/PLR");
		convert_string_display(tmpStatistics);
		printf("%s<br>", tmpStatistics);
		if (totalNBF!=0) {
			sprintf(tmpStatistics, "NonBF (%3lu%%): %7lu  %7lu (%2lu%%) %5lu (%1lu%%)",
					100*totalNBF/totalTx, pCnt->TxSuccessCount,
					pCnt->TxRetryCount, 100*pCnt->TxRetryCount/(pCnt->TxSuccessCount+pCnt->TxRetryCount),
					pCnt->TxFailCount, 100*pCnt->TxFailCount/totalNBF);
			convert_string_display(tmpStatistics);
			printf("%s<<br>", tmpStatistics);
		}
		if (totalEBF!=0) {
			sprintf(tmpStatistics, "ETxBF (%3lu%%): %7lu  %7lu (%2lu%%) %5lu (%1lu%%)",
					100*totalEBF/totalTx, pCnt->ETxSuccessCount,
					pCnt->ETxRetryCount, 100*pCnt->ETxRetryCount/(pCnt->ETxSuccessCount+pCnt->ETxRetryCount),
					pCnt->ETxFailCount, 100*pCnt->ETxFailCount/totalEBF);
			convert_string_display(tmpStatistics);
			printf("%s<br>", tmpStatistics);
		}

		if (totalIBF!=0) {
			sprintf(tmpStatistics, "ITxBF (%3lu%%): %7lu  %7lu (%2lu%%) %5lu (%1lu%%)",
					100*totalIBF/totalTx, pCnt->ITxSuccessCount,
					pCnt->ITxRetryCount, 100*pCnt->ITxRetryCount/(pCnt->ITxSuccessCount+pCnt->ITxRetryCount),
					pCnt->ITxFailCount, 100*pCnt->ITxFailCount/totalIBF);
			convert_string_display(tmpStatistics);
			printf("%s<br>", tmpStatistics);
		}

		sprintf(tmpStatistics, "Total         %7lu  %7lu (%2lu%%) %5lu (%1lu%%)",
				totalSuccess, totalRetry, 100*totalRetry/(totalSuccess + totalRetry),
				pCnt->TxFailCount+pCnt->ETxFailCount+pCnt->ITxFailCount,
				100*(pCnt->TxFailCount+pCnt->ETxFailCount+pCnt->ITxFailCount)/totalTx);
		convert_string_display(tmpStatistics);
		printf("%s", tmpStatistics);
		printf("</kbd></td></tr>");
	}
}
#endif

struct apcliSsidInfo
{
	char apcliChannel[4];
	char apcliSsid[186];
	char apcliSecurity[23];
	char apcliSignal[9];
	int  flag;//0: 2.4G; 1: 5g
}apcliSsidInfo;

#define MAX_SSID_NUM 100
#if defined (RT2860_APCLI_SUPPORT)
void ApcliScan(char *argv[])
{
	FILE *pp;
	char cmd[CMDLEN], *ptr, wif[5],count_s[8]={0};
	char channel[4], ssid[186], bssid[20], security[23];
	char signal[9], mode[7], ext_ch[7], net_type[3];
	int i, space_start,count=0;

	if (!strcmp(argv[1], WLAN2_CONF))
		strcpy(wif, "rai0");
	else 
		{
		strcpy(wif, "ra0");
		
		count =atoi(nvram_bufget(RTDEV_NVRAM, "apcli5count"));
		}
	do_system("iwpriv %s set SiteSurvey=1", wif);
	sleep(5);
	sprintf(cmd, "iwpriv %s get_site_survey", wif);
	if(!(pp = popen(cmd, "r"))) {
		DBG_MSG("execute get_site_survey fail!");
		return;
	}

	memset(cmd, 0, sizeof(cmd));
	fgets(cmd, sizeof(cmd), pp);
	fgets(cmd, sizeof(cmd), pp);

	while (fgets(cmd, sizeof(cmd), pp)) {
		if (strlen(cmd) < 4)
			break;
		ptr = cmd;
		sscanf(ptr, "%s ", channel);
		ptr += 37;
		sscanf(ptr, "%s %s %s %s %s %s", bssid, security, signal, mode, ext_ch, net_type);
		ptr = cmd+4;
		i = 0;
		while (i < 33) {
			if ((ptr[i] == 0x20) && (i == 0 || ptr[i-1] != 0x20))
				space_start = i;
			i++;
		}
		ptr[space_start] = '\0';
		strcpy(ssid, cmd+4);
		convert_string_display(ssid);
#if 0
		printf("<tr>\n");
		printf("<td>%s</td><td>%s</td>\n", channel, ssid);
		printf("<td>%s</td><td>%s</td>\n", bssid, security);
		printf("<td>%s</td><td>%s</td>\n", signal, mode);
		printf("<td>%s</td><td>%s</td>\n", ext_ch, net_type);
		printf("</tr>\n");
#else

	if(!strcmp(argv[1], WLAN2_CONF))
	{
		count++;
		printf("<tr style=\"cursor:pointer;\" id=\"%d\">\r\n",count);
		printf("<td id=\"channel%d\">%s</td><td id=\"ssid%d\">%s</td>\r\n", count,channel,count, ssid);
		printf("<td id=\"security%d\">%s</td><td id=\"signal%d\">%s</td><td><input type=\"radio\" name=\"rowSelect\" id=\"select%d\"></td>\r\n",count, security, count,signal, count);
		printf("</tr>\r\n");
		
		sprintf(count_s,"%d",count);
	}
	else
	{
		count++;
		printf("<tr style=\"cursor:pointer;\" id=\"%d\">\r\n",count);
		printf("<td id=\"channel%d\">%s</td><td id=\"ssid%d\">%s</td>\r\n", count,channel,count, ssid);
		printf("<td id=\"security%d\">%s</td><td id=\"signal%d\">%s</td><td><input type=\"radio\" name=\"rowSelect\" id=\"select%d\"></td>\r\n",count, security, count,signal, count);
		printf("</tr>\r\n");
		
	}
#endif
	}

	if(!strcmp(argv[1], WLAN2_CONF))
		{
			nvram_bufset(RTDEV_NVRAM, "apcli5count",count_s);
		}
	pclose(pp);
}

int comp(const void *p1, const void *p2)
{
	int signal1, signal2;

	struct apcliSsidInfo *item1 = (struct apcliSsidInfo*)p1;
    struct apcliSsidInfo *item2 = (struct apcliSsidInfo*)p2;
	signal1 = atoi(item1->apcliSignal);
	signal2 = atoi(item2->apcliSignal);

	int flag = 0;
	if (signal1 > signal2) {
		flag = -1;	
	} else if (signal1 < signal2) {
	  flag = 1;
	}
	
	return flag;
}

void ApcliScanAll(void)
{
	FILE *pp;
	char cmd[CMDLEN], *ptr, wif[5],count_s[8]={0};
	char channel[4], ssid[186], bssid[20], security[23];
	char signal[9], mode[7], ext_ch[7], net_type[3];
	int i,j,temp, space_start,count=0;

	struct apcliSsidInfo apclInfo[MAX_SSID_NUM]={0};

	system("echo get_site_survey > /tmp/apcliSsid");		

	do_system("iwpriv %s set SiteSurvey=1","ra0");

	sleep(1);
	
	sprintf(cmd, "iwpriv %s get_site_survey","ra0");	
	if(!(pp = popen(cmd, "r"))) {
		DBG_MSG("execute get_site_survey fail!");
		return;
	}

	memset(cmd, 0, sizeof(cmd));
	fgets(cmd, sizeof(cmd), pp);
	fgets(cmd, sizeof(cmd), pp);


	while (fgets(cmd, sizeof(cmd), pp)) {
		if (strlen(cmd) < 4)
			break;
		ptr = cmd;
		sscanf(ptr, "%s ", channel);
		ptr += 37;
		sscanf(ptr, "%s %s %s %s %s %s", bssid, security, signal, mode, ext_ch, net_type);
		ptr = cmd+4;
		i = 0;
		while (i < 33) {
			if ((ptr[i] == 0x20) && (i == 0 || ptr[i-1] != 0x20))
				space_start = i;
			i++;
		}
		ptr[space_start] = '\0';
		strcpy(ssid, cmd+4);
		convert_string_display(ssid);

		if(strlen(ssid) == 0)
			continue;
		
		sprintf(apclInfo[count].apcliChannel,"%s",channel);
		sprintf(apclInfo[count].apcliSsid,"%s",ssid);
		sprintf(apclInfo[count].apcliSecurity,"%s",security);
		sprintf(apclInfo[count].apcliSignal,"%d",atoi(signal));
		apclInfo[count].flag=0;//2.4g

		count++;		
	}	

	pclose(pp);

#if defined (CONFIG_RTDEV)
	do_system("iwpriv %s set SiteSurvey=1","rai0");

	sleep(1);

	sprintf(cmd, "iwpriv %s get_site_survey","rai0");
	if(!(pp = popen(cmd, "r"))) {
		DBG_MSG("execute get_site_survey fail!");
		return;
	}

	memset(cmd, 0, sizeof(cmd));
	fgets(cmd, sizeof(cmd), pp);
	fgets(cmd, sizeof(cmd), pp);


	while (fgets(cmd, sizeof(cmd), pp)) {
		if (strlen(cmd) < 80)
			break;
		ptr = cmd;
		sscanf(ptr, "%s ", channel);
		ptr += 37;
		sscanf(ptr, "%s %s %s %s %s %s", bssid, security, signal, mode, ext_ch, net_type);
		ptr = cmd+4;
		i = 0;
		while (i < 33) {
			if ((ptr[i] == 0x20) && (i == 0 || ptr[i-1] != 0x20))
				space_start = i;
			i++;
		}
		ptr[space_start] = '\0';
		strcpy(ssid, cmd+4);
		convert_string_display(ssid);

		if(strlen(ssid) == 0)
			continue;
	
		ptr = cmd + 80;
		i = 0;
		while (i < 10) {
			if ((ptr[i] == 0x20) && (i == 0 || ptr[i-1] != 0x20))
				space_start = i;
			i++;
		}
		ptr[space_start] = '\0';
		strcpy(signal, cmd+80);
		
		sprintf(apclInfo[count].apcliChannel,"%s",channel);
		sprintf(apclInfo[count].apcliSsid,"%s",ssid);
		sprintf(apclInfo[count].apcliSecurity,"%s",security);
		sprintf(apclInfo[count].apcliSignal,"%d",atoi(signal));
		apclInfo[count].flag=1;//5g

		count++;		
	}	

	pclose(pp);


#endif

	/** Sort by signal from strong to weak
	for(i=0;i<count;i++) 
		for(j=0;j<count-i;j++)
			{
				if(atoi(apclInfo[j].apcliSignal) < atoi(apclInfo[j+1].apcliSignal))
					{
						temp = atoi(apclInfo[j+1].apcliSignal);
						sprintf(apclInfo[j+1].apcliSignal,"%s",apclInfo[j].apcliSignal);
						sprintf(apclInfo[j].apcliSignal,"%d",temp);		
				}
			}
	**/

	qsort(apclInfo, count, sizeof(struct apcliSsidInfo), comp);

	for(i=0;i<count;i++)
	{		
		printf("<tr style=\"cursor:pointer;\" id=\"%d\">\r\n",i);
		printf("<td id=\"channel%d\">%s</td><td id=\"ssid%d\">%s</td>\r\n",i,apclInfo[i].apcliChannel,i, apclInfo[i].apcliSsid);
		printf("<td id=\"security%d\">%s</td><td id=\"signal%d\">%s</td><td><input type=\"radio\" name=\"rowSelect\" id=\"select%d\"></td>\r\n",i, apclInfo[i].apcliSecurity, i,apclInfo[i].apcliSignal, i);
		printf("<td style=\"display:none;\" id=\"netType%d\">%d</td>\r\n", i, apclInfo[i].flag);
		printf("</tr>\r\n");
	}	
}


#endif

void GetPINCode(char *argv[])
{
	int socket_id;
	struct iwreq wrq;
	unsigned int data = 0;

	//DBG_MSG();
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	//DBG_MSG("socket id = %d", socket_id);
	if (!strcmp(argv[1], WLAN2_CONF)) {
		sprintf(wrq.ifr_name, "rai0");
	} else {
		sprintf(wrq.ifr_name, "ra0");
	}
	//DBG_MSG("if: %s",wrq.ifr_name);
	wrq.u.data.length = sizeof(data);
	wrq.u.data.pointer = (caddr_t) &data;
	wrq.u.data.flags = RT_OID_WSC_PIN_CODE;
	//DBG_MSG();
	if( ioctl(socket_id, RT_PRIV_IOCTL, &wrq) == -1)
		DBG_MSG("ioctl error");
	close(socket_id);
	//DBG_MSG();
	printf("%08d", data);
}

void Andmu_GetRouterWireless(char *argv[])
{
	FILE *fp;
	struct dhcpOfferedAddr {
		unsigned char hostname[16];
		unsigned char mac[16];
		unsigned long ip;
		unsigned long expires;
	} lease;

	char sMacAddr[32] = {0};
	int i, j, fd, nCount;
	char *pTemp;
	int findFlag =0;

#define MAC(addr) \
        		((unsigned char *)&addr)[0], \
        		((unsigned char *)&addr)[1], \
        		((unsigned char *)&addr)[2], \
        		((unsigned char *)&addr)[3], \
        		((unsigned char *)&addr)[4], \
        		((unsigned char *)&addr)[5]

#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x"

	
	struct in_addr addr;
	struct iwreq iwr;
	RT_802_11_MAC_TABLE table = {0};
	//Get Online Users
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	if (!strcmp(argv[1], WLAN2_CONF))
		strncpy(iwr.ifr_name, "rai0", IFNAMSIZ);
	else
		strncpy(iwr.ifr_name, "ra0", IFNAMSIZ);
	
	iwr.u.data.pointer = (caddr_t) &table;
	if (fd < 0) {
		return;
	}

	if (ioctl(fd, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &iwr) < 0) {
		close(fd);
		return;
	}
	close(fd);

	//Get DHCP Users
	do_system("killall -q -USR1 udhcpd");
	fp = fopen("/var/udhcpd.leases", "r");
	if (NULL == fp) {
		return;
	}
	
	
	nCount = 0;
	
	for (i = 0; i < table.Num; i++) {
	
		findFlag=0;
		RT_802_11_MAC_ENTRY *pe = &(table.Entry[i]);
		//DBG_MSG("pe->Addr=["MAC_FMT"]\n",MAC(pe->Addr));

		//如果在dhcp中，则取出hostname等相关信息
		while (fread(&lease, 1, sizeof(lease), fp) == sizeof(lease)) 
		{
			//DBG_MSG("=lease.mac["MAC_FMT"],pe->Addr=["MAC_FMT"]\n",MAC(lease.mac),MAC(pe->Addr));
			//mac compare
			if (!strncmp(pe->Addr, lease.mac, 5)) {
				
				findFlag=1;
								
				if (nCount > 0) 
					printf(";");
				
				//1. macAddr
				pTemp = sMacAddr;
				sprintf(pTemp, "%02X", lease.mac[0]);
				pTemp += 2; 
				
				for (j = 1; j < 6; j++) {
					sprintf(pTemp, ":%02X", lease.mac[j]);
					pTemp += 3;
				}

				//2. deviceName

				//3. ipAddr
				addr.s_addr = lease.ip;

				printf("%s,%s,%s", sMacAddr, inet_ntoa(addr), lease.hostname);
				nCount++;
				break;
			}	
		}

		//没有在dhcp中
		if(0 == findFlag)
		{
			if (nCount > 0) 
					printf(";");
		
			printf(MAC_FMT",%s,%s",  MAC(pe->Addr), "", "");
			nCount++;
		}
	}
	
	fclose(fp);
	return;
}

static void bridge_dhcp_filter(int sockid, char *data)
{
	#define MAX_PAYLOAD 4096 /* maximum payload size*/
	
	struct sockaddr_nl src_addr, dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	int sock_fd;
	struct msghdr msg;	//msghdr includes: struct iovec *	msg_iov;

	sock_fd=socket(PF_NETLINK, SOCK_RAW, sockid);
	if(sock_fd<0) return;

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid(); /* self pid */

	bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;		/* For Linux Kernel */
	dest_addr.nl_groups = 0;	/* unicast */
	
	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid();	//self pid
	nlh->nlmsg_flags = 0;

	strcpy(NLMSG_DATA(nlh), "Hello");	//put "Hello" into nlh

	iov.iov_base = (void *)nlh; 		//iov -> nlh
	iov.iov_len = nlh->nlmsg_len;
	msg.msg_name = (void *)&dest_addr;	//msg_name is Socket name: dest
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov; 				//msg -> iov
	msg.msg_iovlen = 1;

  //  DBG_MSG("Sending message to kernel\n");
	sendmsg(sock_fd,&msg,0);	//msg -> find the (destination) socket name: dest
	/* Read message from kernel */
	recvmsg(sock_fd, &msg, 0);	//msg is also receiver for read
//	DBG_MSG("Received message payload: %s\n", NLMSG_DATA(nlh));  //msg -> iov -> nlh
	strncpy(data,NLMSG_DATA(nlh),strlen(NLMSG_DATA(nlh)));
   
   	//释放内存资源
	if (nlh) free(nlh);
	
	close(sock_fd);
}


void GetStatClients(char *argv[])
{
	char data[4096]={0};
	bridge_dhcp_filter(28, data);
	printf("%s", data);
}

void Andmu_GetAPWireless(char *argv[])
{
#define MAC(addr) \
					((unsigned char *)&addr)[0], \
					((unsigned char *)&addr)[1], \
					((unsigned char *)&addr)[2], \
					((unsigned char *)&addr)[3], \
					((unsigned char *)&addr)[4], \
					((unsigned char *)&addr)[5]
	
#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x"

	char data[4096]={0};
	char data_output[4096]={0};
	char onlineMac[64][24]={0};
	char* temp=NULL;
	char temp_s[64]={0};
	int fd,i,count;
	
	struct iwreq iwr;
	RT_802_11_MAC_TABLE table = {0};
	//Get Online Users
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	if (!strcmp(argv[1], WLAN2_CONF))
		strncpy(iwr.ifr_name, "rai0", IFNAMSIZ);
	else
		strncpy(iwr.ifr_name, "ra0", IFNAMSIZ);
	
	iwr.u.data.pointer = (caddr_t) &table;
	if (fd < 0) {
		return;
	}

	if (ioctl(fd, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &iwr) < 0) {
		close(fd);
		return;
	}
	close(fd);

	count = table.Num; 
	for (i = 0; i < table.Num; i++) {
		
		RT_802_11_MAC_ENTRY *pe = &(table.Entry[i]);
		sprintf(onlineMac[i],MAC_FMT,MAC(pe->Addr));
//		printf("onlineMac:%s\n",onlineMac[i]);
		}
	
	bridge_dhcp_filter(27, data);
//	printf("data:%s\n",data);
	
	temp = strtok(data,"\n");
	if (temp == NULL) {
		printf("");
		return;
	}
	do{
		for (i = 0; i < count+1; i++) {
			if(strncasecmp(onlineMac[i],temp,17) == 0)
				{
					memset(temp_s,0,64);
					sprintf(temp_s,"%s;",temp);
					strcat(data_output,temp_s);
					
					break;
				}
			}
			 temp = strtok(NULL,"\n");
		}while(temp);
	
	printf("%s",data_output);
}


void Andmu_GetWireless(char *argv[]) {

	nvram_init(RT2860_NVRAM);
	char *operationMode = nvram_bufget(RT2860_NVRAM, "OperationMode");
	if (operationMode != NULL && operationMode[0] == '1') { //Router Mode
		Andmu_GetRouterWireless(argv);
	} else {
		Andmu_GetAPWireless(argv);
	}
	nvram_close(RT2860_NVRAM);
}

void Andmu_GetWired()
{
	FILE *fp;
	struct dhcpOfferedAddr {
		unsigned char hostname[16];
		unsigned char mac[16];
		unsigned long ip;
		unsigned long expires;
	} lease;

	char sMacAddr[32] = {0};
	int i, j, fd, nCount, nFound;
	char *pTemp;
	struct in_addr addr;
	struct iwreq iwr;
	RT_802_11_MAC_TABLE table = {0};
	//Get Online Users
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(iwr.ifr_name, "ra0", IFNAMSIZ);
	iwr.u.data.pointer = (caddr_t) &table;
	if (fd < 0) {
		return;
	}

	if (ioctl(fd, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &iwr) < 0) {
		close(fd);
		return;
	}
	close(fd);
//获取5G列表	
#if defined CONFIG_RTDEV
	int fd_5G;
	struct in_addr addr_5G;
	struct iwreq iwr_5G;
	RT_802_11_MAC_TABLE table_5G = {0};
	//Get Online Users
	fd_5G = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(iwr_5G.ifr_name, "rai0", IFNAMSIZ);
	iwr_5G.u.data.pointer = (caddr_t) &table_5G;
	if (fd_5G < 0) {
		return;
	}

	if (ioctl(fd_5G, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &iwr_5G) < 0) {
		close(fd_5G);
		return;
	}
	close(fd_5G);
#endif
	//Get DHCP Users
	do_system("killall -q -USR1 udhcpd");
	fp = fopen("/var/udhcpd.leases", "r");
	if (NULL == fp) {
		return;
	}
	
	nCount = nFound = 0;
	while (fread(&lease, 1, sizeof(lease), fp) == sizeof(lease)) {
		nFound = 0;
		for (i = 0; i < table.Num; i++) {			
			RT_802_11_MAC_ENTRY *pe = &(table.Entry[i]);			 
			//mac compare
			if (!strncmp(pe->Addr, lease.mac, 5)) {
				nFound = 1;
				break;
			}			
		}
		
#if defined CONFIG_RTDEV
		for (i = 0; i < table_5G.Num; i++) {			
			RT_802_11_MAC_ENTRY *pe_5G = &(table_5G.Entry[i]);			 
			//mac compare
			if (!strncmp(pe_5G->Addr, lease.mac, 5)) {
				nFound = 1;
				break;
			}			
		}
#endif
		//不在无线列表中的数据，则表示为有线
		if (!nFound) {
			if (nCount > 0) printf(";");
			
			//1. macAddr
			pTemp = sMacAddr;
			sprintf(pTemp, "%02X", lease.mac[0]);
			pTemp += 2; 
			
			for (j = 1; j < 6; j++) {
				sprintf(pTemp, ":%02X", lease.mac[j]);
				pTemp += 3;
			}

			//2. deviceName

			//3. ipAddr
			addr.s_addr = lease.ip;

			printf("%s,%s,%s", sMacAddr, inet_ntoa(addr), lease.hostname);
			nCount++;
		}

	}
	
	fclose(fp);
	return;
}

void Andmu_GetWirelessCount()
{
	FILE *fp;

	int i,  fd;
	int Wirelesscount=0;
	struct in_addr addr;
	struct iwreq iwr;
	RT_802_11_MAC_TABLE table = {0};
	//Get Online Users
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(iwr.ifr_name, "ra0", IFNAMSIZ);
	iwr.u.data.pointer = (caddr_t) &table;
	if (fd < 0) {
		printf("%d", 0);
		return;
	}

	if (ioctl(fd, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &iwr) < 0) {
		close(fd);
		printf("%d", 0);
		return;
	}
	close(fd);
	Wirelesscount += (int)table.Num;

	//获取5G列表	
#if defined CONFIG_RTDEV
		int fd_5G;
		struct in_addr addr_5G;
		struct iwreq iwr_5G;
		RT_802_11_MAC_TABLE table_5G = {0};
		//Get Online Users
		fd_5G = socket(AF_INET, SOCK_DGRAM, 0);
		strncpy(iwr_5G.ifr_name, "rai0", IFNAMSIZ);
		iwr_5G.u.data.pointer = (caddr_t) &table_5G;
		if (fd_5G < 0) {
			return;
		}
	
		if (ioctl(fd_5G, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &iwr_5G) < 0) {
			close(fd_5G);
			return;
		}
		close(fd_5G);
		Wirelesscount += (int)table_5G.Num;
#endif

	printf("%d", Wirelesscount);
	return;

}


void WebWifiGet(char *argv[])
{
	if (!strcmp(argv[3], "version")) {
		GetWifiVersion(argv);
	} else if (!strcmp(argv[3], "getAChannels")) {
		GetAChannels(argv);
	} else if (!strcmp(argv[3], "getGChannels")) {
		GetGChannels(argv);
	} else if (!strcmp(argv[3], "wpsPINCode")) {
		GetPINCode(argv);
	} else if (!strcmp(argv[3], "staInfo")) {
		StaInfo(argv);
	} else if (!strcmp(argv[3], "apStats")) {
		ApStats(argv);
	} else if  (!strcmp(argv[3], "getWireless")) {
		Andmu_GetWireless(argv);
	} else if  (!strcmp(argv[3], "getWired")) {
		Andmu_GetWired();
	} else if  (!strcmp(argv[3], "getWirelessCount")) {
		Andmu_GetWirelessCount();
	} else if (!strcmp(argv[3], "getStatClients")) {
		GetStatClients(argv);
	} else if (!strcmp(argv[3], "apSNR")) {
		ApSNR(argv);
#if defined (RT2860_TXBF_SUPPORT)
	} else if (!strcmp(argv[3], "apTxBfStats") && !strcmp(argv[1], WLAN1_CONF)) {
		ApTxBfStats(argv);
#endif
#if defined (RTDEV_TXBF_SUPPORT)
	} else if (!strcmp(argv[3], "apTxBfStats") && !strcmp(argv[1], WLAN2_CONF)) {
		ApTxBfStats(argv);
#endif
	} else if (!strcmp(argv[3], "maxStream")) {
#if defined (RT2860_3T3R_SUPPORT)
		printf("%d", 3);
#elif defined (RT2860_1T1R_SUPPORT)
		printf("%d", 1);
#else
		printf("%d", 2);
#endif
	} else if (!strcmp(argv[3], "maxBssNum")) {
#if defined (CONFIG_NEW_MBSSID_MODE)
		printf("%d", 16);
#elif defined (RT2860_MBSS_SUPPORT)
		printf("%d", 8);
#else
		printf("%d", 1);
#endif
#if defined (RT2860_APCLI_SUPPORT)
	} else if (!strcmp(argv[3], "ApcliScan")) {
		ApcliScan(argv);
	} else if (!strcmp(argv[3], "ApcliScanAll")) {
		ApcliScanAll();
#endif
	}
}

void GetGateway(void)
{
	char   buff[256];
	int    nl = 0 ;
	struct in_addr dest;
	struct in_addr gw;
	int    flgs, ref, use, metric;
	unsigned long int d,g,m;
	int    find_default_flag = 0;
	char sgw[16];
	FILE *fp = fopen("/proc/net/route", "r");

	while (fgets(buff, sizeof(buff), fp) != NULL) {
		if (nl) {
			int ifl = 0;
			while (buff[ifl]!=' ' && buff[ifl]!='\t' && buff[ifl]!='\0')
				ifl++;
			buff[ifl]=0;    /* interface */
			if (sscanf(buff+ifl+1, "%lx%lx%X%d%d%d%lx",
						&d, &g, &flgs, &ref, &use, &metric, &m)!=7) {
				fclose(fp);
				DBG_MSG("format error");
				return;
			}

			if (flgs&RTF_UP) {
				dest.s_addr = d;
				gw.s_addr   = g;
				strcpy(sgw, (gw.s_addr==0 ? "" : inet_ntoa(gw)));

				if (dest.s_addr == 0) {
					find_default_flag = 1;
					break;
				}
			}
		}
		nl++;
	}
	fclose(fp);

	if (find_default_flag == 1)
		printf("%s", sgw);
}

void GetDns(int index)
{
	FILE *fp;
	char buf[80] = {0}, ns_str[11], dns[16] = {0};
	int type, i = 0;

	fp = fopen("/etc/resolv.conf", "r");
	if (NULL == fp) 
		return;
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (strncmp(buf, "nameserver", 10) != 0)
			continue;
		sscanf(buf, "%s%s", ns_str, dns);
		i++;
		if (i == index)
			break;
	}
	fclose(fp);

	printf("%s", dns);
}

/*
 * description: write DHCP client list
 */
#define BYOD_SUPPORT //by chhung
void GetDhcpClientList(void)
{
	FILE *fp;
	struct dhcpOfferedAddr {
		unsigned char hostname[16];
		unsigned char mac[16];
		unsigned long ip;
		unsigned long expires;
	} lease;
	int i;
	struct in_addr addr;
	unsigned long expires;
	unsigned d, h, m;
	char tmpValue[256];

	do_system("killall -q -USR1 udhcpd");
	fp = fopen("/var/udhcpd.leases", "r");
	if (NULL == fp)
		return;
	while (fread(&lease, 1, sizeof(lease), fp) == sizeof(lease)) {
		if (strlen(lease.hostname) > 0) {
			sprintf(tmpValue, "%s", lease.hostname);
			convert_string_display(tmpValue);			
			printf("<tr><td>%-16s</td>", tmpValue);
		} else {
			printf("<tr><td><br /></td>");
		}
		printf("<td>%02X", lease.mac[0]);
		for (i = 1; i < 6; i++)
			printf(":%02X", lease.mac[i]);
		addr.s_addr = lease.ip;
		expires = ntohl(lease.expires);
		printf("</td><td>%s</td><td>", inet_ntoa(addr));
		d = expires / (24*60*60); expires %= (24*60*60);
		h = expires / (60*60); expires %= (60*60);
		m = expires / 60; expires %= 60;
		if (d) printf("%u days ", d);
		printf("%02u:%02u:%02u</td>", h, m, (unsigned)expires);
#ifdef BYOD_SUPPORT
		FILE *pp;
		char cmd[128], msg_os[64];
		sprintf(cmd, "fingerprint.sh query %s", inet_ntoa(addr));
		pp = popen(cmd, "r");
		memset(msg_os, 0, 64);
		fread(msg_os, 64, 1, pp);
		pclose(pp);
		printf("<td>%s</td>", msg_os);
#endif
		printf("</tr>");
	}
	fclose(fp);
}

char *getLanWanNamebyIf(char *ifname)
{
	const char *mode = nvram_bufget(RT2860_NVRAM, "OperationMode");

	if (NULL == mode)
		return "Unknown";

	if (!strcmp(mode, "0")){    // bridge mode
		if(!strcmp(ifname, "br0"))
			return "LAN";
		return ifname;
	}

	if (!strcmp(mode, "1")) {   // gateway mode
#if defined CONFIG_RAETH_ROUTER || defined CONFIG_MAC_TO_MAC_MODE || defined CONFIG_RT_3052_ESW
		if(!strcmp(ifname, "br0"))
			return "LAN";
#if defined (CONFIG_RAETH_SPECIAL_TAG)
#if defined (CONFIG_WAN_AT_P0)
		if(!strcmp(ifname, "eth2.1") || !strcmp(ifname, "ppp0"))
#else
			if(!strcmp(ifname, "eth2.5") || !strcmp(ifname, "ppp0"))
#endif
#else
				if(!strcmp(ifname, "eth2.2") || !strcmp(ifname, "ppp0") || !strncmp(ifname,"eth2.2.",7))
#endif
					return "WAN";
		return ifname;
#elif defined  CONFIG_ICPLUS_PHY && CONFIG_RT2860V2_AP_MBSS
		char *num_s = nvram_bufget(RT2860_NVRAM, "BssidNum");
		if(strtol(num_s, NULL, 10) > 1 && !strcmp(ifname, "br0") )  // multiple ssid
			return "LAN";
		if(strtol(num_s, NULL, 10) == 1 && !strcmp(ifname, "ra0"))
			return "LAN";
		if (!strcmp(ifname, "eth2") || !strcmp(ifname, "ppp0"))
			return "WAN";
		return ifname;
#elif defined (CONFIG_GE1_RGMII_AN) && defined (CONFIG_GE2_RGMII_AN)
		if(!strcmp(ifname, "br0"))
			return "LAN";
		if(!strcmp(ifname, "eth3"))
			return "WAN";
#else
		if(!strcmp(ifname, "ra0"))
			return "LAN";
		return ifname;
#endif

	}else if (!strncmp(mode, "2", 2)) { // ethernet convertor
		if(!strcmp("eth2", ifname))
			return "LAN";
		if(!strcmp("ra0", ifname))
			return "WAN";
		return ifname;
	}else if (!strncmp(mode, "3", 2)) { // apcli mode
		if(!strcmp("br0", ifname))
			return "LAN";
		if(!strcmp("apcli0", ifname))
			return "WAN";
		return ifname;
	}

	return ifname;
}

void GetRoutingTable(void)
{
	char   result[4096] = {0};
	char   buff[512];
	int    nl = 0, index;
	char   ifname[32], interface[128];
	struct in_addr dest, gw, netmask;
	char   dest_str[32], gw_str[32], netmask_str[32], comment[32];
	int    flgs, ref, use, metric;
	int    *running_rules = NULL;
	unsigned long int d,g,m;
	char *rrs;
	int  rule_count;
	FILE *fp = fopen("/proc/net/route", "r");
	if(!fp)
		return;

	rrs = (char *) nvram_bufget(RT2860_NVRAM, "RoutingRules");
	rule_count = get_nums(rrs, ';');
	if(rule_count){
		running_rules = calloc(1, sizeof(int) * rule_count);
		if(!running_rules) {
			fclose(fp);
			return;
		}
	}

	while (fgets(buff, sizeof(buff), fp) != NULL) {
		if (nl) {
			if (sscanf(buff, "%s%lx%lx%X%d%d%d%lx",
						ifname, &d, &g, &flgs, &ref, &use, &metric, &m) != 8) {
				DBG_MSG("format error");
				free(running_rules);
				fclose(fp);
				printf("");
				return;
			}
			dest.s_addr = d;
			gw.s_addr = g;
			netmask.s_addr = m;

			if(! (flgs & 0x1) ) // skip not usable
				continue;

			strncpy(dest_str, inet_ntoa(dest), sizeof(dest_str));
			strncpy(gw_str, inet_ntoa(gw), sizeof(gw_str));
			strncpy(netmask_str, inet_ntoa(netmask), sizeof(netmask_str));

			if(nl > 1)
				strncat(result, ";", sizeof(result));
			strncat(result, ifname, sizeof(result));        strncat(result, ",", sizeof(result));
			strncat(result, dest_str, sizeof(result));      strncat(result, ",", sizeof(result));
			strncat(result, gw_str, sizeof(result));            strncat(result, ",", sizeof(result));
			strncat(result, netmask_str, sizeof(result) );  strncat(result, ",", sizeof(result));
			snprintf(result, sizeof(result), "%s%d,%d,%d,%d,", result, flgs, ref, use, metric);

			// check if internal routing rules
			strcpy(comment, " ");
			if( (index=get_index_routingrule(rrs, dest_str, netmask_str, ifname)) != -1){
				char one_rule[256];

				if(index < rule_count)
					running_rules[index] = 1;
				else
					DBG_MSG("fatal error");

				snprintf(result, sizeof(result), "%s%d,", result, index);
				if(rrs && strlen(rrs)){
					if( get_nth_value(index, (char *)rrs, ';', one_rule, sizeof(one_rule)) != -1){

						if( get_nth_value(3, one_rule, ',', interface, sizeof(interface)) != -1){
							strncat(result, interface, sizeof(result));
							strncat(result, ",", sizeof(result));
						}
						if( get_nth_value(6, one_rule, ',', comment, sizeof(comment)) != -1){
							// do nothing;
						}
					}
				}
			}else{
				strncat(result, "-1,", sizeof(result));
				strncat(result, getLanWanNamebyIf(ifname), sizeof(result));
				strncat(result, ",", sizeof(result));
			}
			strncat(result, "0,", sizeof(result));  // used rule
			strncat(result, comment, sizeof(result));
		}
		nl++;
	}

	for(index=0; index < rule_count; index++){
		char one_rule[256];

		if(running_rules[index])
			continue;

		if(get_nth_value(index, (char *)rrs, ';', one_rule, sizeof(one_rule)) == -1)
			continue;

		if(get_nth_value(0, one_rule, ',', dest_str, sizeof(dest_str)) == -1)
			continue;

		if(get_nth_value(1, one_rule, ',', netmask_str, sizeof(netmask_str)) == -1)
			continue;

		if(get_nth_value(2, one_rule, ',', gw_str, sizeof(gw_str)) == -1)
			continue;

		if(get_nth_value(3, one_rule, ',', interface, sizeof(interface)) == -1)
			continue;

		if(get_nth_value(4, one_rule, ',', ifname, sizeof(ifname)) == -1)
			continue;

		if(get_nth_value(6, one_rule, ',', comment, sizeof(comment)) == -1)
			continue;

		if(strlen(result))
			strncat(result, ";", sizeof(result));

		snprintf(result, sizeof(result), "%s%s,%s,%s,%s,0,0,0,0,%d,%s,1,%s", result, ifname, dest_str, gw_str, netmask_str, index, interface, comment);
	}

	printf("%s", result);
	fclose(fp);
	if(running_rules)
		free(running_rules);
}

void GetCurrentTime(void)
{
	char buf[64];
	FILE *pp = popen("date", "r");
	char *temp;

	if (!pp) {
		printf("none");
		return;
	}
	fgets(buf, 64, pp);
	pclose(pp);
	temp =  strchr(buf, '\n');
	*temp = '\0';

	printf("%s", buf);
}

void GetHeMuCurrentTime(void)
{   
	time_t time_T;
    time_T = time(NULL);

    struct tm *tmTime;
    // tm对象格式的时间
    tmTime = localtime(&time_T);

   // printf("Now Time is: %d:%d:%d\n", (*tmTime).tm_hour, (*tmTime).tm_min, (*tmTime).tm_sec);

    char* format = "%Y-%m-%d %H:%M:%S";
    char strTime[100];
    strftime(strTime, sizeof(strTime), format, tmTime);

    printf("%s", strTime);
}

void GetMemTotal(void)
{
	char buf[1024], *semiColon, *key, *value;
	FILE *fp = fopen(PROC_MEM_STATISTIC, "r");

	if(!fp){
		DBG_MSG("no proc?");
		return;
	}

	while(fgets(buf, 1024, fp)){
		if(! (semiColon = strchr(buf, ':'))  )
			continue;
		*semiColon = '\0';
		key = buf;
		value = semiColon + 1;
		if(!strcmp(key, "MemTotal")){
			value = strip_space(value);
			printf("%s", value);
			break;
		}
	}
	fclose(fp);
}

void GetMemFree(void)
{
	char buf[1024], *semiColon, *key, *value;
	FILE *fp = fopen(PROC_MEM_STATISTIC, "r");

	if(!fp){
		DBG_MSG("no proc?");
		return;
	}

	while(fgets(buf, 1024, fp)){
		if(! (semiColon = strchr(buf, ':'))  )
			continue;
		*semiColon = '\0';
		key = buf;
		value = semiColon + 1;
		if(!strcmp(key, "MemFree")){
			value = strip_space(value);
			printf("%s", value);
			break;
		}
	}
	fclose(fp);
}

static long long getIfStatistic(char *interface, int type)
{
	int found_flag = 0;
	int skip_line = 2;
	char buf[1024], *field, *semiColon = NULL;
	FILE *fp = fopen(PROC_IF_STATISTIC, "r");
	if(!fp){
		DBG_MSG("no proc?");
		return -1;
	}

	while(fgets(buf, 1024, fp)){
		char *ifname;
		if(skip_line != 0){
			skip_line--;
			continue;
		}
		if(! (semiColon = strchr(buf, ':'))  )
			continue;
		*semiColon = '\0';
		ifname = buf;
		ifname = strip_space(ifname);

		if(!strcmp(ifname, interface)){
			found_flag = 1;
			break;
		}
	}
	fclose(fp);

	semiColon++;

	switch(type){
	case TXBYTE:
		if(  (field = get_field(semiColon, " ", 8))  ){
			return strtoll(field, NULL, 10);
		}
		break;
	case TXPACKET:
		if(  (field = get_field(semiColon, " ", 9))  ){
			return strtoll(field, NULL, 10);
		}
		break;
	case RXBYTE:
		if(  (field = get_field(semiColon, " ", 0))  ){
			return strtoll(field, NULL, 10);
		}
		break;
	case RXPACKET:
		if(  (field = get_field(semiColon, " ", 1))  ){
			return strtoll(field, NULL, 10);
		}
		break;
	}

	return -1;
}

void GetAllNICStatisticASP(void)
{
	char result[1024];
	char buf[1024];
	int rc = 0, pos = 0, skip_line = 2;
	int first_time_flag = 1;
	FILE *fp = fopen(PROC_IF_STATISTIC, "r");
	if (!fp) {
		DBG_MSG("no proc?");
		return;
	}

	while(fgets(buf, 1024, fp)){
		char *ifname, *semiColon;
		long long rxpkt, rxbyte, txpkt, txbyte;
		if(skip_line != 0){
			skip_line--;
			continue;
		}
		if(! (semiColon = strchr(buf, ':'))  )
			continue;
		*semiColon = '\0';

		ifname = buf;
		ifname = strip_space(ifname);

		rxpkt = getIfStatistic(ifname, RXPACKET);
		rxbyte = getIfStatistic(ifname, RXBYTE);
		txpkt = getIfStatistic(ifname, TXPACKET);
		txbyte = getIfStatistic(ifname, TXBYTE);
		if (rxpkt <= 0 && rxbyte <= 0 && txpkt <= 0 && txbyte <= 0)
			continue;
		/* try to get statistics data */
		if(getIfStatistic(ifname, RXPACKET) >= 0){
			/* a success try */
			if(first_time_flag){
				pos = snprintf(result+rc, 1024-rc, "\"%s\"", ifname);
				rc += pos;
				first_time_flag = 0;
			}else{
				pos = snprintf(result+rc, 1024-rc, ",\"%s\"", ifname);
				rc += pos;
			}

		}else   /* failed and just skip */
			continue;

		pos = snprintf(result+rc, 1024-rc, ",\"%lld\"", rxpkt);
		rc += pos;
		pos = snprintf(result+rc, 1024-rc, ",\"%lld\"", rxbyte);
		rc += pos;
		pos = snprintf(result+rc, 1024-rc, ",\"%lld\"", txpkt);
		rc += pos;
		pos = snprintf(result+rc, 1024-rc, ",\"%lld\"", txbyte);
		rc += pos;
	}
	fclose(fp);

	printf("%s", result);
}

#define ETH_ALEN	6		/* Octets in one ethernet addr	 */
#define ETH_HLEN	14		/* Total octets in header.	 */
#define ETH_ZLEN	60		/* Min. octets in frame sans FCS */
#define ETH_DATA_LEN	1500		/* Max. octets in payload	 */
#define ETH_FRAME_LEN	1514		/* Max. octets in frame sans FCS */
/* PPPoE codes */
#define CODE_PADI           0x09
#define CODE_PADO           0x07
#define CODE_PADR           0x19
#define CODE_PADS           0x65
#define CODE_PADT           0xA7
#define CODE_SESS           0x00
/* Ethernet frame types according to RFC 2516 */
#define ETH_PPPOE_DISCOVERY 0x8863
#define ETH_PPPOE_SESSION   0x8864
/* Header size of a PPPoE packet */
#define PPPOE_OVERHEAD 6  /* type, code, session, length */
#define HDR_SIZE (sizeof(struct ethhdr) + PPPOE_OVERHEAD)
#define MAX_PPPOE_PAYLOAD (ETH_DATA_LEN - PPPOE_OVERHEAD)
#define MAX_PPPOE_MTU (MAX_PPPOE_PAYLOAD - 2)

struct ethhdr 
{
	unsigned char	h_dest[ETH_ALEN];	/* destination eth addr	*/
	unsigned char	h_source[ETH_ALEN];	/* source ether addr	*/
	unsigned short	h_proto;		/* packet type ID field	*/
};

/* A PPPoE Packet, including Ethernet headers */
typedef struct PPPoEPacketStruct {
    struct ethhdr ethHdr;	/* Ethernet header */

    unsigned int type:4;	/* PPPoE Type (must be 1) */
    unsigned int ver:4;		/* PPPoE Version (must be 1) */
    unsigned int code:8;	/* PPPoE code */
    unsigned int session:16;	/* PPPoE session */
    unsigned int length:16;	/* Payload length */
    unsigned char payload[ETH_DATA_LEN]; /* A bit of room to spare */
} PPPoEPacket;

#include<linux/if_packet.h>
#include <errno.h>

/* Initial timeout for PADO/PADS */
#define PADI_TIMEOUT 2

void GetConnectType(void)
{
	PPPoEPacket packet;
	int optval=1;
	int sock;
	struct sockaddr_ll addr;

	if ((sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_PPPOE_DISCOVERY))) < 0) {
		DBG_MSG("Failed to create PPPoE socket: %m");
		goto failed;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sll_family = PF_PACKET;
	addr.sll_ifindex = if_nametoindex(get_wanif_name());

	/* Set destination to Ethernet broadcast address */
	//00:0C:29:59:41:C6
	unsigned char myEth[6] = {0};
	myEth[0] = 0xB0;
	myEth[1] = 0xD5;
	myEth[2] = 0x9D;
	myEth[3] = 0x88;
	myEth[4] = 0x88;
	myEth[5] = 0x00;

	memset(packet.ethHdr.h_dest, 0xFF, ETH_ALEN);
	memcpy(packet.ethHdr.h_source, myEth, ETH_ALEN);

	packet.ethHdr.h_proto = htons(ETH_PPPOE_DISCOVERY);
	packet.ver = 1;
	packet.type = 1;
	packet.code = CODE_PADI;
	packet.session = 0;
	packet.length = 0;

	if (sendto(sock, &packet, 22, 0, (struct sockaddr *)(&addr), sizeof(addr)) < 0) {
		DBG_MSG("sendto (sendPacket) error");
		perror("sendto");
		goto failed;
	}

	/* Wait readable */
	fd_set readable;
	int r;
	int get_recv = 0;
	int get_PADO = 0;

	struct timeval tv;
	tv.tv_sec = PADI_TIMEOUT;
	tv.tv_usec = 0;

	FD_ZERO(&readable);
	FD_SET(sock, &readable);

	while(1)
	{
		r = select(sock+1, &readable, NULL, NULL, &tv);
		/* EINTR 该调用被信号所中断 */
		if (r > 0)
		{
			get_recv = 1;
			break;
		}

		if (r < 0 && errno != EINTR)
			DBG_MSG("select error, But continue");

		if (r == 0) //timeout
		{
			DBG_MSG("timeout!");
			get_recv = 0;
			break;
		}
	}

	/* get PADO */
	int read_len = 0;
	PPPoEPacket recv_packet;
	if (get_recv)
	{
		if ((read_len = recv(sock, &recv_packet, sizeof(PPPoEPacket), 0)) < 0) {
			DBG_MSG("recv error");
			goto failed;
		}

		if (ntohs(recv_packet.length) + HDR_SIZE > read_len)
		{
			DBG_MSG("Bogus PPPoE length field (%u)",
				   (unsigned int) ntohs(recv_packet.length));
			goto failed;
		}

		/* If it's not a Discovery packet */
		if ((uint16_t )ntohs(recv_packet.ethHdr.h_proto) != ETH_PPPOE_DISCOVERY)
		{
			DBG_MSG("Not PADO");
			goto failed;
	    }

		if (memcmp(recv_packet.ethHdr.h_dest, packet.ethHdr.h_source, ETH_ALEN))
		{
			DBG_MSG("My eth addr or peer eth addr error!");
			goto failed;
		}

		if (recv_packet.code == CODE_PADO)
		{
			DBG_MSG("Great, get PADO!");
			get_PADO = 1;
			printf("%s", "PPPOE");
			if (sock) close(sock);
			return ;
		}
	}

failed:
	printf("%s", "DHCP");
	if (sock) close(sock);
	return ;
}

void isNeedWizard(void)
{
	char *noNeedWizard = nvram_get(RT2860_NVRAM, "noNeedWizard");

	if (noNeedWizard == NULL || strlen(noNeedWizard) == 0)
	{
		printf("%d", 1);
	}
	else
	{
		printf("%d", 0);
	}

	return ;
}

void GetPortStatus(void)
{
#if (defined (CONFIG_RAETH_ROUTER) || defined CONFIG_RT_3052_ESW) && defined (CONFIG_USER_ETHTOOL)
	int port, rc;
	FILE *pp;
	char buf[1024];

	for(port=0; port<5; port++){
		char *pos;
		char link = '0';
		int speed = 100;
		char duplex = 'F';
		FILE *proc_file = fopen("/proc/rt2880/gmac", "w");
		if(!proc_file){
			DBG_MSG("open /proc/rt2880/gmac fail!");     // indicate error
			return;
		}
		fprintf(proc_file, "%d", port);
		fclose(proc_file);

		if((pp = popen("ethtool eth2", "r")) == NULL){
			DBG_MSG("open ethtool command fail!");     // indicate error
			return;
		}
		rc = fread(buf, 1, 1024, pp);
		pclose(pp);
		if(rc == -1){
			DBG_MSG("read fail!");     // indicate error
			return;
		}else{
			// get Link status
			if((pos = strstr(buf, "Link detected: ")) != NULL){
				pos += strlen("Link detected: ");
				if(*pos == 'y')
					link = '1';
			}
			// get speed
			if((pos = strstr(buf, "Speed: ")) != NULL){
				pos += strlen("Speed: ");
				if(*pos == '1' && *(pos+1) == '0' && *(pos+2) == 'M')
					speed = 10;
				if(*pos == '1' && *(pos+1) == '0' && *(pos+2) == '0' && *(pos+3) == '0' && *(pos+4) == 'M')
					speed = 1000;
			}
			// get duplex
			if((pos = strstr(buf, "Duplex: ")) != NULL){
				pos += strlen("Duplex: ");
				if(*pos == 'H')
					duplex = 'H';
			}

			printf("%c,%d,%c,", link, speed, duplex);
		}
	}
#else
	DBG_MSG("not support!");
#endif
}

void GetSdkVersion(void)
{
#if 0
	FILE *fp = fopen("/etc_ro/web/cgi-bin/History", "r");
	char version[16] = "";

	if (fp != NULL)
	{
		char buf[30];
		while (fgets(buf, sizeof(buf), fp) != NULL) {
			if (strncasecmp(buf, "Version", 7) != 0)
				continue;
			sscanf(buf, "%*s%s", version);
			break;
		}
		fclose(fp);
	}
	printf("%s", version);
#else
FILE *fp = fopen("/etc_ro/version", "r");
char version[32] = "";

if (fp != NULL)
{
	char buf[32];
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (strstr(buf, "_V"))
			{
				sscanf(buf, "%s", version);
				break;
			}
	}
	fclose(fp);
}
printf("%s", version);

#endif
}

void GetnetStatus(void)
{
	FILE *fp = fopen("/tmp/internetstatus", "r");
	char version[8] = "";
	
	if (fp != NULL)
	{
		char buf[32];
		while (fgets(buf, sizeof(buf), fp) != NULL) {
					sscanf(buf, "%s", version);
					break;
		}
		fclose(fp);
	}
	printf("%c", version[0]);

}

#define SN_BUF_LEN		18
void router_getDevSn(void)
{
	char buf[32] = {0};	
	char g_sn[SN_BUF_LEN] = {0};

	char *ptr = NULL;

	FILE *pp = popen("eth_mac g sn", "r");
	if (pp)
	{
		while(fgets(buf, sizeof(buf), pp))
		{
			if (strlen(buf))
			{
				ptr = strstr(buf, "SN:");
				if (ptr && (strlen(ptr) > 4))
				{
					ptr += strlen("SN:");
					strncpy(g_sn, ptr, SN_BUF_LEN-1);
					g_sn[strlen(g_sn)-1] = '\0'; //cut '\n'
				}
			}
		}

		pclose(pp);
	}

	printf("%s",g_sn);
}

void GetportStatus(void)
{
	FILE *fp = fopen("/tmp/portStatus", "r");
	char portStatus[8] = "";
	
	if (fp != NULL)
	{
		char buf[32];
		while (fgets(buf, sizeof(buf), fp) != NULL) {
					sscanf(buf, "%s", portStatus);
					break;
		}
		fclose(fp);
	}
	portStatus[7] = '\0';
	printf("%s", portStatus);

}


void GetUptime(void)
{
	struct tm *utime;
	time_t usecs;

	time(&usecs);
	utime = localtime(&usecs);

	if (utime->tm_hour > 0)
		printf("%d hour%s, %d min%s, %d sec%s",
			utime->tm_hour, (utime->tm_hour == 1)? "" : "s",
			utime->tm_min, (utime->tm_min == 1)? "" : "s",
			utime->tm_sec, (utime->tm_sec == 1)? "" : "s");
	else if (utime->tm_min > 0)
		printf("%d min%s, %d sec%s",
			utime->tm_min, (utime->tm_min == 1)? "" : "s",
			utime->tm_sec, (utime->tm_sec == 1)? "" : "s");
	else
		printf("%d sec%s",
			utime->tm_sec, (utime->tm_sec == 1)? "" : "s");
}

void GetPlatform(void)
{
	char platform[9];
#if defined CONFIG_RALINK_RT2883
	strcpy(platform, "RT2883");
#elif defined CONFIG_RALINK_RT3052
	strcpy(platform, "RT3052");
#elif defined CONFIG_RALINK_RT3883
	strcpy(platform, "RT3883");
#elif defined CONFIG_RALINK_RT3352
	strcpy(platform, "RT3352");
#elif defined CONFIG_RALINK_RT5350
	strcpy(platform, "RT5350");
#elif defined CONFIG_RALINK_RT6855
	strcpy(platform, "RT6855");
#elif defined CONFIG_RALINK_RT6855A
	strcpy(platform, "RT6855/6");
#elif defined CONFIG_RALINK_MT7620
	strcpy(platform, "MT7620");
#elif defined CONFIG_RALINK_MT7621
	strcpy(platform, "MT7621");
#else
	strcpy(platform, "RT2880");
#endif

#ifdef CONFIG_RAETH_ROUTER
	printf("%s with IC+ MACPHY", platform);
#endif
#ifdef CONFIG_ICPLUS_PHY
	printf("%s with IC+ PHY", platform);
#endif
#ifdef CONFIG_RT_MARVELL
	printf("%s with MARVELL", platform);
#endif
#ifdef CONFIG_MAC_TO_MAC_MODE
	printf("%s with Vitesse", platform);
#endif
#ifdef CONFIG_RT_3052_ESW
	printf("%s embedded switch", platform);
#endif
#if defined (CONFIG_GE1_RGMII_AN) && defined (CONFIG_GE2_RGMII_AN)
	printf("%s with Two Giga PHY", platform);
#endif
}

void GetCmdShow(void)
{
	FILE *fp;
	char buf[1024];

	fp = fopen(SYSTEM_COMMAND_LOG, "r");
	if (!fp) {
		DBG_MSG("opne fail!");
		return;
	}

	while(fgets(buf, 1024, fp)){
		printf("%s", buf);
	}
	fclose(fp);
}

void GetSysLog(void)
{
	FILE *pp = NULL;
	char *log;

	pp = popen("logread", "r");
	if (!pp) {
		DBG_MSG("open logread command fail!");
		printf("-1");
		goto error;
	}

	log = malloc(LOG_MAX * sizeof(char));
	if (!log) {
		DBG_MSG("memory allocate fail!");
		printf("-1");
		goto error;
	}
	memset(log, 0, LOG_MAX);
	fread(log, 1, LOG_MAX, pp);
	printf("%s", log);

	free(log);
error:
	if(pp)
		pclose(pp);
}

void GetHeMuSysLog(void)
{
	FILE *pp = NULL;
	char *log;
	size_t cout;

	pp = popen("logread", "r");
	if (!pp) {
		DBG_MSG("open logread command fail!");
		printf("-1");
		goto error;
	}

	log = malloc(LOG_MAX * sizeof(char));
	if (!log) {
		DBG_MSG("memory allocate fail!");
		printf("-1");
		goto error;
	}
	memset(log, 0, LOG_MAX);
	cout=fread(log, 1, LOG_MAX, pp);


	FILE *fp = fopen("/var/log/logfile","w");
	if (! fp)
	{
		DBG_MSG("open /var/log/logfile by write fail!");
		printf("-1");
		free(log);
		goto error;
	}
	fwrite(log,  sizeof(char), cout, fp);//写入文件
	fclose(fp);
	
	fp = fopen("/var/log/logfile","r");
	if (! fp)
	{
		DBG_MSG("open /var/log/logfile by read fail!");
		printf("-1");
		free(log);
		goto error;
	}
	
	memset(log, 0, LOG_MAX);
	char buffer[1024] = {0};
	while (fgets(buffer, sizeof(buffer), fp)) {
		if(strstr(buffer,"kernel"))
			continue;
		buffer[strlen(buffer)-1] = '\0';//去掉换行符\n，在界面会报错
		//DBG_MSG("+++zero_debug===buffer=[%s]",buffer);
		strcat(log,buffer);
		strcat(log,"&&&&");//把换行符换成&&&&
	}
	
	fclose(fp);	
	printf("%s", log);
	
	free(log);
error:
	if(pp)
		pclose(pp);
	
}


#if defined CONFIG_USER_STORAGE
int GetDiskDir(int show)
{
	FILE *fp_mount = fopen(MOUNT_INFO, "r");
	char part[30], path[50];
	//char dir_name[30];
	int dir_len = 0;
	int dir_count = 0;

	if (NULL == fp_mount) {
		DBG_MSG("opne %s fail!", MOUNT_INFO);
		goto error;
	}

	while(EOF != fscanf(fp_mount, "%s %s %*s %*s %*s %*s\n", part, path))
	{
		DIR *dp;
		struct dirent *dirp;
		struct stat statbuf;

		if (0 != strncmp(path, "/media/", 7))
		{
			continue;
		}
		if (NULL == (dp = opendir(path)))
		{
			DBG_MSG("open %s error\n", path);
			goto error;
		}
		chdir(path);
		while(NULL != (dirp = readdir(dp)))
		{
			lstat(dirp->d_name, &statbuf);
			if(S_ISDIR(statbuf.st_mode))
			{
				//if (0 == strncmp(dirp->d_name, ".", 1))
				if ((0 == strncmp(dirp->d_name, ".", 1)) ||
				    (strlen(dirp->d_name) <= 0))
					continue;
#if 0
				strcpy(dir_name, dirp->d_name);
				dir_len = strlen(dirp->d_name);
				if (dir_len <= 0)
					continue;
#endif
				if (show) {
					printf("<tr>");
					printf("<td><input type=\"radio\" name=\"dir_path\" value=\"%s/%s\"></td>", 
						path, dirp->d_name);
					printf("<td>%s/%s</td>", path, dirp->d_name);
					printf("<td>%s</td>", part);
					printf("</tr>");
				}
				dir_count++;
				/*
				if (dir_len < 30 && dir_len > 0)
				{
					if (show) {
						printf("<tr>");
						printf("<td>%s/%s</td>", path, dirp->d_name);
						printf("<td>%s</td>", part);
						printf("</tr>");
					} else {
						dir_count++;
					}
				}
				*/
			}
		}
		chdir("/");
		closedir(dp);
	}
error:
	if(fp_mount)
		fclose(fp_mount);

	return dir_count;
}

int GetDiskPartition(int show)
{
	FILE *fp = fopen(MOUNT_INFO, "r");
	char part[30], path[50];
	int part_count = 0;

	if (NULL == fp) {
		DBG_MSG("fopen %s fail!", MOUNT_INFO);
		goto error;
	}

	while(EOF != fscanf(fp, "%s %s %*s %*s %*s %*s\n", part, path))
	{
		// if (strncmp(path, "/var", 4) != 0)
		if (0 != strncmp(path, "/media/", 7))
			continue;
		if (show)
		{
			printf("<tr align=center>");
			printf("<td><input type=\"radio\" name=\"disk_part\" value=\"%s\"></td>",
					path);
			printf("<td>%s</td>", part);
			printf("<td>%s</td>", path);
			printf("</tr>");
		}
		part_count++;
	}
error:
	if (fp)
		fclose(fp);

	return part_count;
}

#if defined CONFIG_USER_STORAGE && defined CONFIG_USER_MINIDLNA
int GetMediaDir(int show)
{
	char path[50];
	int index, media_dir_count = 0;
	struct media_config cfg[4];

	memset(cfg, 0, sizeof(cfg));
	fetch_media_cfg(cfg, 0);
	for(index=0;index<4;index++)
	{
		if (0 != strlen(cfg[index].path))
		{
			if (show)
			{
				printf("<tr align=\"center\">");
				printf("<td><input type=\"radio\" name=\"media_dir\" value=\"%d\"></td>",
						index);
				printf("<td>%s</td>", cfg[index].path);
				printf("</tr>");
			}
			media_dir_count++;
		}
	}

	return media_dir_count;
}
#endif

void GetCounter(char *field)
{
	if (0 == strcmp(field, "n_AllDir")) {
		printf("%d", GetDiskDir(0));
		// fprintf(stderr,"AllDir: %s\n", count);
	} else if (0 == strcmp(field, "n_AllPart")) {
		printf("%d", GetDiskPartition(0));
		// fprintf(stderr,"AllPart: %s\n", count);
#if defined CONFIG_USER_STORAGE && defined CONFIG_USER_MINIDLNA
	} else if (0 == strcmp(field, "n_AllMediaDir")) {
		printf("%d", GetMediaDir(0));
		// fprintf(stderr,"AllPart: %s\n", count);
#endif
	}
}
#endif

int router_getCfgValue(const char *cmd, char *cfgValue, size_t len)
{
    FILE* fp; 
    if((fp = popen(cmd, "r")) == NULL) 
    {
        printf("nGetRouterCfg:popen failed.");
        return -1;
    }
	
	fgets(cfgValue, len, fp);
	
	pclose(fp);
	return 0;
}

void router_getOnlineDuration()
{
	char sUpTime[64] = {0};
	int pos = 0,  nOnlineTime = 0;
	router_getCfgValue("cat /proc/uptime",  sUpTime, sizeof(sUpTime) -1);
	while(pos < strlen(sUpTime)) {
		if (isspace(sUpTime[pos])) {
			sUpTime[pos] = '\0';
			break;
		}
		pos++;
	}
	
	nOnlineTime = atoi(sUpTime);

	printf("%d hours, %d mins, %d secs", 
		nOnlineTime/3600, 
		(nOnlineTime - (nOnlineTime/3600) *3600)/60,
		(nOnlineTime - (nOnlineTime/3600) *3600 - ((nOnlineTime - (nOnlineTime/3600) *3600)/60)*60));
	
	return;
}


void WebSysGet(char *argv[])
{
	if (!strcmp(argv[3], "wif2_live") && !strcmp(argv[1], "rtdev")) {
		printf("%d", get_if_live("rai0"));
	} else if (!strcmp(argv[3], "onePortOnly")) {
#if defined (CONFIG_ICPLUS_PHY)
		printf("true");
#else
		printf("false");
#endif
	} else if (!strcmp(argv[3], "wanIpAddr")) {
		printf("%s", get_ipaddr(get_wanif_name()));
	} else if (!strcmp(argv[3], "wanNetmask")) {
		printf("%s", get_netmask(get_wanif_name()));
	} else if (!strcmp(argv[3], "wanGateway")) {
		GetGateway();
	} else if (!strcmp(argv[3], "dns1")) {
		GetDns(1);
	} else if (!strcmp(argv[3], "dns2")) {
		GetDns(2);
	} else if (!strcmp(argv[3], "lanIpAddr")) {
		printf("%s", get_ipaddr(get_lanif_name()));
	} else if (!strcmp(argv[3], "lanNetmask")) {
		printf("%s", get_netmask(get_lanif_name()));
	} else if (!strcmp(argv[3], "dhcpClientList")) {
		GetDhcpClientList();
	} else if (!strcmp(argv[3], "routingtable")) {
		GetRoutingTable();
	} else if (!strcmp(argv[3], "lanMacAddr")) {
		nvram_init(RT2860_NVRAM);
		printf("%s", get_macaddr(get_lanif_name()));
		nvram_close(RT2860_NVRAM);
	} else if (!strcmp(argv[3], "wifiMacAddr")) {
		if (!strcmp(argv[1], WLAN2_CONF))
			printf("%s", get_macaddr("rai0"));
		else
			printf("%s", get_macaddr("ra0"));
	} else if (!strcmp(argv[3], "wanMacAddr")) {
		nvram_init(RT2860_NVRAM);
		if (!strncmp(get_wanif_name(), "ppp0", 4))
			printf("%s", get_macaddr("eth2.2"));
		else
			printf("%s", get_macaddr(get_wanif_name()));
		nvram_close(RT2860_NVRAM);
	} else if (!strcmp(argv[3], "currentTime")) {
		GetCurrentTime();
	} else if (!strcmp(argv[3], "HeMucurrentTime")) {
		GetHeMuCurrentTime();
	}
	else if (!strcmp(argv[3], "memTotal")) {
		GetMemTotal();
	} else if (!strcmp(argv[3], "memFree")) {
		GetMemFree();
	} else if (!strcmp(argv[3], "wanRxPkt")) {
		long long data = getIfStatistic(get_wanif_name(), RXPACKET);
		printf("%lld", data);
	} else if (!strcmp(argv[3], "wanRxByte")) {
		long long data = getIfStatistic(get_wanif_name(), RXBYTE);
		printf("%lld", data);
	} else if (!strcmp(argv[3], "wanTxPkt")) {
		long long data = getIfStatistic(get_wanif_name(), TXPACKET);
		printf("%lld", data);
	} else if (!strcmp(argv[3], "wanTxByte")) {
		long long data = getIfStatistic(get_wanif_name(), TXBYTE);
		printf("%lld", data);
	} else if (!strcmp(argv[3], "lanRxPkt")) {
		long long data = getIfStatistic(get_lanif_name(), RXPACKET);
		printf("%lld", data);
	} else if (!strcmp(argv[3], "lanRxByte")) {
		long long data = getIfStatistic(get_lanif_name(), RXBYTE);
		printf("%lld", data);
	} else if (!strcmp(argv[3], "lanTxPkt")) {
		long long data = getIfStatistic(get_lanif_name(), TXPACKET);
		printf("%lld", data);
	} else if (!strcmp(argv[3], "lanTxByte")) {
		long long data = getIfStatistic(get_lanif_name(), TXBYTE);
		printf("%lld", data);
	} else if (!strcmp(argv[3], "allStatistic")) {
		GetAllNICStatisticASP();
	} /*else if (!strcmp(argv[3], "portStatus")) {
		GetPortStatus();
	} */else if (!strcmp(argv[3], "sdkVersion")) {
		GetSdkVersion();
	} else if (!strcmp(argv[3], "buildTime")) {
		printf("%s", __DATE__);
	} else if (!strcmp(argv[3], "uptime")) {
		router_getOnlineDuration();
	}else if (!strcmp(argv[3], "Uptime")) {
		GetUptime();
	}else if (!strcmp(argv[3], "platform")) {
		GetPlatform();
	} else if (!strcmp(argv[3], "cmdShow")) {
		GetCmdShow();
	} else if (!strcmp(argv[3], "syslog")) {
		GetSysLog();
	} else if (!strcmp(argv[3], "syslog_Hemu")) {
		GetHeMuSysLog();
#if defined CONFIG_USER_STORAGE
	} else if (!strcmp(argv[3], "DiskDir")) {
		GetDiskDir(1);
	} else if (!strcmp(argv[3], "DiskPartition")) {
		GetDiskPartition(1);
#if defined CONFIG_USER_STORAGE && defined CONFIG_USER_MINIDLNA
	} else if (!strcmp(argv[3], "MediaDir")) {
		GetMediaDir(1);
#endif
	} else if (!strcmp(argv[3], "n_AllDir") || 
		   !strcmp(argv[3], "n_AllPart") || 
		   !strcmp(argv[3], "n_AllMediaDir")) {
		GetCounter(argv[3]);
#endif
	}else if (!strcmp(argv[3], "internetStatus")) {
		GetnetStatus();
	}else if (!strcmp(argv[3], "portStatus")) {
		GetportStatus();
	}else if (!strcmp(argv[3], "connectType")) {
		GetConnectType();
	}else if (!strcmp(argv[3], "isNeedWizard")) {
		isNeedWizard();
	}else if (!strcmp(argv[3], "wanConnectionMode")) {
		printf(nvram_get(RT2860_NVRAM, "wanConnectionMode"));
	}else if (!strcmp(argv[3], "wan_pppoe_user")) {
		printf(nvram_get(RT2860_NVRAM, "wan_pppoe_user"));
	}else if (!strcmp(argv[3], "wan_pppoe_pass")) {
		printf(nvram_get(RT2860_NVRAM, "wan_pppoe_pass"));
	}else if (!strcmp(argv[3], "serialNum")) {
		router_getDevSn();
	}

}

void GetIPPortRuleNum(void)
{
	char *rules;

	nvram_init(RT2860_NVRAM);
	rules = (char *) nvram_bufget(RT2860_NVRAM, "IPPortFilterRules");
	if(!rules || !strlen(rules) ) {
		printf("0");
		return;
	}
	printf("%d", get_nums(rules, ';'));
	nvram_close(RT2860_NVRAM);
}

void GetRulesPacketCount(void)
{
	FILE *pp;
	int i, step_in_chains=0;
	char buf[1024], *default_policy;
	int default_drop_flag;
	int index=0, pkt_count;
	int *result;

	// check if the default policy is "drop"
	nvram_init(RT2860_NVRAM);
	default_policy = (char *)nvram_bufget(RT2860_NVRAM, "DefaultFirewallPolicy");
	if(!default_policy)
		default_policy = "0";
	default_drop_flag = strtol(default_policy, NULL, 10);
	nvram_close(RT2860_NVRAM);

	result = (int *)malloc(sizeof(int) * 128);
	if (!result)
		return;

	pp = popen("iptables -t filter -L -v", "r");
	if (!pp) {
		free(result);
		return;
	}

	while (fgets(buf, 1024, pp) && index < 128) {
		if(step_in_chains) {
			if(buf[0] == '\n')
				break;
			if(buf[0] == ' ' && buf[1] == 'p' && buf[2] == 'k' && buf[3] == 't' )
				continue;
			// Skip the first one rule if default policy is drop.
			if (default_drop_flag) {
				default_drop_flag = 0;
				continue;
			}
			sscanf(buf, "%d ", &pkt_count);
			result[index++] = pkt_count;
		}

		if(strstr(buf, "Chain " IPPORT_FILTER_CHAIN))
			step_in_chains = 1;
	}
	pclose(pp);

	if(index > 0)
		printf("%d", result[0]);
	for(i=1; i<index; i++)
		printf(" %d", result[i]);

	free(result);
}

void GetIPPortFilterRule(void)
{
	int i;
	int sprf_int, sprt_int, proto;
	char mac_address[32];
	char sip_1[32], sip_2[32], sprf[8], sprt[8], comment[16], protocol[8], action[4];
	char dip_1[32], dip_2[32], dprf[8], dprt[8];
	int dprf_int, dprt_int;
	char rec[256];
	char *default_policy, *rules;

	nvram_init(RT2860_NVRAM);
	rules = (char *) nvram_bufget(RT2860_NVRAM, "IPPortFilterRules");
	default_policy = (char *) nvram_bufget(RT2860_NVRAM, "DefaultFirewallPolicy");
	// add the default policy to the end of FORWARD chain
	if(!rules)
		return;
	if (!default_policy)
		return;
	if (!strlen(default_policy))
		return;

	i=0;
	while (get_nth_value(i, rules, ';', rec, sizeof(rec)) != -1 && strlen(rec)) {
		DBG_MSG("i=%d, rec=%s, strlen(rec)=%d", i, rec, strlen(rec));
		// get ip 1
		if((get_nth_value(0, rec, ',', sip_1, sizeof(sip_1)) == -1)){
			i++;
			continue;
		}
		if(!is_ipnetmask_valid(sip_1)){
			i++;
			continue;
		}
		// translate "any/0" to "any" for readable reason
		if( !strcmp(sip_1, "any/0"))
			strcpy(sip_1, "-");

		// get ip 2
		// get ip address
		if((get_nth_value(1, rec, ',', sip_2, sizeof(sip_2)) == -1)){
			i++;
			continue;
		}
		// dont verify cause we dont have ip range support
		//if(!is_ip_valid(sip_2))
		//    continue;
		// get port range "from"
		if((get_nth_value(2, rec, ',', sprf, sizeof(sprf)) == -1)){
			i++;
			continue;
		}
		if( (sprf_int = strtol(sprf, NULL, 10)) > 65535){
			i++;
			continue;
		}

		// get port range "to"
		if((get_nth_value(3, rec, ',', sprt, sizeof(sprt)) == -1)){
			i++;
			continue;
		}
		if( (sprt_int = strtol(sprt, NULL, 10)) > 65535){
			i++;
			continue;
		}

		// get ip 1
		if((get_nth_value(4, rec, ',', dip_1, sizeof(dip_1)) == -1)){
			i++;
			continue;
		}
		if(!is_ipnetmask_valid(dip_1)){
			i++;
			continue;
		}
		// translate "any/0" to "any" for readable reason
		if( !strcmp(dip_1, "any/0"))
			strcpy(dip_1, "-");

		// get ip 2
		if((get_nth_value(5, rec, ',', dip_2, sizeof(dip_2)) == -1)){
			i++;
			continue;
		}
		// dont verify cause we dont have ip range support
		//if(!is_ip_valid(dip_2))
		//    continue;
		// get protocol
		if((get_nth_value(8, rec, ',', protocol, sizeof(protocol)) == -1)){
			i++;
			continue;
		}
		proto = strtol(protocol, NULL, 10);
		switch(proto){
		case PROTO_TCP:
		case PROTO_UDP:
		case PROTO_NONE:
		case PROTO_ICMP:
			break;
		default:
			continue;
		}

		// get port range "from"
		if((get_nth_value(6, rec, ',', dprf, sizeof(dprf)) == -1)){
			i++;
			continue;
		}
		if( (dprf_int = strtol(dprf, NULL, 10)) > 65535){
			i++;
			continue;
		}

		// get port range "to"
		if((get_nth_value(7, rec, ',', dprt, sizeof(dprt)) == -1)){
			i++;
			continue;
		}
		if( (dprt_int = strtol(dprt, NULL, 10)) > 65535){
			i++;
			continue;
		}

		// get action
		if((get_nth_value(9, rec, ',', action, sizeof(action)) == -1)){
			i++;
			continue;
		}

		// get comment
		if((get_nth_value(10, rec, ',', comment, sizeof(comment)) == -1)){
			i++;
			continue;
		}

		// get mac address
		if((get_nth_value(11, rec, ',', mac_address, sizeof(mac_address)) == -1)){
			i++;
			continue;
		}
		if(!strlen(mac_address))
			strcpy(mac_address, "-");

		printf("<tr>");
		// output No.
		printf("<td> %d&nbsp; <input type=\"checkbox\" name=\"delRule%d\"> </td>", i+1, i );
		// output Mac address
		printf("<td align=center> %s </td>", mac_address);

		// output DIP
		printf("<td align=center> %s </td>", dip_1);
		// we dont support ip range
		// printf("<td align=center> %s-%s </td>", ip_1, ip_2);
		// output SIP
		printf("<td align=center> %s </td>", sip_1);
		// we dont support ip range
		// printf("<td align=center> %s-%s </td>", ip_1, ip_2);
		// output Protocol
		switch(proto){
		case PROTO_TCP:
			printf("<td align=center> TCP </td>");
			break;
		case PROTO_UDP:
			printf("<td align=center> UDP </td>");
			break;
		case PROTO_ICMP:
			printf("<td align=center> ICMP </td>");
			break;
		case PROTO_NONE:
			printf("<td align=center> - </td>");
			break;
		}

		// output dest Port Range
		if(dprt_int)
			printf("<td align=center> %d - %d </td>", dprf_int, dprt_int);
		else {
			// we re-descript the port number here because
			// "any" word is more meanful than "0"
			if (!dprf_int)
				printf("<td align=center> - </td>", dprf_int);
			else
				printf("<td align=center> %d </td>", dprf_int);
		}

		// output Source Port Range
		if(sprt_int)
			printf("<td align=center> %d - %d </td>", sprf_int, sprt_int);
		else{
			// we re-descript the port number here because
			// "any" word is more meanful than "0"
			if(!sprf_int){
				printf("<td align=center> - </td>", sprf_int);
			}else{
				printf("<td align=center> %d </td>", sprf_int);
			}
		}


		// output action
		switch(strtol(action, NULL, 10)){
		case ACTION_DROP:
			printf("<td align=center id=portFilterActionDrop%d> Drop </td>", i);
			break;
		case ACTION_ACCEPT:
			printf("<td align=center id=portFilterActionAccept%d> Accept </td>", i);
			break;
		}

		// output Comment
		if(strlen(comment))
			printf("<td align=center> %s</td>", comment);
		else
			printf("<td align=center> &nbsp; </td>");

		// output the id of "packet count"
		printf("<td align=center id=pktCnt%d>-</td>", i);

		printf("</tr>\n");

		i++;
	}

	switch (strtol(default_policy, NULL, 10)) {
	case 0:
		printf("<tr><td align=center colspan=9 id=portCurrentFilterDefaultAccept> Others would be accepted.</td><td align=center id=pktCnt%d>-</td></tr>", i);
		break;
	case 1:
		printf("<tr><td align=center colspan=9 id=portCurrentFilterDefaultDrop> Others would be dropped.</td><td align=center id=pktCnt%d>-</td></tr>", i);
		break;
	}
	nvram_close(RT2860_NVRAM);
}
void GetPortTriggerRuleNum(void)
{
  char *rules; 
  nvram_init(RT2860_NVRAM);
  rules = (char *)nvram_bufget(RT2860_NVRAM, "PortTriggerRules");
	if(!rules || !strlen(rules) ){
		printf("0");
		return;
	}
	printf("%d", get_nums(rules, ';'));
	nvram_close(RT2860_NVRAM);
}
void GetPortForwardRuleNum(void)
{
	char *rules;

	nvram_init(RT2860_NVRAM);
	rules = (char *) nvram_bufget(RT2860_NVRAM, "PortForwardRules");
	if(!rules || !strlen(rules) ){
		printf("0");
		return;
	}
	printf("%d", get_nums(rules, ';'));
	nvram_close(RT2860_NVRAM);
}

void GetSinglePortForwardRuleNum(void)
{
	char *rules;

	nvram_init(RT2860_NVRAM);
	rules = (char *) nvram_bufget(RT2860_NVRAM, "SinglePortForwardRules");
	if(!rules || !strlen(rules) ){
		printf("0");
		return;
	}
	printf("%d", get_nums(rules, ';'));
	nvram_close(RT2860_NVRAM);
}

void GetPortForwardRule(void)
{
	int i=0;
	int prf_int, prt_int, proto;
	char ip_address[32], prf[8], prt[8], comment[16], protocol[8];
	char rec[128];
	char *rules;

	nvram_init(RT2860_NVRAM);
	rules = (char *) nvram_bufget(RT2860_NVRAM, "PortForwardRules");
	if (!rules)
		return;
	if (!strlen(rules))
		return;

	/* format is :
	 *          * [ip],[port_from],[port_to],[protocol],[comment],;
	 *                   */
	while (get_nth_value(i++, rules, ';', rec, sizeof(rec)) != -1 ) {
		//printf("i=%d : \n",i);
		// get ip address
		if((get_nth_value(0, rec, ',', ip_address, sizeof(ip_address)) == -1)){
			//printf("ip fail!!\n");
			continue;
		}
		if(!is_ip_valid(ip_address)){
			continue;
		}

		// get port range "from"
		if((get_nth_value(1, rec, ',', prf, sizeof(prf)) == -1)){
			//printf("prf fail!!\n");
			continue;
		}
		if( (prf_int = strtol(prf, NULL, 10)) == 0 || prf_int > 65535){
			continue;
		}

		// get port range "to"
		if((get_nth_value(2, rec, ',', prt, sizeof(prt)) == -1)){
			//printf("prt fail!!\n");
			continue;
		}

		if( (prt_int = strtol(prt, NULL, 10)) > 65535){
			continue;
		}
		// get protocol
		if((get_nth_value(3, rec, ',', protocol, sizeof(protocol)) == -1)){
			//printf("proto fail!!\n");
			continue;
		}
		proto = strtol(protocol, NULL, 10);
		switch(proto){
		case PROTO_TCP:
		case PROTO_UDP:
		case PROTO_TCP_UDP:
			break;
		default:
			continue;
		}

		if((get_nth_value(4, rec, ',', comment, sizeof(comment)) == -1)){
			continue;
		}
		printf("<tr>\n");
		// output No.
		printf("<td> %d&nbsp; <input type=\"checkbox\" name=\"delRule%d\"> </td>", i, i-1 );

		// output IP address
		printf("<td align=center> %s </td>", ip_address);

		// output Port Range
		if(prt_int)
			printf("<td align=center> %d - %d </td>", prf_int, prt_int);
		else
			printf("<td align=center> %d </td>", prf_int);

		// output Protocol
		switch(proto){
		case PROTO_TCP:
			printf("<td align=center> TCP </td>");
			break;
		case PROTO_UDP:
			printf("<td align=center> UDP </td>");
			break;
		case PROTO_TCP_UDP:
			printf("<td align=center> TCP + UDP </td>");
			break;
		}

		// output Comment
		if(strlen(comment))
			printf("<td align=center> %s</td>", comment);
		else
			printf("<td align=center> &nbsp; </td>");
		printf("</tr>\n");
	}
	nvram_close(RT2860_NVRAM);
}
#if defined CONFIG_MTK_VOIP
void GetVoipDialRule(void)
{
	int i=0;
	char rec[256];
	int shortNumber_int, realNumber_int, dialActive_int;
	char shortNumber[10], realNumber[10], dialActive[6], note[16];
		nvram_init(VOIP_NVRAM);
	char *rules = (char *)nvram_bufget(VOIP_NVRAM, "VoipDialRules");
	if(!rules)
		return;
	if(!strlen(rules))
		return;
		
	while( (get_nth_value(i++, rules, ';', rec, sizeof(rec)) != -1) ){
		//get active
		if((get_nth_value(0, rec, ',', dialActive, sizeof(dialActive)) == -1)){
			continue;
		}
	  dialActive_int= atoi(dialActive);
		// get shortnumber
		if((get_nth_value(1, rec, ',', shortNumber, sizeof(shortNumber)) == -1)){
			continue;
		}
		shortNumber_int=atoi(shortNumber);
    if((get_nth_value(2, rec, ',', realNumber, sizeof(realNumber)) == -1))
			continue;
		realNumber_int=atoi(realNumber);	
		if((get_nth_value(3, rec, ',', note, sizeof(note)) == -1)){
			continue;
		}
		//printf(wp, T("<tr>\n"));
		printf("<tr> \n");
		// output No.
		//printf(wp, T("<td> %d&nbsp; <input type=\"checkbox\" name=\"delRule%d\"> </td>"), i, i-1 );
    printf("<td> %d&nbsp; <input type=\"checkbox\" name=\"delRule%d\"> </td>", i, i-1 );
	
	  if(dialActive_int == 1)
	    printf("<td align=center> Y </td>");
	  else
	  	printf("<td align=center> N </td>");

			printf("<td align=center> %d </td>", shortNumber_int);

      printf("<td align=center> %d </td>", realNumber_int);
		// output Comment
		if(strlen(note)){
			//printf(wp, T("<td align=center> %s</td>"), comment);
			printf("<td align=center> %s</td>", note);
		}	
		else{
			//printf(wp, T("<td align=center> &nbsp; </td>"));
   		//printf(wp, T("</tr>\n"));
   		printf("<td align=center> &nbsp; </td>");
   		printf("</tr>\n");
   	}
  }	
  	nvram_close(VOIP_NVRAM);	
}
void GetVoipDialRule2(void)
{
	int i=0;
	char rec[256];
	int shortNumber_int, realNumber_int, dialActive_int;
	char shortNumber[10], realNumber[10], dialActive[6], note[16];
			nvram_init(VOIP_NVRAM);
	char *rules = (char *)nvram_bufget(VOIP_NVRAM, "VoipDialRules2");
	if(!rules)
		return;
	if(!strlen(rules))
		return;
		
	while( (get_nth_value(i++, rules, ';', rec, sizeof(rec)) != -1) ){
		//get active
		if((get_nth_value(0, rec, ',', dialActive, sizeof(dialActive)) == -1)){
			continue;
		}
	  dialActive_int= atoi(dialActive);
		// get shortnumber
		if((get_nth_value(1, rec, ',', shortNumber, sizeof(shortNumber)) == -1)){
			continue;
		}
		shortNumber_int=atoi(shortNumber);
    if((get_nth_value(2, rec, ',', realNumber, sizeof(realNumber)) == -1))
			continue;
		realNumber_int=atoi(realNumber);	
		if((get_nth_value(3, rec, ',', note, sizeof(note)) == -1)){
			continue;
		}
		//printf(wp, T("<tr>\n"));
		printf("<tr> \n");
		// output No.
		//printf(wp, T("<td> %d&nbsp; <input type=\"checkbox\" name=\"delRule%d\"> </td>"), i, i-1 );
    printf("<td> %d&nbsp; <input type=\"checkbox\" name=\"delRule%d\"> </td>", i, i-1 );
	
	  if(dialActive_int == 1)
	    printf("<td align=center> Y </td>");
	  else
	  	printf("<td align=center> N </td>");

			printf("<td align=center> %d </td>", shortNumber_int);

      printf("<td align=center> %d </td>", realNumber_int);
		// output Comment
		if(strlen(note)){
			//printf(wp, T("<td align=center> %s</td>"), comment);
			printf("<td align=center> %s</td>", note);
		}	
		else{
			//printf(wp, T("<td align=center> &nbsp; </td>"));
   		//printf(wp, T("</tr>\n"));
   		printf("<td align=center> &nbsp; </td>");
   		printf("</tr>\n");
   	}
  }		
  nvram_close(VOIP_NVRAM);
}
#endif
void GetPortTriggerRule(void)
{
	int i=0;
	char rec[256];
	int triggerPort_int, incomingPort_int, triggerProto, incomingProto;
	char triggerPort[8], incomingPort[8], triggerProtocol[8], incomingProtocol[8], comment[16];
	char *rules = (char *)nvram_bufget(RT2860_NVRAM, "PortTriggerRules");
	
	if(!rules)
		return;
	if(!strlen(rules))
		return;
		
	while( (get_nth_value(i++, rules, ';', rec, sizeof(rec)) != -1) ){
		//get trigger protocol
		if((get_nth_value(0, rec, ',', triggerProtocol, sizeof(triggerProtocol)) == -1))
			continue;
		triggerProto = atoi(triggerProtocol);
		// get trigger port
		if((get_nth_value(1, rec, ',', triggerPort, sizeof(triggerPort)) == -1)){
			printf("triggerPort = %s\n", triggerPort);	
			continue;
		}
		if( (triggerPort_int = atoi(triggerPort)) == 0 || triggerPort_int > 65535)
			continue;
        // get incoming portoocol
    if((get_nth_value(2, rec, ',', incomingProtocol, sizeof(incomingProtocol)) == -1))
			continue;
		incomingProto = atoi(incomingProtocol);
		// get incoming port
    if((get_nth_value(3, rec, ',', incomingPort, sizeof(incomingPort)) == -1)){
			printf("incomingPort = %s\n", incomingPort);	
			continue;
		}
		if( (incomingPort_int = atoi(incomingPort)) == 0 || incomingPort_int > 65535)
			continue;
		
		if((get_nth_value(4, rec, ',', comment, sizeof(comment)) == -1)){
			continue;
		}
		//printf(wp, T("<tr>\n"));
		printf("<tr> \n");
		// output No.
		//printf(wp, T("<td> %d&nbsp; <input type=\"checkbox\" name=\"delRule%d\"> </td>"), i, i-1 );
    printf("<td> %d&nbsp; <input type=\"checkbox\" name=\"delRule%d\"> </td>", i, i-1 );
		// output Trigger Protocol
     switch(triggerProto){
        case PROTO_TCP:
					//printf(wp, T("<td align=center> TCP </td>"));
					printf("<td align=center> TCP </td>");
					break;
        case PROTO_UDP:
					//printf(wp, T("<td align=center> UDP </td>"));
					printf("<td align=center> UDP </td>");
				break;
		}

		// output trigger Port 
			//printf(wp, T("<td align=center> %d </td>"), triggerPort_int);
			printf("<td align=center> %d </td>", triggerPort_int);
		// output Incoming Protocol
        switch(incomingProto){
            case PROTO_TCP:
							//printf(wp, T("<td align=center> TCP </td>"));
							printf("<td align=center> TCP </td>");
							break;
            case PROTO_UDP:
							//printf(wp, T("<td align=center> UDP </td>"));
							printf("<td align=center> UDP </td>");
							break;
				}

		// output incoming Port 
			//printf(wp, T("<td align=center> %d </td>"), incomingPort_int);
    printf("<td align=center> %d </td>", incomingPort_int);
		// output Comment
		if(strlen(comment)){
			//printf(wp, T("<td align=center> %s</td>"), comment);
			printf("<td align=center> %s</td>", comment);
		}	
		else{
			//printf(wp, T("<td align=center> &nbsp; </td>"));
   		//printf(wp, T("</tr>\n"));
   		printf("<td align=center> &nbsp; </td>");
   		printf("</tr>\n");
   	}
  }	
}
void GetSinglePortForwardRule(void)
{
	int i=0;
	int publicPort_int, privatePort_int, proto;
	char ip_address[32], publicPort[8], privatePort[8], comment[16], protocol[8];
	char rec[128];
	char *rules;

	nvram_init(RT2860_NVRAM);
	rules = (char *) nvram_bufget(RT2860_NVRAM, "SinglePortForwardRules");
	if (!rules)
		return;
	if (!strlen(rules))
		return;

	/* format is :
	 *          * [ip],[port_public],[port_private],[protocol],[comment],;
	 *                   */
	while(get_nth_value(i++, rules, ';', rec, sizeof(rec)) != -1 ){
		// get ip address
		if((get_nth_value(0, rec, ',', ip_address, sizeof(ip_address)) == -1)){
			continue;
		}
		if(!is_ip_valid(ip_address)){
			continue;
		}

		// get public port
		if((get_nth_value(1, rec, ',', publicPort, sizeof(publicPort)) == -1)){
			continue;
		}
		if( (publicPort_int = strtol(publicPort, NULL, 10)) == 0 || publicPort_int > 65535){
			continue;
		}

		// get private port
		if((get_nth_value(2, rec, ',', privatePort, sizeof(privatePort)) == -1)){
			continue;
		}
		if( (privatePort_int = strtol(privatePort, NULL, 10)) == 0 || privatePort_int > 65535){
			continue;
		}

		// get protocol
		if((get_nth_value(3, rec, ',', protocol, sizeof(protocol)) == -1)){
			continue;
		}
		proto = strtol(protocol, NULL, 10);
		switch(proto){
		case PROTO_TCP:
		case PROTO_UDP:
		case PROTO_TCP_UDP:
			break;
		default:
			continue;
		}

		if((get_nth_value(4, rec, ',', comment, sizeof(comment)) == -1)){
			continue;
		}

		printf("<tr>\n");
		// output No.
		printf("<td> %d&nbsp; <input type=\"checkbox\" name=\"delRule%d\"> </td>", i, i-1 );

		// output IP address
		printf("<td align=center> %s </td>", ip_address);

		// output Public port
		printf("<td align=center> %d </td>", publicPort_int);

		// output Private port
		printf("<td align=center> %d </td>", privatePort_int);

		// output Protocol
		switch(proto){
		case PROTO_TCP:
			printf("<td align=center> TCP </td>");
			break;
		case PROTO_UDP:
			printf("<td align=center> UDP </td>");
			break;
		case PROTO_TCP_UDP:
			printf("<td align=center> TCP + UDP </td>");
			break;
		}

		// output Comment
		if(strlen(comment))
			printf("<td align=center> %s</td>", comment);
		else
			printf("<td align=center> &nbsp; </td>");
		printf("</tr>\n");
	}
	nvram_close(RT2860_NVRAM);
}
char *getNameIntroFromPat(char *filename)
{
	static char result[512];
	char buf[512], *begin, *end, *desh;
	char path_filename[512];
	char *rc;
	FILE *fp;

	sprintf(path_filename, "%s/%s", "/etc_ro/l7-protocols", filename);
	if(! (fp = fopen(path_filename, "r")))
		return NULL;
	result[0] = '\0';
	rc = fgets(buf, sizeof(buf), fp);
	if(rc){
		// find name
		begin = buf + 2;
		if(! ( desh = strchr(buf, '-'))){
			printf("warning: can't find %s name.\n", filename);
			fclose(fp);
			return "N/A#N/A";
		}
		end = desh;
		if(*(end-1) == ' ')
			end--;
		*end = '\0';
		strncat(result, begin, sizeof(result));
		strncat(result, "#", sizeof(result));

		// find intro
		if(!(end = strchr(desh+1, '\n'))){
			printf("warning: can't find %s intro.\n", filename);
			fclose(fp);
			return "N/A#N/A";
		}
		*end = '\0';
		strncat(result, desh + 2 , sizeof(result));
	}else{
		printf("warning: can't read %s intro.\n", filename);
		fclose(fp);
		return "N/A#N/A";
	}

	fclose(fp);
	return result;	
}
void GetLayer7FilterName(void)
{
	char *delim;
	struct dirent *dir;
	DIR *d;
	char *intro;

	l7name[0] = '\0';
	if(!(d = opendir("/etc_ro/l7-protocols")))
		return;
	
	while((dir = readdir(d))){
		if(dir->d_name[0] == '.')
			continue;
		if(!(delim = strstr(dir->d_name, ".pat")) )
			continue;
		
		intro = getNameIntroFromPat(dir->d_name);
		*delim = '\0';
		if(l7name[0] == '\0'){
			strncat(l7name, dir->d_name, sizeof(l7name));
			strncat(l7name, "#", sizeof(l7name));
			strncat(l7name, intro, sizeof(l7name));
		}else{
			strncat(l7name, ";", sizeof(l7name));
			strncat(l7name, dir->d_name, sizeof(l7name));
			strncat(l7name, "#", sizeof(l7name));
			strncat(l7name, intro, sizeof(l7name));
		}
	}
	closedir(d);
	//websLongWrite(wp, l7name);
	printf("%s",l7name);
}
#if defined CONFIG_MTK_VOIP
void GetStunStatus(void){
		char *value;
		nvram_init(VOIP_NVRAM);
	
		value = nvram_bufget(VOIP_NVRAM, "SC_ACCT_1_NAT_SRV_ADDR");

		if(strcmp(value, "")!=0){
			printf("Enable");
		}else{
	    printf("Disable");
		}
	nvram_close(VOIP_NVRAM);
	
}
void GetStunStatus2(void){
		char *value;
		nvram_init(VOIP_NVRAM);
	
		value = nvram_bufget(VOIP_NVRAM, "SC_ACCT_2_NAT_SRV_ADDR");

		if(strcmp(value, "")!=0){
			printf("Enable");
		}else{
	    printf("Disable");
		}
	nvram_close(VOIP_NVRAM);
	
}

void WebVoipGet(char *argv[])
{
	if (!strcmp(argv[3], "voipDialRule")) {
		GetVoipDialRule();
	}else if(!strcmp(argv[3], "voipDialRule2")) {
		GetVoipDialRule2();
  }else if(!strcmp(argv[3], "stun_status")) {
		GetStunStatus();
  }else if(!strcmp(argv[3], "stun_status2")) {
		GetStunStatus2();
  }
	
}
void getCertInfo(char *name)
{

	#if (1)
	char *buf;
	char filename[256];
	int flag=0;
	unsigned is_load = 0;
	unsigned size;
	X509 *pCert=NULL;
	memset(filename, 0, 256);
	sprintf(filename,"/etc/%s.pem", name);
	FILE *fp=fopen(filename,"r");

  nvram_init(VOIP_NVRAM);
  size = 256;
	buf = malloc(size);

	/**
	 * 20100127 Alex Huang 
	 * Read Cert from MOM
	 */
/*
	if (!fp) {
		unsigned type = 0xFFFFFFFF, *is_load;
		unsigned size;
		char *buf2 = NULL;
		//int ret = -1;

		if (strcmp("wmx_root_ca", argv[0]) == 0) 
			type = WMXAPI_SERVER_CERT;
		else if (strcmp("wmx_client_ca", argv[0]) == 0)
			type = WMXAPI_DEVICE_CERT;

		if (type != 0xFFFFFFFF) {
			size = 256;
			ret = wmxapi_get_cert(type, buf, &size);
			if (ret != 0 && size != 0 && size < 128*1024) {
				buf2 = malloc(size);
				if (buf2) 
					ret = wmxapi_get_cert(type, buf2, &size);
			}
		}

		if (ret == 0) {
			if (size != 0) {
				int i;
				sprintf(filename,"/tmp/%s.pem", name);
				fp = fopen(filename, "w");
				fwrite(buf, size, 1, fp);
				fclose(fp);
				fp = fopen(filename, "r");
			}
			is_load = 1;
		}
	}
*/
	if(!fp)
	{
		sprintf(buf,"No certificate file found");
		printf("%s", buf);
		return;
	}

	pCert=PEM_read_X509(fp,NULL,0,NULL);
	if(pCert != NULL)  //certificate is PEM format
		flag = 1;
	else
	{
		fseek(fp, 0, SEEK_SET);
		pCert=d2i_X509_fp(fp,NULL);
		if(pCert != NULL)  //certificate is DER format
			flag = 2;
	}
	
	if(flag > 0)
	{
		while(pCert != NULL)
		{
			memset(buf,0,256);
			X509_NAME_oneline(X509_get_subject_name(pCert), buf, 256);
			//web_putsEscape(conn_fp, buf, "'\\");
			printf("%s", buf);
			pCert=NULL;
			if(flag == 1)
				pCert=PEM_read_X509(fp,NULL,0,NULL);
			else if(flag == 2)
				pCert=d2i_X509_fp(fp,NULL);
			if(pCert != NULL)
				printf("\\n\\n");
		}
	}
	else
	{
		sprintf(buf,"Unknown certificate file format");
		printf("%s", buf);
	}
 
	fclose(fp);
	if (is_load) 
		unlink(filename);
	
	nvram_commit(VOIP_NVRAM);
	nvram_close(VOIP_NVRAM);
	#endif
}


void WebCertInfoGet(char *argv[])
{
	if (!strcmp(argv[3], "cwmp_cacert")) {
		getCertInfo("cwmp_cacert");
	}else if(!strcmp(argv[3], "cwmp_client")) {
		getCertInfo("cwmp_client");
  }

}
#endif
void WebFirewallGet(char *argv[])
{
	if (!strcmp(argv[3], "ipPortRuleNum")) {
		GetIPPortRuleNum();
	} else if (!strcmp(argv[3], "rulePktCount")) {
		GetRulesPacketCount();
	} else if (!strcmp(argv[3], "ipPortFilterRule")) {
		GetIPPortFilterRule();
	} else if (!strcmp(argv[3], "portForwardRuleNum")) {
		GetPortForwardRuleNum();
	} else if (!strcmp(argv[3], "singlePortForwardRuleNum")) {
		GetSinglePortForwardRuleNum();
	} else if (!strcmp(argv[3], "portTriggerRuleNum")) {
		GetPortTriggerRuleNum();
	} else if (!strcmp(argv[3], "portTriggerRule")) {
		GetPortTriggerRule();			
	} else if (!strcmp(argv[3], "portForwardRule")) {
		GetPortForwardRule();
	} else if (!strcmp(argv[3], "singlePortForwardRule")) {
		GetSinglePortForwardRule();
	} else if (!strcmp(argv[3], "getLayer7")) {
		GetLayer7FilterName();
	} 
}

void do_auto_wan_detect(void)
{
#if	defined CONFIG_RALINK_MT7620 || CONFIG_RALINK_MT7628
	const char *vlanEnble, *vlanID;
	vlanEnble = nvram_bufget(RT2860_NVRAM, "VLANEnable");
	vlanID = nvram_bufget(RT2860_NVRAM, "VLANID");
	if(0==strncmp(vlanEnble,"1",2))
	{
		char if_name_buff[64] = {0};
		sprintf(if_name_buff,"auto_wan_pt 1 eth2.2.%s",vlanID);
		do_system(if_name_buff);
	}
	else
	{
		do_system("auto_wan_pt 1 eth2.2");
	}
#endif

#if	defined (CONFIG_RALNK_MT7621)
	do_system("auto_wan_pt 1 eth3");
#endif
	printf("0");
}
void usage(char *aout)
{
	//set_usage("STF");
	get_usage(aout);

	return;
}

//add by luozf 2017.11.24
#if defined CONFIG_CUSTOM_QOS
static void get_ipaddr_by_mac(char *argv[])
{
	if (argv[3] == NULL) 
	{
		printf("%s","Fail");
		return ;
	}
	char macAddr[64] = {0};
	char ipaddress[64] = {0};

	strcpy(macAddr,argv[3]);
	if(0 == strlen(macAddr))
	{
		printf("%s","Fail");
		return;
	}
	
	FILE *fp;
	struct dhcpOfferedAddr {
		unsigned char hostname[16];
		unsigned char mac[16];
		unsigned long ip;
		unsigned long expires;
	} lease;
	struct in_addr addr;
	char macTemp[64] = {0};
	char buffer[1024] = {0};
	
	if( NULL == macAddr)
	{	
		printf("%s","Fail");
		return ;
	}

	do_system("killall -q -USR1 udhcpd");
	fp = fopen("/var/udhcpd.leases", "r");
	if (NULL == fp)
	{
		printf("%s","Fail");
		return;
	}
	while (fread(&lease, 1, sizeof(lease), fp) == sizeof(lease)) 
	{
		//根据mac获取ip
		//1. macAddr	
		sprintf(macTemp, "%02X:%02X:%02X:%02X:%02X:%02X", 
				(unsigned char)lease.mac[0],(unsigned char)lease.mac[1],
				(unsigned char)lease.mac[2],(unsigned char)lease.mac[3],
				(unsigned char)lease.mac[4],(unsigned char)lease.mac[5]);

	//	DBG_MSG("+++++Zero_debug++++[%s %d]======macTemp=%s,macAddr=%s",__func__,__LINE__,macTemp,macAddr);

		if(strcasecmp(macTemp,macAddr) == 0)
		{
			//ipAddr
			addr.s_addr = lease.ip;	
			strcpy(ipaddress, inet_ntoa(addr));
	//		DBG_MSG("+++++Zero_debug++++[%s %d]======ipaddress=%s",__func__,__LINE__,ipaddress);
			//获取ip之后返回
			fclose(fp);
			printf("%s",ipaddress);
			return ;
		}	
	}//end of while
	fclose(fp);

//	DBG_MSG("+++++Zero_debug++++[%s %d]======get /proc/net/arp",__func__,__LINE__);

	//程序执行到这里，说明在dhcp没有查询到，去读取arp表
	/*IP address       HW type     Flags       HW address            Mask     Device*/
	fp = fopen("/proc/net/arp", "r");
	if (NULL == fp)
	{
		printf("%s","Fail");
		return;
	}
	char arp_ip[32], arp_flag[8], arp_macaddr[32];
	while(fgets(buffer, sizeof(buffer), fp))
	{
//		DBG_MSG("+++++Zero_debug++++[%s %d]======buffer=%s",__func__,__LINE__,buffer);
		memset(arp_ip,0,sizeof(arp_ip));
		memset(arp_flag,0,sizeof(arp_flag));
		memset(arp_macaddr,0,sizeof(arp_macaddr));
		
		sscanf(buffer, "%s %*s %s %s %*s %*s", arp_ip, arp_flag, arp_macaddr);
	//	DBG_MSG("+++++Zero_debug++++[%s %d]======arp_ip=%s,arp_flag=%s,arp_macaddr=%s",__func__,__LINE__,arp_ip,arp_flag,arp_macaddr);
		if(!strcasecmp(arp_flag, "0x2"))
		{
			if(strcasecmp(arp_macaddr,macAddr) == 0)
			{
				//ipAddr
				strcpy(ipaddress, arp_ip);
				//获取ip之后返回
			//	DBG_MSG("+++++Zero_debug++++[%s %d]======ipaddress=%s",__func__,__LINE__,ipaddress);
			//获取ip之后返回
				fclose(fp);
				printf("%s",ipaddress);
				return;
			}				
		}//end of if(!strcasecmp(arp_flag, "0x2"))
	}//end of while
	fclose(fp);

	//返回失败
	printf("%s","Fail");
	return ;
}
#endif
//end of by add

int main(int argc, char *argv[])
{
	int cmd;

	if (argc < 4) {
		get_usage(argv[0]);
		return;
	}

	if (!strncmp(argv[2], "nvram", 6))
		cmd = CMD_NVRAM_GET;
	else if (!strncmp(argv[2], "build", 6))
		cmd = CMD_BUILD_GET;
	else if (!strncmp(argv[2], "wifi", 5))
		cmd = CMD_WIFI_GET;
	else if (!strncmp(argv[2], "sys", 4))
		cmd = CMD_SYS_GET;
	else if (!strncmp(argv[2], "firewall", 5))
		cmd = CMD_FIREWALL_GET;
	else if (!strncmp(argv[2], "test", 6))
		cmd = CMD_TEST_GET;
#if defined CONFIG_MTK_VOIP
	else if (!strncmp(argv[2], "voip", 5))
		cmd = CMD_VOIP_GET;
	else if (!strncmp(argv[2], "tr069", 5))
	  cmd = CMD_TR069_GET;
#endif	  
#if defined CONFIG_RALINKAPP_AUTO_WAN
	else if (!strncmp(argv[2], "auto_wan", 5))
		cmd = CMD_AUTO_WAN_GET;	
#endif
#if defined CONFIG_CUSTOM_QOS
	else if (!strncmp(argv[2], "qos", 5))
			cmd = CMD_QOS_GET; 
#endif

	else
		cmd = 0;

	switch(cmd) {
	case CMD_NVRAM_GET:
		WebNvramGet(argv);
		break;
	case CMD_BUILD_GET:
		WebBuildGet(argv);
		break;
	case CMD_WIFI_GET:
		WebWifiGet(argv);
		break;
	case CMD_SYS_GET:
		WebSysGet(argv);
		break;
	case CMD_FIREWALL_GET:
		WebFirewallGet(argv);
		break;
#if defined CONFIG_MTK_VOIP
	case CMD_VOIP_GET:
		WebVoipGet(argv);
		break;
	case CMD_TR069_GET:
		WebCertInfoGet(argv);
		break;
#endif
#if defined CONFIG_RALINKAPP_AUTO_WAN
	case CMD_AUTO_WAN_GET:
		do_auto_wan_detect();
		break;
#endif
#if defined CONFIG_CUSTOM_QOS
	case CMD_QOS_GET:
		get_ipaddr_by_mac(argv);
		break;
#endif
	case CMD_TEST_GET:
		printf("SSID1\tSSID2\tSSID3\t\tSSID4\tSSID5\tSSID6\t\tSSID7\tSSID8\tSSID9");
		break;
	}
}

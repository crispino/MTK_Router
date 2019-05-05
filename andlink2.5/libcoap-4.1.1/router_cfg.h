#ifndef ROUTER_CFG_H
#define ROUTER_CFG_H

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/sysinfo.h>
#include <linux/wireless.h>
#include <ctype.h>
#include <errno.h>
#include "nvram.h"
#include "queue.h"
#include "oid.h"

#define __DEBUG
#ifdef __DEBUG
	#define DEBUG(format, ...)	printf("__LINE: %d__FUNC :%s__"format"\n", __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
	#define DEBUG(format, ...)
#endif

#define CLOUD_GATEWAY	1
#define HOME_GATEWAY 	2
#define SERVER_ADDR_LENGTH	64


#define ANDLINK_LOGFILE	"/dev/console"
#define IF_NAMESIZE     16
#define ROUTER_GET_WIRELESS_CLIENTS	"web 2860 wifi getWireless"
#define ROUTER_GET_S_VERSION	"web 2860 config swVersion"
#define ROUTER_GET_H_VERSION	"web 2860 config hwVersion"
#define ROUTER_GET_STAT_CLIENTS "cat /etc_ro/client_flow_stats.txt"


typedef struct t_RouterClient
{
 	char clientDownlinkRate[16];
 	char clientUplinkRate[16];
	char macAddr[32];
	char ipAddr[20];
	char name[128];
	char onlineDuration[32];
	char RSSI[8];
 	SLIST_ENTRY(recRouterClient) next;
} recRouterClient;

typedef struct t_RouterBoardband {
	int type;
	char PPPoEName[128];
	char PPPoECode[128];
	char ipAddr[16];
	char gateWayIP[16];
	char subMask[16];
} recRouterBoardband;

SLIST_HEAD(RouterClientList, recRouterClient);

time_t router_getCompileTimeStamp();
	

void convert_string_display(char *str);

void do_system(char *fmt, ...);

void router_getUpLinkSignal(const char* upLinkSSID, char *upLinkSignal);

/** 获取路由器Wifi信道 */
void router_getWifiChannel(char *wifiChannel, size_t len);

int router_getCfgValue(const char *cmd, char *cfgValue, size_t len);

int getAutoChannel( );

void router_getUpAndDownLinkRate(char* upLinkRate, char *downLinkRate);

char* router_getIpaddr(char *ifname);

int router_getConnectDevices(struct RouterClientList *prClientList);

int router_getAPConnectDevices(struct RouterClientList *prClientList);

int router_getNthValue(int index, char *value, char delimit, char *result, int len);

/** 字符串替换操作**/
char* router_strReplace(const char *original, const char *pattern,  const char *replacement);

/**Trim头部和尾部的无效字符**/
void router_trimLeft(char *s, char c);
void router_trimRight(char *s, char c);
void router_trim(char *s, char c);


void get_split_value(char *record, char *split, int index, char *result);
int getPidByName(char *proc_name);
char *set_nth_value(int index, char *old_values, char *new_value);
void set_nth_value_flash(int nvram, int index, char *flash_key, char *value);
char *strip_space(char *str);
unsigned int GetUptime(void);
char *get_macaddr(char *ifname);
char *get_macaddr_colon(char *ifname);
char *get_ipaddr(char *ifname);
char *get_boardcast_ipaddr(char *ifname);
char *get_ifname(void);
int get_popen_str(char *cmd, char *str, char *out, int len);
void wifi_power(char *radio, char *power);
void open_wifi(char *radio);
void close_wifi(char *radio);
void router_getDevSn(char *sn, int len);


#endif





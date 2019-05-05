#ifndef ZLINK_H
#define ZLINK_H

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
	#define ZDEBUG(format, ...)	printf("__LINE: %d__FUNC :%s__"format"\n", __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
	#define ZDEBUG(format, ...)
#endif

#define IF_NAMESIZE     16


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

SLIST_HEAD(RouterClientList, recRouterClient);

void zlink_getUpLinkSignal(const char* upLinkSSID, char *upLinkSignal);

/** 获取路由器Wifi信道 */
void zlink_getWifiChannel(char *wifiChannel, size_t len);

int router_getCfgValue(const char *cmd, char *cfgValue, size_t len);

void zlink_getUpAndDownLinkRate(char* upLinkRate, char *downLinkRate);

char *zlink_getIfname(void);

char* zlink_getIpaddr(char *ifname);

int zlink_getConnectDevices(struct RouterClientList *prClientList);

int zlink_getAPConnectDevices(struct RouterClientList *prClientList);


#endif





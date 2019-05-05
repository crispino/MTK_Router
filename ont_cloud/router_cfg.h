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
#include <net/route.h>
#include <sys/sysinfo.h>
#include <linux/wireless.h>
#include <ctype.h>
#include <errno.h>
#include "nvram.h"
#include "queue.h"
#include "oid.h"
#include "utils.h"
#include "autoconf.h"

#ifdef CONFIG_RTDEV
#define WIRELESS_5G
#endif


#define ROUTER_MAC		"web 2860 sys wanMacAddr"
#define ROUTER_DEVICE_NAME		"web 2860 nvram HostName"
#define ROUTER_WAN_IP_ADDR	"web 2860 sys wanIpAddr"
#define ROUTER_GET_WIRELESS_CLIENTS	"web 2860 wifi getWireless"

#define ROUTER_PHY_RATE	"100M"

#define PROTO_UNKNOWN	0
#define PROTO_TCP		1
#define PROTO_UDP		2
#define PROTO_TCP_UDP	3
#define PROTO_ICMP		4
#define PROTO_NONE		5

#define ACTION_DROP		0
#define ACTION_ACCEPT	1

#define IPPORT_FILTER_CHAIN				"macipport_filter"
#define ROUTER_GET_STAT_CLIENTS "cat /etc_ro/client_flow_stats.txt"


typedef struct t_RouterClient
{
 	char clientDownlinkRate[32];
 	char clientUplinkRate[32];
 	char deviceStatus[2];
	char macAddr[32];
	char ipAddr[20];
	char name[128];
	char onlineDuration[32];
	char accessInernet[2];
	char qosSwitch[2];
	char maxUplinkRate[32];
	char maxDownlinkRate[32];
	char ssid[128];
 	SLIST_ENTRY(recRouterClient) next;
	
} recRouterClient;


typedef struct t_QosRuleClient
{
	char name[128];
	char qosSwitch[8];
	char ipAddr[32];
	char macAddr[32];
	char maxUplinkRate[32];
	char maxDownlinkRate[32];
 	SLIST_ENTRY(recQosRuleClient) next;
} recQosRuleClient;


typedef struct t_RouterBoardband {
	char type[2];
	char PPPoEName[128];
	char PPPoECode[128];
	char ipAddr[16];
	char gateWayIP[16];
	char subMask[16];
	char dns1[16];
	char dns2[16];	
} recRouterBoardband;

typedef struct t_RouterSSIDEntry {
    char ssid[64];
    int index;    
} recRouterSSIDEntry;

#define MAX_SSID_NUM 1

typedef struct t_RouterSSIDTable {
    recRouterSSIDEntry Entry[MAX_SSID_NUM];
    int num;
} recRouterSSIDTable;


SLIST_HEAD(RouterClientList, recRouterClient);

SLIST_HEAD(RouterQosRuleList, recQosRuleClient);

int router_getCfgValue(const char *cmd, char *cfgValue, size_t len);


/** 获取路由器设备名称 */
void router_getDeviceName(char *devName, size_t len);

/** 获取路由器Wifi信道 */
void router_getWifiChannel(char *wifiChannel, size_t len);

/** 获取路由器在线时长，单位: 秒 */
void router_getOnlineDuration(char *onlineTime);

/** 获取路由器上连SSID */
void router_getLinkSSID(char *ssid, size_t len);

/** 获取防火墙开关状态 */
void router_getFilewareStatus(char *status, size_t len);

/** TODO 下行网速 */
void router_getDownlinkRate(char *downRate);

/** TODO 上行网速 */
void router_getUplinkRate(char *upRate);

/** 获取 Wan口 IP地址 **/
void router_getWanIpAddr(char * wanIp, size_t len);

/** 获取Wifi状态 */
void router_getWifiStatus(int nvram_id, char *wifiStatus, size_t len);

/** 获取连接客户端 */
int router_getConnectDevices(struct RouterClientList *prRouterList);

/** 获取连接客户端 */
int router_getConnectAPDevices(struct RouterClientList *prRouterList);


/** 根据ip地址获取QoS规则 */
int router_getQosRuleByIP(struct recQosRuleClient *prQosrule,char *ipaddress);

/** 获取网关信息  **/
void router_getGateway(char *sGateway, size_t len);

/** 获取DNS信息 */
void router_getDns(int index, char *sDns, size_t len);

/** 分割字段信息 */
void router_getSplitValue(char *record, char *split, int index, char *result);

/** 获取SSID 索引ID */
int router_getSSIDIndex(recRouterSSIDTable *pSSIDTable, const char *ssid);

/** 获取SSID 列表 */
void router_getSSIDTable(recRouterSSIDTable *pSSIDTable);


/** 根据Mac地址获取设备上网权限 */
int router_getClientAccessRight(const char* macAddr);

/** 设置上网权限 */
void router_setClientAccessRight(const char* macAddr, int accessFlag);

/** 设置每个客户端qos的规则 */
void router_setQosRule(const char* macAddr, int enable,const char* maxUpRate,const char* maxDownRate);

/** 获取Uplink SSID的信号强度 */
void router_getUpLinkSignal(const char* upLinkSSID, char *upLinkSignal);

/** 获取uplink rate 和 downlink rate */
void router_getUpAndDownLinkRate(char* upLinkRate, char *downLinkRate);

/** 获取每个客户端的uplink rate 和 downlink rate */
void router_getClientFlow(const char* ipAddr,char* clientDownLinkRate,char * clientUpLinkRate);

/** 获取每个客户端的Qos限速 */
void router_getQosRule(const char* macAddr,char *qosEnable,char* maxUpRate,char * maxDownRate);

/** 字符串替换操作**/
char * router_strReplace(const char *original, const char *pattern,  const char *replacement);

/**Trim头部和尾部的无效字符**/
void router_trimLeft(char *s, char c);
void router_trimRight(char *s, char c);
void router_trim(char *s, char c);

/** 防火墙设置 **/
void router_firewallSetting(char *opType);

int router_getNthValue(int index, char *value, char delimit, char *result, int len);

#endif





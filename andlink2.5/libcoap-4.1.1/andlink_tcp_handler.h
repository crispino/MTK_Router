/******************************************************************************
 * file:zlink_handler.h  
 * version: 1.0.0
 * description:- a client to simulate the gateway
 * modification history
 * --------------------
 * author: xiaoning 2018-06-08 15:06:48
 * --------------------
*******************************************************************************/


#ifndef ANDLINK_TCP_HANDLER_H
#define ANDLINK_TCP_HANDLER_H

#ifdef	__cplusplus
extern "C" {
#endif


#include "andlink_tcp_device.h"


/***********    RPCMethod    ***********/
#define MSG_HeartBeat					"heartbeat"
#define MSG_Disconnect					"Disconnect"
#define MSG_RequestPlugin				"RequestPlug-in"


#define MSG_WiFiParameterSync			"WiFiParameterSync"
#define MSG_WiFiSwitch					"WiFiSwitch"
#define MSG_RadioConfig					"RadioConfig"
#define MSG_WPS							"WPS"
#define MSG_MacFilter					"MacFilter"
#define MSG_Reboot						"Reboot"
#define MSG_LEDControl					"LEDControl"
#define MSG_RoamingConfig				"RoamingConfig"

#define MSG_ApConfigInfo				"ApConfigInfo"
#define MSG_UplinkStatus				"UplinkStatus"
#define MSG_STAInfo						"STAInfo"
#define MSG_WiFiStats					"WiFiStats"
#define MSG_Unbind						"unbind"
#define	MSG_File						"file"


typedef struct _ANDLINK_HANDLE
{
	char *rpcMethod;
	void (*func)();
	char *desc;
	
} ANDLINK_HANDLE;

#define ANDLINK_GET_RPCMethod(j) \
	(cJSON_HasObjectItem(j, "RPCMethod") ? cJSON_GetObjectItem(j, "RPCMethod")->valuestring : "")

#define ANDLINK_ARRAY_SIZE(array)	(sizeof(array)/sizeof(ANDLINK_HANDLE))

void vUdpMessageHandler(int nSockId,  struct sockaddr *sendaddr, const char* data, int data_len);

void vTcpMessageHandler(int nSockId, const char* data, int data_len);

void vTcpCommonResponse(int nSockId, const char* id, int respCode, const char* respContent);


/* boot and register **/
void udp_request_plugin(int nSockId, struct sockaddr *sendaddr, cJSON* json);

void udp_heartbeat(int nSockId, struct sockaddr *sendaddr, cJSON* json);

void tcp_heartbeat(int nSockId, cJSON* json);

void tcp_disconnect(int nSockId, cJSON* json);


/* Control **/
void tcp_control_wifi_para_sync(int nSockId, cJSON* json);
void tcp_control_wifi_swtich(int nSockId, cJSON* json);
void tcp_control_wifi_radio(int nSockId, cJSON* json);
void tcp_control_wifi_wps(int nSockId, cJSON* json);
void tcp_control_mac_filter(int nSockId, cJSON* json);
void tcp_control_reboot(int nSockId, cJSON* json);
void tcp_control_led_control(int nSockId, cJSON* json);
void tcp_control_sta_roaming(int nSockId, cJSON* json);


/* Query **/
void tcp_query_cfg_info(int nSockId, cJSON* json);

void tcp_query_ap_uplink(int nSockId, cJSON* json);

void tcp_query_sta_info(int nSockId, cJSON* json);

void tcp_query_dev_statis(int nSockId, cJSON* json);


void tcp_regist_dev_unbind(int nSockId, cJSON* json);

void tcp_regist_dev_file(int nSockId, cJSON* json);


#ifdef __cplusplus
}
#endif


#endif


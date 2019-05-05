#ifndef __ANDLINK__H__
#define __ANDLINK__H__

/* ++++++++++++ INCLUDE ++++++++++++ */
#include "config.h"
#include "resource.h"
#include "coap.h"
#include "cJSON.h"
#include "nvram.h"


/* ++++++++++++ GLOBAL VAR ++++++++++++ */
extern char *g_deviceId;
extern char *g_dev_ip;
extern char *g_dev_boardcast_ip;
extern char *g_dev_mac;
extern char *g_dev_up_mac;
extern char *g_dev_type;
extern char gwip[];


/* ++++++++++++ Event type ++++++++++++ */
#define EVENT_DEVICE_ONLINE		"EVENT_DEVICE_ONLINE"
#define EVENT_DEVICE_REONLINE	"EVENT_DEVICE_REONLINE"
#define EVENT_DEVICE_FILE		"EVENT_DEVICE_FILE"

/* ++++++++++++ define ++++++++++++ */
#define COAP_PROTOCOL	"coap://"
#define ANDLINK_PORT	5683

#define URL_LEN			128
#define MAX_SSID_NUM	5

#define UPLOAD_FILE "/var/tmpFW"

#define S_VERSION	"V1.0"
#define H_VERSION	"HV1.0"

#define UPGRADE_SUCCESS			2001
#define ONT_URL_ERR				-7
#define ONT_DOWNLOAD_ERR		-8
#define ONT_FILE_ERR			-9
#define ONT_WRITE_FLASH_ERR		-10


/* ++++++++++++ MACRO ++++++++++++ */
#define CREATE_URL(ip, uri)	char url[URL_LEN] = {0};snprintf(url, URL_LEN, "%s%s/%s", COAP_PROTOCOL, ip, uri);

#define REVISE_VAL(on) \
	(on == 1 ? 0 : 1)

#define HAS_XXX(j, e) \
	if (cJSON_HasObjectItem(j, e)) \

#define AL_RESP_SUCC(j) \
	if(cJSON_HasObjectItem(j, "respCode") \
		&& (AL_SUCCESS == cJSON_GetObjectItem(j, "respCode")->valueint))

#define AL_RESP_FAILED(j) \
			if(cJSON_HasObjectItem(j, "respCode") \
				&& (AL_SUCCESS != cJSON_GetObjectItem(j, "respCode")->valueint))

#define FILE_SUCCESS_RET \
			do {	\
				coap_add_data(response, strlen("{\"respCode\":0, \"respCont\":\"Success\"}"), \
					(unsigned char *)"{\"respCode\":0, \"respCont\":\"Success\"}"); \
				return ; \
			} while(0)

#define SUCCESS_RET \
			do {	\
				coap_add_data(response, strlen("{\"respCode\":1, \"respCont\":\"Success\"}"), \
					(unsigned char *)"{\"respCode\":1, \"respCont\":\"Success\"}"); \
				return ; \
			} while(0)

#define SUCCESS_NO_RET \
			do {	\
				coap_add_data(response, strlen("{\"respCode\":1, \"respCont\":\"Success\"}"), \
					(unsigned char *)"{\"respCode\":1, \"respCont\":\"Success\"}"); \
			} while(0)

#define FILE_SUCCESS_NO_RET \
			do {	\
				coap_add_data(response, strlen("{\"respCode\":0, \"respCont\":\"Success\"}"), \
					(unsigned char *)"{\"respCode\":0, \"respCont\":\"Success\"}"); \
			} while(0)

#define AUTH_FAILED_RET \
		do {	\
			coap_printf("Authorization failed"); \
			coap_add_data(response, strlen("{\"respCode\":1000, \"respCont\":\"Authorization failed\"}"), \
				(unsigned char *)"{\"respCode\":1000, \"respCont\":\"Authorization failed\"}"); \
			return ; \
		} while(0)

#define UNSUPP_ARG_RET \
		do {	\
			coap_add_data(response, strlen("{\"respCode\":1001, \"respCont\":\"Invalid argument\"}"), \
			(unsigned char *)"{\"respCode\":1001, \"respCont\":\"Invalid argument\"}"); \
			return ; \
		} while(0)

#define LACK_ARG_RET \
		do {	\
			coap_printf("Lack of argument"); \
			coap_add_data(response, strlen("{\"respCode\":1002, \"respCont\":\"Lack of argument\"}"), \
			(unsigned char *)"{\"respCode\":1002, \"respCont\":\"Lack of argument\"}"); \
			return ; \
		} while(0)

#define INVALID_VAL_RET \
		do {	\
			coap_add_data(response, strlen("{\"respCode\":1003, \"respCont\":\"Invalid value\"}"), \
			(unsigned char *)"{\"respCode\":1003, \"respCont\":\"Invalid value\"}"); \
			return ; \
		} while(0)

#define INVALID_CMD_RET \
		do {	\
			coap_add_data(response, strlen("{\"respCode\":1004, \"respCont\":\"Invalid command\"}"), \
			(unsigned char *)"{\"respCode\":1004, \"respCont\":\"Invalid command\"}"); \
			return ; \
		} while(0)

#define UPGRADE_SUCCESS_RET \
		do {	\
			coap_add_data(response, strlen("{\"respCode\":2001, \"respCont\":\"upgrade success\"}"), \
			(unsigned char *)"{\"respCode\":2001, \"respCont\":\"upgrade success\"}"); \
			return ; \
		} while(0)

#define UNKNOWN_ERR_RET \
		do {	\
			coap_add_data(response, strlen("{\"respCode\":9999, \"respCont\":\"Unknow Error\"}"), \
			(unsigned char *)"{\"respCode\":9999, \"respCont\":\"Unknow Error\"}"); \
			return ; \
		} while(0)

#define FILE_TRANSFER_COMPLETE		2000
#define FILE_UPGRADE_SUCCESS		2001
#define FILE_TRANSFREING			2002
#define FILE_TRANSFER_INTERRUPT		2003


/* my question. should response error code to GW when post is none? */
#define VERIFY_DATA(size) \
		if (0 == size) { \
			coap_printf("POST none"); \
			return; }


/* ++++++   device ----> GW  ++++++ */
/* qlink */
#define QLINK_SEARCHGW	"qlink/searchgw"
#define QLINK_REQUEST	"qlink/request"
#define QLINK_SUCCESS	"qlink/success"
/* regist */
#define DEV_REGIST		"device/inform/bootstrap" 	//子设备的注册不适用，忽略
#define DEV_ONLINE		"device/inform/boot"
#define DEV_OFFLINE		"device/inform/offline"		//不适用，忽略
#define DEV_HEARTBEAT	"device/inform/heartbeat"
#define DEV_NOTICE		"device/inform/data"		//暂无，根据需要扩展


/* ++++++   GW ----> device  ++++++ */
/* qlink */
#define QLINK_SEARCHACK	"qlink/searchack"
#define QLINK_NETINFO	"qlink/netinfo"
/* regist */
#define DEV_CONTROL		"device/command/control"		//已经转化为如下/apdevice/control 接口
#define DEV_DATA		"device/command/data"			//GET_CON，已经转化为如下/apdevice/query 接口
#define DEV_UNBIND		"device/command/unbind"
#define DEV_FILE		"device/command/file"
/* control */
#define WIFI_PARA_SYNC	"apdevice/control/WiFiParameterSync"
#define WIFI_SWTICH		"apdevice/control/WiFiSwitch"
#define WIFI_RADIO		"apdevice/control/RadioConfig"
#define WIFI_WPS		"apdevice/control/WPS"
#define MAC_FILTER		"apdevice/control/MacFilter"
#define DEV_REBOOT		"apdevice/control/Reboot"
#define LED_CONTROL		"apdevice/control/LEDControl"
#define STA_ROAMING		"apdevice/control/RoamingConfig"
#define AP_CFG_INFO		"apdevice/query/ApConfigInfo"	//GET_CON，设备配置信息查询
#define AP_UPLINK		"apdevice/query/UplinkStatus"	//GET_CON，上联状态
#define STA_INFO		"apdevice/query/STAInfo"		//GET_CON，下挂sta信息
#define DEV_STATIS		"apdevice/query/WiFiStats"		//GET_CON，收发报文统计

/* ++++++   description  ++++++ */
/* qlink */
#define QLINK_SEARCHACK_M	"\"qlink/searchack\""
#define QLINK_NETINFO_M		"\"qlink/netinfo\""
/* regist */
#define DEV_CONTROL_M		"\"device/command/control\""
#define DEV_DATA_M			"\"device/command/data\""
#define DEV_UNBIND_M		"\"device/command/unbind\""
#define DEV_FILE_M			"\"device/command/file\""
/* control */
#define WIFI_PARA_SYNC_M	"\"apdevice/control/WiFiParameterSync\""
#define WIFI_SWTICH_M		"\"apdevice/control/WiFiSwitch\""
#define WIFI_RADIO_M		"\"apdevice/control/RadioConfig\""
#define WIFI_WPS_M			"\"apdevice/control/WPS\""
#define MAC_FILTER_M		"\"apdevice/control/MacFilter\""
#define DEV_REBOOT_M		"\"apdevice/control/Reboot\""
#define LED_CONTROL_M		"\"apdevice/control/LEDControl\""
#define STA_ROAMING_M		"\"apdevice/control/RoamingConfig\""
#define AP_CFG_INFO_M		"\"apdevice/query/ApConfigInfo\""	//GET_CON
#define AP_UPLINK_M			"\"apdevice/query/UplinkStatus\""	//GET_CON
#define STA_INFO_M			"\"apdevice/query/STAInfo\""		//GET_CON
#define DEV_STATIS_M		"\"apdevice/query/WiFiStats\""		//GET_CON


/* ++++++++++++ FUNCTION ++++++++++++ */
int qlink_search_gw(const char *deviceId, const char *gwip);
void qlink_join_default_ssid(void);
int qlink_request(const char *deviceId, const char *gwip);
void qlink_join_ssid(void);
int qlink_success(const char *deviceId, const char *gwip);
int device_inform_regist(const char *gwip);
int device_inform_online(const char *deviceId, const char *gwip);
int device_inform_heartbeat(const char *deviceId, const char *gwip);
int device_inform_data(const char *deviceId, const char *gwip, char *event_type, int event_code, char *event_cont);


#endif

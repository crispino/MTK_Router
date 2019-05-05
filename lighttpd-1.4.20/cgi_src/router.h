#ifndef ROUTER_CGI_H
#define ROUTER_CGI_H

#include "utils.h"
#include "cJSON.h"


#define ONT_CMD_OK			0
#define ONT_RSP_ERR			-1
#define ONT_COMMON_ERR		-2
#define ONT_JSON_ERR		-3
#define ONT_MALLOC_ERR		-4
#define VERSION_N		"2.0"

#define MSG_WTP_SET_BROADBAND_SETTING_REQ		"MSG_WTP_SET_BROADBAND_SETTING_REQ"
#define MSG_WTP_SET_BROADBAND_SETTING_RSP		"MSG_WTP_SET_BROADBAND_SETTING_RSP"

#define MSG_WTP_GET_BROADBAND_SETTING_REQ		"MSG_WTP_GET_BROADBAND_SETTING_REQ"
#define MSG_WTP_GET_BROADBAND_SETTING_RSP		"MSG_WTP_GET_BROADBAND_SETTING_RSP"

#define MSG_APP_DEVICE_PASSWORD_REQ             "MSG_GET_DEVICE_PASSWORD_REQ"
#define MSG_WTP_DEVICE_PASSWORD_RSP			    "MSG_GET_DEVICE_PASSWORD_RSP"

#define MSG_APP_DEVICE_INFO_REQ                 "MSG_GET_DEVICE_INFO_REQ"
#define MSG_APP_DEVICE_INFO_RSP                 "MSG_GET_DEVICE_INFO_RSP"

#define CFG_ONT_PASSWD  "ont_passwd"

#define ONT_UNSUPPORT_MSGTYPE			"801004"
#define ONT_UPGRADE_FAILED				"801003"
#define ONT_UPGRADE_NO_DOMAIN			"801005"
#define ONT_UPGRADE_NO_PATH				"801006"
#define ONT_UPGRADE_NO_HOST				"801007"
#define ONT_UPGRADE_HOST_NOT_AVAILABLE	"801008"
#define ONT_UPGRADE_CONN_FAILED			"801009"
#define ONT_UPGRADE_NO_MORE_SPACE		"801010"
#define ONT_UPGRADE_NOT_FOUND			"801011"
#define ONT_UPGRADE_MD5_FAILED			"801012"

typedef struct _ONT_APP_LACAL_HANDLE
{
	char * name;
	int (*func)();
	char *desc;
	int mode;
} ONT_APP_LACAL_HANDLE;

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

#define ONT_ARRAY_SIZE(array)		(sizeof(array)/sizeof(ONT_APP_LACAL_HANDLE))

int ont_unsupported_msgtype_rsp(char *msgType, char *msgSeq);
int ont_get_device_password_rsp(char *msgSeq, cJSON *in_json);
int ont_get_device_info_rsp(char *msgSeq, cJSON *in_json);
int ont_set_broadband_setting_req(char *msgSeq, cJSON *in_json);
int ont_get_broadband_setting_rsp(char *msgSeq, cJSON *in_json);
int http_response_process_ont_app();

#endif


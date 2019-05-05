#ifndef	__SAMPLE_LINUX_H__
#define __SAMPLE_LINUX_H__

#include "ont/platform.h"
#include "ont/device.h"
#include "ont_tcp_channel.h"
//#include "sys_status.h"
//#include "version.h"
#include "cJSON.h"
//#include "eventlog.h"
//#include <cfg_def.h>

#define SN_LEN		16

extern char fw_version[];
extern char g_sn[];
extern char g_mac[];

/***********    typedefine    ***********/
typedef struct _ONT_HANDLE
{
	char * name;
	int (*func)();
	char *desc;
	int mode;
} ONT_HANDLE;

typedef struct _DEV_VENDOR
{
	char *mac;
	char *index;
} DEV_VENDOR;
extern DEV_VENDOR dev_vendor[];
extern char *vendor_name_info[];
extern int dev_vendor_num(void);

#define ONT_CMD_LEN		256
#define ONT_ARRAY_SIZE(array)		(sizeof(array)/sizeof(ONT_HANDLE))

#define ONT_CMD_OK			0
#define ONT_RSP_ERR			-1
#define ONT_COMMON_ERR		-2
#define ONT_JSON_ERR		-3
#define ONT_MALLOC_ERR		-4

#define ONT_FMW_NO_SPACE		-5
#define ONT_FMW_NOT_FOUND		-6
#define ONT_URL_ERR				-7
#define ONT_DOWNLOAD_ERR		-8
#define ONT_FILE_ERR			-9
#define ONT_WRITE_FLASH_ERR		-10


/***********    msgSeq    ***********/
/* Describe the rule */
#define GET_INITINFO					"13223614123123-111"
#define ABILITY_NOTIFY					"13223614123123-112"
#define DEVICE_STATUS_CHANGE			"13223614123"
#define DEV_UPDATE_INFO					"13223614123123-114"
#define UPLOAD_LOG						"13223614123123-115"


/***********    upgrade status    ***********/
//1: OK, 2: upgrading
#define UPGRADING						"2"
#define UPGRADE_COM						"1"
#define UPGRADE_FAL						"1"
//#define UPGRADING						"upgrading"
//#define UPGRADE_COM					"upgrade complete"
//#define UPGRADE_FAL					"upgrade failed"


#define ONT_VENDOR		"ZOWEE"
#define DEV_TYPE		"1" //第3位代表产品类别。1代表无线路由器，2代表无线中继器，3代表面板式AP。(详见<序列号规则>文档)
#define VERSION_N		"2.0"
#define ONE_DAY_TEST	"30"
#define ONE_DAY			(60*60*24)
#define ONT_IMAGE_SEGMENT		(4*1024)
#define UPLOAD_FILE	"/var/tmpFW"

#ifdef PRODUCT_RELEASE
#define SVR_NAME		"networkingsrv"
#else
#define SVR_NAME		"devenv"
#endif




/***********    msgType    ***********/
#define MSG_GET_INITINFO_REQ					"MSG_GET_INITINFO_REQ"
#define MSG_GET_INITINFO_RSP					"MSG_GET_INITINFO_RSP"

#define MSG_WTP_ABILITY_NOTIFY_REQ				"MSG_WTP_ABILITY_NOTIFY_REQ"
#define MSG_WTP_ABILITY_NOTIFY_RSP				"MSG_WTP_ABILITY_NOTIFY_RSP"

#define MSG_AP_GET_WIFI_SWITCH_STATUS_REQ		"MSG_AP_GET_WIFI_SWITCH_STATUS_REQ"
#define MSG_AP_GET_WIFI_SWITCH_STATUS_RSP		"MSG_AP_GET_WIFI_SWITCH_STATUS_RSP"

#define MSG_WTP_GET_DEVICE_WIFI_INFO_REQ		"MSG_WTP_GET_DEVICE_WIFI_INFO_REQ"
#define MSG_WTP_GET_DEVICE_WIFI_INFO_RSP		"MSG_WTP_GET_DEVICE_WIFI_INFO_RSP"

#define MSG_WTP_SET_DEVICE_WIFI_REQ				"MSG_WTP_SET_DEVICE_WIFI_REQ"
#define MSG_WTP_SET_DEVICE_WIFI_RSP				"MSG_WTP_SET_DEVICE_WIFI_RSP"

#define MSG_WTP_GET_DEVICE_INFO_REQ				"MSG_WTP_GET_DEVICE_INFO_REQ"
#define MSG_WTP_GET_DEVICE_INFO_RSP				"MSG_WTP_GET_DEVICE_INFO_RSP"

#define MSG_WTP_GET_DEVICE_WIFI_INFO_NEW_REQ	"MSG_WTP_GET_DEVICE_WIFI_INFO_NEW_REQ"
#define MSG_WTP_GET_DEVICE_WIFI_INFO_NEW_RSP	"MSG_WTP_GET_DEVICE_WIFI_INFO_NEW_RSP"

#define MSG_WTP_SET_DEVICE_WIFI_NEW_REQ			"MSG_WTP_SET_DEVICE_WIFI_NEW_REQ"
#define MSG_WTP_SET_DEVICE_WIFI_NEW_RSP			"MSG_WTP_SET_DEVICE_WIFI_NEW_RSP"

#define MSG_WTP_WIFI_ACCELERATE_REQ				"MSG_WTP_WIFI_ACCELERATE_REQ"
#define MSG_WTP_WIFI_ACCELERATE_RSP				"MSG_WTP_WIFI_ACCELERATE_RSP"

#define MSG_WTP_DEVICE_OPERATE_REQ				"MSG_WTP_DEVICE_OPERATE_REQ"
#define MSG_WTP_DEVICE_OPERATE_RSP				"MSG_WTP_DEVICE_OPERATE_RSP"

#define MSG_WTP_DEVICE_STATUS_CHANG_REQ			"MSG_WTP_DEVICE_STATUS_CHANG_REQ"
#define MSG_WTP_DEVICE_STATUS_CHANG_RSP			"MSG_WTP_DEVICE_STATUS_CHANG_RSP"

#define MSG_GET_DEVUPDATE_INFO_REQ				"MSG_GET_DEVUPDATE_INFO_REQ"
#define MSG_GET_DEVUPDATE_INFO_RSP				"MSG_GET_DEVUPDATE_INFO_RSP"

#define MSG_SEND_DEVUPDATE_REQ					"MSG_SEND_DEVUPDATE_REQ"
#define MSG_SEND_DEVUPDATE_RSP					"MSG_SEND_DEVUPDATE_RSP"

#define MSG_WTP_LOG_UPLOAD_REQ					"MSG_WTP_LOG_UPLOAD_REQ"
#define MSG_WTP_LOG_UPLOAD_RSP					"MSG_WTP_LOG_UPLOAD_RSP"

/***********    APP msgType    ***********/
#define MSG_WTP_SCAN_DEVICES_REQ				"MSG_WTP_SCAN_DEVICES_REQ"

#define MSG_WTP_ONLINE_NOTIFICATION_REQ			"MSG_WTP_ONLINE_NOTIFICATION_REQ"

#define MSG_WTP_GET_ROUTE_OVERVIEW_REQ			"MSG_WTP_GET_ROUTE_OVERVIEW_REQ"
#define MSG_WTP_GET_ROUTE_OVERVIEW_RSP			"MSG_WTP_GET_ROUTE_OVERVIEW_RSP"

#define MSG_WTP_GET_BROADBAND_SETTING_REQ		"MSG_WTP_GET_BROADBAND_SETTING_REQ"
#define MSG_WTP_GET_BROADBAND_SETTING_RSP		"MSG_WTP_GET_BROADBAND_SETTING_RSP"

#define MSG_WTP_SET_BROADBAND_SETTING_REQ		"MSG_WTP_SET_BROADBAND_SETTING_REQ"
#define MSG_WTP_SET_BROADBAND_SETTING_RSP		"MSG_WTP_SET_BROADBAND_SETTING_RSP"

#define MSG_WTP_GET_ROUTE_BASE_INFO_REQ			"MSG_WTP_GET_ROUTE_BASE_INFO_REQ"
#define MSG_WTP_GET_ROUTE_BASE_INFO_RSP			"MSG_WTP_GET_ROUTE_BASE_INFO_RSP"

#define MSG_WTP_GET_CLIENT_ACCESS_CONTROL_INFO_REQ      "MSG_WTP_GET_CLIENT_ACCESS_CONTROL_INFO_REQ"
#define MSG_WTP_GET_CLIENT_ACCESS_CONTROL_INFO_RSP      "MSG_WTP_GET_CLIENT_ACCESS_CONTROL_INFO_RSP"

#define MSG_WTP_SET_CLIENT_ACCESS_CONTORL_REQ      "MSG_WTP_SET_CLIENT_ACCESS_CONTROL_REQ"
#define MSG_WTP_SET_CLIENT_ACCESS_CONTORL_RSP      "MSG_WTP_SET_CLIENT_ACCESS_CONTROL_RSP"

#define MSG_WTP_SET_QOS_REQ							"MSG_WTP_SET_QOS_REQ"
#define MSG_WTP_SET_QOS_RSP							"MSG_WTP_SET_QOS_RSP"

/***********    error codes    ***********/
#define ONT_SUCCESS				"0000"
#define ONT_SRV_ERR				"0001"
#define ONT_FORM_ERR			"0002"
#define ONT_PARAMETER_ERR        "0003"
#define ONT_OTHER_EXAMPLE		"801000"

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

#define ONT_RET_ERR(e, v) \
	strcpy(e, v)


/***********    funciton    ***********/
extern void addReqhead(cJSON *, char *, char *, char *);
extern void addRsphead(cJSON *, char *, char *, char *, char *, char *);

extern int ont_get_initinfo_req(ont_device_t *);
extern int ont_get_initinfo_rsp(ont_device_t *, const char *, size_t);

extern int ont_unsupported_msgtype_rsp(ont_device_t *, char *, char *);

extern int ont_wtp_ability_notify_req(ont_device_t *);
extern int ont_wtp_ability_notify_rsp(ont_device_t *, const char *, size_t);

extern int ont_ap_get_wifi_info_req(ont_device_t *, const char *, size_t);
extern int ont_ap_set_wifi_info_req(ont_device_t *, const char *, size_t);

extern int ont_ap_get_wifi_switch_status_req(ont_device_t *, const char *, size_t);

extern int ont_wtp_get_device_info_req(ont_device_t *, const char *, size_t);

extern int ont_wtp_get_device_wifi_info_req(ont_device_t *, const char *, size_t);

extern int ont_wtp_set_device_wifi_req(ont_device_t *, const char *, size_t);

extern int ont_wtp_wifi_accelerate_req(ont_device_t *, const char *, size_t);

extern int ont_wtp_device_operate_req(ont_device_t *, const char *, size_t);

extern int ont_wtp_device_status_chang_req(ont_device_t *, char *);
extern int ont_wtp_device_status_chang_rsp(ont_device_t *, const char *, size_t);

extern int ont_get_devupdate_info_req(ont_device_t *);
extern int ont_get_devupdate_info_rsp(ont_device_t *, const char *, size_t);

extern int ont_send_devupdate_req(ont_device_t *, const char *, size_t);

extern int ont_wtp_log_upload_req(ont_device_t *, const char *);
extern int ont_wtp_log_upload_rsp(ont_device_t *, const char *, size_t);

extern void ont_sys_reboot(ont_device_t *);

/**获取路由器基本信息 */
extern int ont_wtp_get_route_overview_req(ont_device_t *, const char *, size_t);
extern int ont_wtp_get_route_overview_rsp(ont_device_t *, char *);

/**获取上网信息 */
extern int ont_wtp_get_boardband_setting_req(ont_device_t *, const char *, size_t);
extern int ont_wtp_get_boardband_setting_rsp(ont_device_t *, char *);


/**设置上网信息*/
extern int ont_wtp_set_boardband_setting_req(ont_device_t *, const char *, size_t);
extern int ont_wtp_set_boardband_setting_rsp(ont_device_t *, char *);

/** 获取路由器基本信息 */
extern int ont_wtp_get_base_info_req(ont_device_t *, const char *, size_t);
extern int ont_wtp_get_base_info_rsp(ont_device_t *, char *);

/**查询连接设备上网权限 **/
extern int ont_wtp_get_access_control_info_req(ont_device_t *, const char *, size_t);
extern int ont_wtp_get_access_control_info_rsp(ont_device_t *, char *);

/**设置连接设备上网权限 **/
extern int ont_wtp_set_acccess_control_req(ont_device_t *, const char *, size_t);
extern int ont_wtp_set_acccess_control_rsp(ont_device_t *, char *);

/**设置连接qos规则 **/
extern int ont_wtp_set_qos_req(ont_device_t *, const char *, size_t);
extern int ont_wtp_set_qos_rsp(ont_device_t *, char *);


/***********    APP funciton    ***********/
extern int ont_wtp_scan_devices_req(int sockfd, const char *recvMsg, struct sockaddr *cliaddr);


/***********    ont app thread   ***********/
extern void thread_ont_app(void);


/***********    AES ECB interface   ***********/
extern ont_aes128_ecb_pkcs5_encrypt(unsigned char key [ 16 ], unsigned char *, unsigned char *);
extern ont_aes128_ecb_pkcs5_decrypt(unsigned char key [ 16 ], unsigned char *, unsigned char *);


extern int ont_cmd_test(void);
extern int ont_cmd_factory_reset(void);
extern int ont_cmd_reboot(void);

#endif


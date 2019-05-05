/*	
 * OneNet cmds handle.
 *
 *
*/
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/reboot.h>
#include "nvram.h"
#include "base64.h"
#include "ont_handle.h"
#include "router_cfg.h"

#define SYS_PRODUCT_NAME "CMTT"

#define CFG_ONT_CYCLE "ont_cycle"
#define CFG_ONT_PVER  "ont_pversion"
#define CFG_ONT_PASSWD  "ont_passwd"

#define GET_DEV_TYPE(NAME, SN) \
	 char NAME[2] = {0}; NAME[0] = SN[2]; NAME[1] = '\0';


char *safe_strncpy(char *dst, const char *src, size_t size)
{   
    dst[size-1] = '/0';
    return strncpy(dst,src,size-1);   
}

void composeRspHead(char *message, char *msgType, char *msgSeq, char *errcode, char *desc)
{
    sprintf(message, "{\"version\":\"%s\", \"msgType\":\"%s\",\"msgSeq\":\"%s\"," 
                     "\"erroCode\":\"%s\", \"description\":\"%s\"}",
                VERSION_N, msgType, msgSeq, errcode, desc);

}

int responseRequestParseError(ont_device_t *dev)
{
    char response[256] = {0};
	composeRspHead(response,  "COMMON", "0",  ONT_FORM_ERR, "json format error");
	int err = ont_edp_send_transdata(dev, SVR_NAME, response, strlen(response));
    if (ONT_ERR_OK != err) {
		SL_DEBUG("Failed to send transdata , error=%d", err);
		return ONT_RSP_ERR;
	}

    return ONT_ERR_OK;
}        

int responseSystemError(ont_device_t *dev, char *msgType, char *msgSeq)
{
    char response[256] = {0};
	composeRspHead(response,  msgType, msgSeq,  ONT_SRV_ERR, "system error");
	int err = ont_edp_send_transdata(dev, SVR_NAME, response, strlen(response));
    if (ONT_ERR_OK != err) {
		SL_DEBUG("Failed to send transdata , error=%d", err);
		return ONT_RSP_ERR;
	}

    return ONT_ERR_OK;
}

int responseParameterInvalid(ont_device_t *dev, char *msgType, char *msgSeq)
{
    char response[256] = {0};
	composeRspHead(response,  msgType, msgSeq,  ONT_PARAMETER_ERR, "json parameters error");
	int err = ont_edp_send_transdata(dev, SVR_NAME, response, strlen(response));
    if (ONT_ERR_OK != err) {
		SL_DEBUG("Failed to send transdata , error=%d", err);
		return ONT_RSP_ERR;
	}

    return ONT_ERR_OK;
}

void addReqhead(cJSON *json, char *version, char *msgType, char *msgSeq)
{
	cJSON_AddItemToObject(json, "version", cJSON_CreateString(version));
	cJSON_AddItemToObject(json, "msgType", cJSON_CreateString(msgType));
	cJSON_AddItemToObject(json, "msgSeq", cJSON_CreateString(msgSeq));

	return ;
}

void addRsphead(cJSON *json, char *version, char *msgType, char *msgSeq, 
		char *errcode, char *desc)
{
	cJSON_AddItemToObject(json, "version", cJSON_CreateString(version));
	cJSON_AddItemToObject(json, "msgType", cJSON_CreateString(msgType));
	cJSON_AddItemToObject(json, "msgSeq", cJSON_CreateString(msgSeq));
	cJSON_AddItemToObject(json, "erroCode", cJSON_CreateString(errcode));
	cJSON_AddItemToObject(json, "description", cJSON_CreateString(desc));

	return ;
}

/*
 * name: ont unsupported msgtype rsp
 * desc: 终端不支持的操作，返回错误码。
 * direction: WTP ----> AMNMP
 * other: 
*/
int ont_unsupported_msgtype_rsp(ont_device_t *dev, char *msgType, char *msgSeq)
{
	int err = ONT_CMD_OK;
	cJSON *json = NULL;
	char *jsonValue = NULL;
	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	addRsphead(json, VERSION_N, msgType, msgSeq, ONT_UNSUPPORT_MSGTYPE, "Unsupported msgtype");

	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("rsp: %s", jsonValue);
	err = ont_edp_send_transdata(dev, SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		err = ONT_RSP_ERR;
	}
	
	ont_platform_free(jsonValue);
	cJSON_Delete(json);

	return err;
}

/*
 * name: get initinfo req
 * desc: 设备接入平台第一件事是获取初始化信息。
 * direction: WTP ----> AMNMP
 * other: 
*/
int ont_get_initinfo_req(ont_device_t *dev)
{
	SL_DEBUG();
	int err = ONT_CMD_OK;
	cJSON *json = NULL;
	char *jsonValue = NULL;

	GET_DEV_TYPE(devType, g_sn);

	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	addReqhead(json, VERSION_N, MSG_GET_INITINFO_REQ, GET_INITINFO);
	cJSON_AddItemToObject(json, "devID", cJSON_CreateString(g_sn));
	cJSON_AddItemToObject(json, "deviceType", cJSON_CreateString(devType));
	cJSON_AddItemToObject(json, "IP", cJSON_CreateString(get_ipaddr(get_ifname())));

	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("%s", jsonValue);
	err = ont_edp_send_transdata(dev, SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		err = ONT_RSP_ERR;
	}
	
	ont_platform_free(jsonValue);
	cJSON_Delete(json);

	return err;
}


/*
 * name: get initinfo rsp
 * desc: 接受平台返回的初始化信息并配置到设备中。
 * direction: WTP <---- AMNMP
 * other: 
*/
int ont_get_initinfo_rsp(ont_device_t *dev, const char *data, size_t data_len)
{
	SL_DEBUG("check data: %s", data);

	cJSON *json = NULL;
	char *cycle = NULL, *time = NULL, *timeZone = NULL, *pVersion = NULL, *passwd = NULL; 
	char settime_cmd[64] = {0};
	char busybox_time[16] = {0};
	char ont_password[512] = {0};

	json = cJSON_Parse(data);
	if (!json) return ONT_JSON_ERR;
	
	ONT_CHECK_JSON(json) //failed
	{
		SL_DEBUG("cJSON format error");
		cJSON_Delete(json);
		return ONT_COMMON_ERR;
	}
	else
	{
		cycle = cJSON_GetObjectItem(json, "cycle")->valuestring;
		time = cJSON_GetObjectItem(json, "time")->valuestring;
		timeZone = cJSON_GetObjectItem(json, "timeZone")->valuestring;
		pVersion = cJSON_GetObjectItem(json, "platformVersion")->valuestring;
		passwd = cJSON_GetObjectItem(json, "password")->valuestring;

		char encode_encrypt_pw[512] = {0};
		ont_platform_snprintf(encode_encrypt_pw, sizeof(encode_encrypt_pw), "%s", passwd);
		SL_DEBUG("ont encrypt password: %s", encode_encrypt_pw);

		//Base64Decode and AES Decrypt
		char base64_target[512] = {0};
		unsigned char key[17] = {0};
		ont_platform_snprintf(key, sizeof key, ONT_KEY);
		base64_decode(encode_encrypt_pw, base64_target, sizeof base64_target);
		SL_DEBUG("ont decode password: %s", base64_target);
		ont_aes128_ecb_pkcs5_decrypt(key, base64_target, ont_password);
		SL_DEBUG("ont decrypt password: %s", ont_password);

		/* should setting */
		//cycle, keepalive time, reserve.
		/*CFG_set_str(CFG_ONT_CYCLE, cycle);
		CFG_set_str(CFG_ONT_TIME, time);
		SL_DEBUG("get timeZone: %s", timeZone);
		CFG_set_str(CFG_SYS_TZONE, "45");
		CFG_set_str(CFG_ONT_PVER, pVersion);
		CFG_set_str(CFG_ONT_PASSWD, ont_password);*/
		nvram_bufset(RT2860_NVRAM, CFG_ONT_CYCLE, cycle);
		
		strncpy(busybox_time, &time[4], 8);// MMDDHHMM
		strncpy(&busybox_time[8], time, 4);//YYYY
		busybox_time[12] = '\0';
		sprintf(settime_cmd, "date -s %s", busybox_time);
		SL_DEBUG("busybox settime: %s\n", busybox_time);
		system(settime_cmd);

		nvram_bufset(RT2860_NVRAM, CFG_ONT_PVER, pVersion);
        // for binding cellphone APP, store the crypted password from ONT cloud platform
		nvram_bufset(RT2860_NVRAM, CFG_ONT_PASSWD, passwd); 

		ont_wtp_ability_notify_req(dev);
	}

	cJSON_Delete(json);

	return ONT_CMD_OK;
}



void buildApSsid(char * new_ssid)
{

	char mac_copy[14];

	memset(mac_copy,0,sizeof(mac_copy));
	strncpy(mac_copy, "001234567890", sizeof(mac_copy));
    
	sprintf(new_ssid,"%s_%s",SYS_PRODUCT_NAME,mac_copy+6);
}

/*
 * name: wtp ability notify req
 * desc: 终端向平台上报属性。
 		设置获取到初始化信息后，完成上报属性方能正常使用
 * direction: WTP ----> AMNMP
 * other: 
*/
int ont_wtp_ability_notify_req(ont_device_t *dev)
{
	int err = ONT_CMD_OK;
	cJSON *json = NULL;
    cJSON *array = NULL;
	cJSON *deviceCapacity = NULL;
	char *jsonValue = NULL;
	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	//do something, devName isn't sure
	char *devName = "CMCC_Andmu"; //设备名称，CMTT_778899
	char *fac = ONT_VENDOR; //设备厂家.ZOWEE
	
	GET_DEV_TYPE(devType, g_sn); //设备型号
	devName = nvram_get(RT2860_NVRAM, "SSID1");

	addReqhead(json, VERSION_N, MSG_WTP_ABILITY_NOTIFY_REQ, ABILITY_NOTIFY);
	
	cJSON_AddItemToObject(json, "devID", cJSON_CreateString(g_sn));
	cJSON_AddItemToObject(json, "devName", cJSON_CreateString(devName));
	cJSON_AddItemToObject(json, "fac", cJSON_CreateString(fac));
	cJSON_AddItemToObject(json, "type", cJSON_CreateString(devType));
	cJSON_AddItemToObject(json, "softVersion", cJSON_CreateString(fw_version));
	
	cJSON_AddItemToObject(json, "capacitySet", array = cJSON_CreateArray());
	cJSON_AddItemToObject(array, "", deviceCapacity = cJSON_CreateObject());

	cJSON_AddItemToObject(deviceCapacity, "enable2GAccess", cJSON_CreateString("1"));  // 1-支持, 0 - 不支持
#ifdef WIRELESS_5G
	cJSON_AddItemToObject(deviceCapacity, "enable5GAccess", cJSON_CreateString("1"));  // 1-支持, 0 - 不支持
#else
	cJSON_AddItemToObject(deviceCapacity, "enable5GAccess", cJSON_CreateString("0"));  // 1-支持, 0 - 不支持
#endif

	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("%s", jsonValue);
	err = ont_edp_send_transdata(dev, SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		err = ONT_RSP_ERR;
	}
	
	ont_platform_free(jsonValue);
	cJSON_Delete(json);

	return err;
}


/*
 * name: wtp ability notify rsp
 * desc: 上报属性成功后平台的回包。貌似不用做任何处理。
 * direction: WTP <---- AMNMP
 * other: 
*/
int ont_wtp_ability_notify_rsp(ont_device_t *dev, const char *data, size_t data_len)
{
	SL_DEBUG("check data: %s", data);

	return ONT_CMD_OK;
}

#if defined (CONFIG_PRODUCT_339AA) || defined (CONFIG_PRODUCT_339AA_JS) || defined (CONFIG_PRODUCT_331AA)
/*
 * name: set AP wifi rsp
 * desc: 终端响应平台或者APP的WiFi设置请求，无可变消息。
 * direction: WTP ----> AMNMP/APP
 * other: 
*/
static int ont_ap_set_wifi_info_rsp(ont_device_t *dev, char *msgSeq)
{
	int err = ONT_CMD_OK;
	
	cJSON *json = NULL;
	char *jsonValue = NULL;
	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	addRsphead(json, VERSION_N, MSG_WTP_SET_DEVICE_WIFI_RSP, msgSeq, ONT_SUCCESS, "set AP wifi rsp");

	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("rsp: %s", jsonValue);
	err = ont_edp_send_transdata(dev, SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);		
		err = ONT_RSP_ERR;
	}
	
	ont_platform_free(jsonValue);
	cJSON_Delete(json);

	return err;
}


/*
 * name: set AP wifi req
 * desc: 平台或者APP发起设置AP WiFi的请求
 * direction: WTP <---- AMNMP/APP
 * other: 
*/
int ont_ap_set_wifi_info_req(ont_device_t *dev, const char *data, size_t data_len)
{
	SL_DEBUG("check data: %s", data);

	cJSON *json =NULL;
	char *ssid =NULL, *code =NULL, *type =NULL; 

	json = cJSON_Parse(data);
	if (!json) return ONT_JSON_ERR;
	
	ONT_CHECK_JSON(json) //failed
	{
		SL_DEBUG("cJSON format error");
		cJSON_Delete(json);
		return ONT_COMMON_ERR;
	}
	else
	{
		do {
			if (!cJSON_HasObjectItem(json, "SSID")) {			  
				SL_DEBUG("cJSON SSID format error");
				break; 			
			}

			ssid = cJSON_GetObjectItem(json, "SSID")->valuestring;
		
			ONT_CHECK_ITEM(json, "CODE")
				code = cJSON_GetObjectItem(json, "CODE")->valuestring;
			ONT_CHECK_ITEM(json, "encryptType")
				type = cJSON_GetObjectItem(json, "encryptType")->valuestring;
			
			SL_DEBUG("SSID: %s, CODE: %s, encryptType: %s", ssid, code, type);
			
			//Set wifi info
			set_wifi_info(ssid, code, type);

		} while (0);		

	}
        
    ont_ap_set_wifi_info_rsp(dev, ONT_GET_MSGSEQ(json));

    nvram_commit(RT2860_NVRAM);
    do_system("init_system restart");
	cJSON_Delete(json);
	return ONT_CMD_OK;
}

static int ont_ap_get_wifi_info_rsp(ont_device_t *dev, char *msgSeq)
{
	int err = ONT_CMD_OK;
	cJSON *json = NULL;
	char *jsonValue = NULL;
	int Index =0;
	char *ssid = NULL, *pAuthMode = NULL, *pPassword = NULL;
	char sTemp[32] = {0};
	char sAuthMode[2] = {0};
	char real_chiper[256] = {0};
	char base64_target[512] = {0};
	char *key = ONT_KEY;
	char sWifiStatus[4] = {0};

	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	addRsphead(json, VERSION_N, MSG_WTP_GET_DEVICE_WIFI_INFO_RSP, msgSeq, ONT_SUCCESS, "AP WiFi infos rsp");

	for(Index =1; Index <= 1; Index++) {   //Support 4 SSIDS 	
		
		memset(sTemp, 0, sizeof(sTemp));
		sprintf(sTemp, "SSID%d", Index);
		ssid = nvram_get(RT2860_NVRAM, sTemp);
		if (strlen(ssid) > 0) {
			cJSON_AddItemToObject(json, "SSID", cJSON_CreateString(ssid));
			//1. Get CODE			  
			memset(sTemp, 0x0, sizeof(sTemp));
			memset(real_chiper, 0x0, sizeof(real_chiper));
			memset(base64_target, 0x0, sizeof(base64_target));
			sprintf(sTemp, "WPAPSK%d", Index);
			pPassword = nvram_get(RT2860_NVRAM, sTemp);
			SL_DEBUG("ont plain password: %s", pPassword);
			ont_aes128_ecb_pkcs5_encrypt(key, pPassword, real_chiper);
			SL_DEBUG("ont encode password: %s", real_chiper);
			base64_encode(real_chiper, strlen(real_chiper), base64_target, sizeof(base64_target) -1);
			SL_DEBUG("ont base64 password: %s", base64_target); 						   
			
			//2. Get AuthMode
			pAuthMode = nvram_get(RT2860_NVRAM, "AuthMode");		
			memset(sTemp, 0x0, sizeof(sTemp));
			memset(sAuthMode, 0x0, sizeof(sAuthMode));
			//Split the AuthMode like: WPAPSKWPA2PSK;WPAPSKWPA2PSK;OPEN;OPEN
			router_getSplitValue(pAuthMode, ";", Index, sTemp);
			if (!strcmp(sTemp, "WPAPSK")) {
				sAuthMode[0] = '3';
			} else if (!strcmp(sTemp, "WPA2PSK")) {
				sAuthMode[0] = '5';
			} else if (!strcmp(sTemp, "WPAPSKWPA2PSK")) {
				sAuthMode[0] = '7';
			} else {
				sAuthMode[0] = '1';
			}

			cJSON_AddItemToObject(json, "CODE", cJSON_CreateString(base64_target));
			cJSON_AddItemToObject(json, "encryptType", cJSON_CreateString(sAuthMode));
			
		}
	}

	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("rsp: %s", jsonValue);
	err = ont_edp_send_transdata(dev, SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		err = ONT_RSP_ERR;
	}
	
	ont_platform_free(jsonValue);
	cJSON_Delete(json);

	return err;
}

/*
 * name: get wifi AP info
 * desc: 平台请求获取AP设备wifi信息
 * direction: WTP <---- AMNMP/APP
 * other: 
*/
int ont_ap_get_wifi_info_req(ont_device_t *dev, const char *data, size_t data_len)
{
	SL_DEBUG("check data: %s", data);

	cJSON *json = NULL;

	json = cJSON_Parse(data);
	if (!json) return ONT_JSON_ERR;
	
	ONT_CHECK_JSON(json) //failed
	{
		SL_DEBUG("cJSON format error");
		cJSON_Delete(json);
		return ONT_COMMON_ERR;
	}
	else
	{
		ont_ap_get_wifi_info_rsp(dev, ONT_GET_MSGSEQ(json));
	}

	cJSON_Delete(json);

	return ONT_CMD_OK;
}

/*
 * name: wifi switch status rsp
 * desc: 终端向平台或者APP返回WiFi开关状态
 * direction: WTP ----> AMNMP/APP
 * other: 
*/
static int ont_ap_get_wifi_switch_status_rsp(ont_device_t *dev, char *msgSeq)
{
	int err = ONT_CMD_OK;
	cJSON *json = NULL;
	char *jsonValue = NULL;
	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	addRsphead(json, VERSION_N, MSG_AP_GET_WIFI_SWITCH_STATUS_RSP, msgSeq, ONT_SUCCESS, "wifi switch status rsp");
	char status[4] = {0};
    router_getWifiStatus(RT2860_NVRAM, status, sizeof(status) -1);

	cJSON_AddItemToObject(json, "status", cJSON_CreateString(status));

	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("rsp: %s", jsonValue);
	err = ont_edp_send_transdata(dev, SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		err = ONT_RSP_ERR;
	}
	
	ont_platform_free(jsonValue);
	cJSON_Delete(json);

	return err;
}


/*
 * name: get wifi switch status req
 * desc: 平台请求获取终端WiFi开关状态
 * direction: WTP <---- AMNMP/APP
 * other: 
*/
int ont_ap_get_wifi_switch_status_req(ont_device_t *dev, const char *data, size_t data_len)
{
	SL_DEBUG("check data: %s", data);

	cJSON *json = NULL;

	json = cJSON_Parse(data);
	if (!json) return ONT_JSON_ERR;
	
	ONT_CHECK_JSON(json) //failed
	{
		SL_DEBUG("cJSON format error");
		cJSON_Delete(json);
		return ONT_COMMON_ERR;
	}
	else
	{
		ont_ap_get_wifi_switch_status_rsp(dev, ONT_GET_MSGSEQ(json));
	}

	cJSON_Delete(json);

	return ONT_CMD_OK;
}

/*
 * 获取上级信号强度
 * 1:强，2:中，3:低
 */
char *get_signal_level(int signal)
{
	SL_DEBUG("signal: %d", signal);
	char *signal_tag = "2";
	if(signal>-60)
		signal_tag="1";
	else if(signal>-70)
		signal_tag="2";
	else if(signal>-78)
		signal_tag="2";
	else if(signal>-85)
		signal_tag="3";
	else
		signal_tag="3";
	return signal_tag;
}

char ont_clients[2048] = {0};

struct ont_dev_info
{
	char mac[18];
	char _hostname[255];
	struct in_addr inaddr;
	int exist;
	int online;
};

#define MAX_CLIENTS_NUM		32
struct ont_dev_info ont_devs_info[MAX_CLIENTS_NUM];
int signal_val = 0;

void get_device_info_json(cJSON *root)
{
	SL_DEBUG();
	cJSON *array = NULL;
	cJSON *dhcp_client = NULL;
	char sWifiChannel[32] = {0}; //设备无线信道
	int onlineDuration = 0; //设备上线时长
	char sOnlineTime[32] = {0};
	char *upLineSignal = NULL;
	char *get_device = NULL;

	GET_DEV_TYPE(devType, g_sn); //设备型号

	//1. get router base info
	router_getWifiChannel(sWifiChannel, sizeof(sWifiChannel)-1);
	router_getOnlineDuration(sOnlineTime);

	//  TODO: updateApcliRSSI();
	upLineSignal = get_signal_level(signal_val);
	SL_DEBUG("signal_level %s", upLineSignal);

	cJSON_AddItemToObject(root, "type", cJSON_CreateString(devType));
	cJSON_AddItemToObject(root, "MAC", cJSON_CreateString(g_mac));
	cJSON_AddItemToObject(root, "IP", cJSON_CreateString(get_ipaddr(get_ifname())));
	cJSON_AddItemToObject(root, "channel", cJSON_CreateString(sWifiChannel));
	cJSON_AddItemToObject(root, "softwareVersion", cJSON_CreateString(fw_version));
	cJSON_AddItemToObject(root, "onlineDuration", cJSON_CreateString(sOnlineTime));
	cJSON_AddItemToObject(root, "upLineSignal", cJSON_CreateString(upLineSignal));

	int flag = 1;
	struct RouterClientList clientHead;
	SLIST_INIT(&clientHead);
	router_getConnectAPDevices(&clientHead);
	recRouterClient *prClientNode;
	SLIST_FOREACH(prClientNode, &clientHead, next)
	{
		//first time
		if (flag)
		{
			flag = 0;
			cJSON_AddItemToObject(root, "connectDevices", array = cJSON_CreateArray());
		}

		cJSON_AddItemToObject(array, "", dhcp_client= cJSON_CreateObject());
		cJSON_AddItemToObject(dhcp_client, "deviceStatus", cJSON_CreateString("2")); //1: 连接. 2: 活跃
		cJSON_AddItemToObject(dhcp_client, "MAC", cJSON_CreateString(prClientNode->macAddr));
		cJSON_AddItemToObject(dhcp_client, "IP", cJSON_CreateString(prClientNode->ipAddr));
		cJSON_AddItemToObject(dhcp_client, "name", cJSON_CreateString(prClientNode->name));
		cJSON_AddItemToObject(dhcp_client, "onlineDuration", cJSON_CreateString(prClientNode->onlineDuration));
		free(prClientNode);
	}

	return ONT_CMD_OK;
}

/*
 * name: get device info rsp
 * desc: 终端向平台或APP返回设备基本信息
 * direction: WTP ----> AMNMP/APP
 * other: 
*/
static int ont_wtp_get_device_info_rsp(ont_device_t *dev, char *msgSeq)
{
	int err = ONT_CMD_OK;
	cJSON *json = NULL;
	char *jsonValue = NULL;
	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	addRsphead(json, VERSION_N, MSG_WTP_GET_DEVICE_INFO_RSP, msgSeq, ONT_SUCCESS, "device infos rsp");

	get_device_info_json(json);

	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("rsp: %s", jsonValue);
	err = ont_edp_send_transdata(dev, SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		err = ONT_RSP_ERR;
	}

	ont_platform_free(jsonValue);
	cJSON_Delete(json);

	return err;
}


/*
 * name: get device info req
 * desc: 平台请求获取终端设备基本信息
 * direction: WTP <---- AMNMP/APP
 * other: 
*/
int ont_wtp_get_device_info_req(ont_device_t *dev, const char *data, size_t data_len)
{
	SL_DEBUG("check data: %s", data);

	cJSON *json = NULL;

	json = cJSON_Parse(data);
	if (!json) return ONT_JSON_ERR;
	
	ONT_CHECK_JSON(json) //failed
	{
		SL_DEBUG("cJSON format error");
		cJSON_Delete(json);
		return ONT_COMMON_ERR;
	}
	else
	{
		ont_wtp_get_device_info_rsp(dev, ONT_GET_MSGSEQ(json));
	}

	cJSON_Delete(json);

	return ONT_CMD_OK;
}
#endif

void Wireless_on(void){

		do_system("insmod -q rt2860v2_ap");
	#if defined (RT2860_MBSS_SUPPORT) || defined (RTDEV_MBSS_SUPPORT)
		int idx = 0;
		int bssid_num = strtol(nvram_bufget(nvram, "BssidNum"), NULL, 10);
		do {
			do_system("ifconfig ra%d up", idx);
			idx++;
		} while (idx < bssid_num);
	#else
		do_system("ifconfig ra0 up");
	#endif

	nvram_bufset(RT2860_NVRAM, "WiFiOff", "0");
    nvram_commit(RT2860_NVRAM);
    do_system("init_system restart");
	return;

}


void Wireless_off(void){
	#if defined (RT2860_MBSS_SUPPORT) || defined (RTDEV_MBSS_SUPPORT)
		int bssid_num = strtol(nvram_bufget(nvram, "BssidNum"), NULL, 10);
		do {
			bssid_num--;
			do_system("ifconfig ra%d down", bssid_num);
		} while (bssid_num > 0);
	#else
		do_system("ifconfig ra0 down");
	#endif
	
	nvram_bufset(RT2860_NVRAM, "WiFiOff", "1");
    nvram_commit(RT2860_NVRAM);
    do_system("init_system restart");
	return;
		
}


void Load_default(void)
{
	do_system("ralink_init clear 2860");
#if defined CONFIG_LAN_WAN_SUPPORT || defined CONFIG_MAC_TO_MAC_MODE
	do_system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_vlan");
#elif defined CONFIG_ICPLUS_PHY
	do_system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_oneport");
#else
	do_system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_novlan");
#endif
#if defined CONFIG_RTDEV
	do_system("ralink_init clear rtdev");
	do_system("ralink_init renew rtdev /etc_ro/Wireless/iNIC/RT2860AP.dat");
#endif
#if defined CONFIG_MTK_VOIP
	system("ralink_init clear voip");
	system("ralink_init renew voip /etc_ro/voip_default_settings");
#endif
	do_system("reboot");
}

void wifi_accelerate(void){
	nvram_bufset(RT2860_NVRAM, "AutoChannelSelect", "1");
	nvram_commit(RT2860_NVRAM);
	
	do_system("init_system restart");
}


int set_wifi_info(int nvram_id, char *ssid, char *code, char *type)
{
	char base64_target[512] = {0};
    char plainText[64] = {0};
	char sSecurityMode[32] = {0};
    char sCodeName[10] = {0};
	int index  =1;

	//不考虑多SSID的情况
	nvram_bufset(nvram_id, "SSID1", ssid);
	
	if (strlen(code))
	{		
        base64_decode(code, base64_target, sizeof(base64_target));
        SL_DEBUG("ont base64 decode password: %s", base64_target);
        unsigned char key[17] = {0};
        ont_platform_snprintf(key, sizeof(key), ONT_KEY);
        ont_aes128_ecb_pkcs5_decrypt(key, base64_target, plainText);
        SL_DEBUG("ont decrypt password: %s", plainText);
	}

	//1:无密码 2:WEP 3:WPA-PSK 4:WPA-TKIP 5:WAP2-PSK 6:WPA2-AES 7:WAP-PSK+WAP2-PSK 8:WAP-TKIP+WAP2-PSK
	//9:WAP-PSK+WAP2-AES 10:WAP-TKIP+WAP2-AES 16:WAPI
    int nType = atoi(type);
    if (nType ==1) {
        ; //Do Nothing
    }  else if(nType == 3) {
        strcpy(sSecurityMode, "WPAPSK");
    } else if (nType == 5) {
        strcpy(sSecurityMode, "WPA2PSK");
    } else {
        strcpy(sSecurityMode, "WPAPSKWPA2PSK");
    }
    
    if (nType == 1) {
        set_nth_value_flash(nvram_id, index -1, "AuthMode", "OPEN");
        set_nth_value_flash(nvram_id, index -1, "EncrypType", "NONE");
    }  else {
        set_nth_value_flash(nvram_id, index -1, "AuthMode", sSecurityMode);
		
        //2. Pwd
        sprintf(sCodeName, "WPAPSK%d", index);
        nvram_bufset(nvram_id,  sCodeName, plainText);

		//3. Set Other Configuration Default Value
		set_nth_value_flash(nvram_id, index -1, "RekeyMethod", "TIME");
		set_nth_value_flash(nvram_id, index -1, "IEEE8021X", "0");
		set_nth_value_flash(nvram_id, index -1, "DefaultKeyID", "2");
		set_nth_value_flash(nvram_id, index -1, "EncrypType", "AES");
		set_nth_value_flash(nvram_id, index -1, "RekeyInterval", "3600");
    }

	return ONT_CMD_OK;
}

/*
 * name: get wifi inf rsp
 * desc: 终端响应请求向平台或者APP返回WiFi基本信息
 * direction: WTP ----> AMNMP/APP
 * other: 
*/
static int ont_wtp_get_device_wifi_info_rsp(ont_device_t *dev, char *msgSeq)
{
	int err = ONT_CMD_OK;
	cJSON *json = NULL, *array = NULL, *obj =NULL;
	char *jsonValue = NULL;
    int Index =0;
    char *ssid = NULL, *pAuthMode = NULL, *pPassword = NULL;
    char sTemp[32] = {0};
    char sAuthMode[2] = {0};
    char real_chiper[256] = {0};
    char base64_target[512] = {0};
    char *key = ONT_KEY;
    char sWifiStatus[4] = {0};

	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	addRsphead(json, VERSION_N, MSG_WTP_GET_DEVICE_WIFI_INFO_NEW_RSP, msgSeq, ONT_SUCCESS, "WiFi infos rsp");

    router_getWifiStatus(RT2860_NVRAM, sWifiStatus, sizeof(sWifiStatus) -1);
	cJSON_AddItemToObject(json, "WifiStatus", cJSON_CreateString(sWifiStatus));
#ifdef WIRELESS_5G
    router_getWifiStatus(RTDEV_NVRAM, sWifiStatus, sizeof(sWifiStatus) -1);
	cJSON_AddItemToObject(json, "5GWifiStatus", cJSON_CreateString(sWifiStatus));
#else
	cJSON_AddItemToObject(json, "5GWifiStatus", cJSON_CreateString("0"));
#endif
    cJSON_AddItemToObject(json, "SSIDList", array = cJSON_CreateArray());

	/* 2.4G wireless */
    for(Index =1; Index <= MAX_SSID_NUM; Index++) {   //Support 4 SSIDS     
        
        memset(sTemp, 0, sizeof(sTemp));
        sprintf(sTemp, "SSID%d", Index);
        ssid = nvram_get(RT2860_NVRAM, sTemp);
        if (strlen(ssid) > 0) {
            
            cJSON_AddItemToObject(array, "", obj = cJSON_CreateObject());         
            cJSON_AddItemToObject(obj, "SSID", cJSON_CreateString(ssid));
            //1. Get CODE             
            memset(sTemp, 0x0, sizeof(sTemp));
            memset(real_chiper, 0x0, sizeof(real_chiper));
            memset(base64_target, 0x0, sizeof(base64_target));
            sprintf(sTemp, "WPAPSK%d", Index);
            pPassword = nvram_get(RT2860_NVRAM, sTemp);
            SL_DEBUG("ont plain password: %s", pPassword);
    		ont_aes128_ecb_pkcs5_encrypt(key, pPassword, real_chiper);
    		SL_DEBUG("ont encode password: %s", real_chiper);
    		base64_encode(real_chiper, strlen(real_chiper), base64_target, sizeof(base64_target) -1);
    		SL_DEBUG("ont base64 password: %s", base64_target);                            
            
            //2. Get AuthMode
            pAuthMode = nvram_get(RT2860_NVRAM, "AuthMode");            
            memset(sTemp, 0x0, sizeof(sTemp));
            memset(sAuthMode, 0x0, sizeof(sAuthMode));
            //Split the AuthMode like: WPAPSKWPA2PSK;WPAPSKWPA2PSK;OPEN;OPEN
            router_getSplitValue(pAuthMode, ";", Index, sTemp);
            if (!strcmp(sTemp, "WPAPSK")) {
                sAuthMode[0] = '3';
            } else if (!strcmp(sTemp, "WPA2PSK")) {
                sAuthMode[0] = '5';
            } else if (!strcmp(sTemp, "WPAPSKWPA2PSK")) {
                sAuthMode[0] = '7';
            } else {
                sAuthMode[0] = '1';
				memset(base64_target, 0, sizeof(base64_target));
            }

            cJSON_AddItemToObject(obj, "CODE", cJSON_CreateString(base64_target));
            cJSON_AddItemToObject(obj, "encryptType", cJSON_CreateString(sAuthMode));
            cJSON_AddItemToObject(obj, "frequency", cJSON_CreateString("0"));
            
        }
    }

#ifdef WIRELESS_5G
	/* 5G wireles */
    for(Index =1; Index <= MAX_SSID_NUM; Index++) {   //Support 4 SSIDS     
        
        memset(sTemp, 0, sizeof(sTemp));
        sprintf(sTemp, "SSID%d", Index);
        ssid = nvram_get(RTDEV_NVRAM, sTemp);
        if (strlen(ssid) > 0) {
            cJSON_AddItemToObject(array, "", obj = cJSON_CreateObject());         
            cJSON_AddItemToObject(obj, "SSID", cJSON_CreateString(ssid));
            //1. Get CODE             
            memset(sTemp, 0x0, sizeof(sTemp));
            memset(real_chiper, 0x0, sizeof(real_chiper));
            memset(base64_target, 0x0, sizeof(base64_target));
            sprintf(sTemp, "WPAPSK%d", Index);
            pPassword = nvram_get(RTDEV_NVRAM, sTemp);
            SL_DEBUG("ont plain password: %s", pPassword);
    		ont_aes128_ecb_pkcs5_encrypt(key, pPassword, real_chiper);
    		SL_DEBUG("ont encode password: %s", real_chiper);
    		base64_encode(real_chiper, strlen(real_chiper), base64_target, sizeof(base64_target) -1);
    		SL_DEBUG("ont base64 password: %s", base64_target);                            
            
            //2. Get AuthMode
            pAuthMode = nvram_get(RTDEV_NVRAM, "AuthMode");            
            memset(sTemp, 0x0, sizeof(sTemp));
            memset(sAuthMode, 0x0, sizeof(sAuthMode));
            //Split the AuthMode like: WPAPSKWPA2PSK;WPAPSKWPA2PSK;OPEN;OPEN
            router_getSplitValue(pAuthMode, ";", Index, sTemp);
            if (!strcmp(sTemp, "WPAPSK")) {
                sAuthMode[0] = '3';
            } else if (!strcmp(sTemp, "WPA2PSK")) {
                sAuthMode[0] = '5';
            } else if (!strcmp(sTemp, "WPAPSKWPA2PSK")) {
                sAuthMode[0] = '7';
            } else {
                sAuthMode[0] = '1';
				memset(base64_target, 0, sizeof(base64_target));
            }

            cJSON_AddItemToObject(obj, "CODE", cJSON_CreateString(base64_target));
            cJSON_AddItemToObject(obj, "encryptType", cJSON_CreateString(sAuthMode));
            cJSON_AddItemToObject(obj, "frequency", cJSON_CreateString("1"));
            
        }
    }
#endif

	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("rsp: %s", jsonValue);
	err = ont_edp_send_transdata(dev, SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		err = ONT_RSP_ERR;
	}
	
	ont_platform_free(jsonValue);
	cJSON_Delete(json);

	return err;
}


/*
 * name: get wifi info req
 * desc: 平台或者APP请求获取WiFi基本信息
 * direction: WTP <---- AMNMP/APP
 * other: 
*/
int ont_wtp_get_device_wifi_info_req(ont_device_t *dev, const char *data, size_t data_len)
{
	SL_DEBUG("check data: %s", data);

	cJSON *json = NULL;

	json = cJSON_Parse(data);
	if (!json) return ONT_JSON_ERR;
	
	ONT_CHECK_JSON(json) //failed
	{
		SL_DEBUG("cJSON format error");
		cJSON_Delete(json);
		return ONT_COMMON_ERR;
	}
	else
	{
		ont_wtp_get_device_wifi_info_rsp(dev, ONT_GET_MSGSEQ(json));
	}

	cJSON_Delete(json);

	return ONT_CMD_OK;
}


/*
 * name: set wifi rsp
 * desc: 终端响应平台或者APP的WiFi设置请求，无可变消息。
 * direction: WTP ----> AMNMP/APP
 * other: 
*/
static int ont_wtp_set_device_wifi_rsp(ont_device_t *dev, char *msgSeq)
{
	int err = ONT_CMD_OK;
	cJSON *json = NULL;
	char *jsonValue = NULL;
	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	addRsphead(json, VERSION_N, MSG_WTP_SET_DEVICE_WIFI_NEW_RSP, msgSeq, ONT_SUCCESS, "set wifi rsp");

	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("rsp: %s", jsonValue);
	err = ont_edp_send_transdata(dev, SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		err = ONT_RSP_ERR;
	}
	
	ont_platform_free(jsonValue);
	cJSON_Delete(json);

	return err;
}


/*
 * name: set wifi req
 * desc: 平台或者APP发起设置WiFi的请求
 * direction: WTP <---- AMNMP/APP
 * other: 
*/
int ont_wtp_set_device_wifi_req(ont_device_t *dev, const char *data, size_t data_len)
{
	SL_DEBUG("check data: %s", data);

	cJSON *json =NULL, *arrayItem =NULL, *item =NULL;
	char *ssid =NULL, *code =NULL, *type =NULL, *wifiStatus =NULL, *frequency =NULL; 
    int i, size, nvram_id = RT2860_NVRAM; 

	json = cJSON_Parse(data);
	if (!json) return ONT_JSON_ERR;
	
	ONT_CHECK_JSON(json) //failed
	{
		SL_DEBUG("cJSON format error");
		cJSON_Delete(json);
		return ONT_COMMON_ERR;
	}
	else
	{
		//先不考虑多SSID的设置操作
        ONT_CHECK_ITEM(json, "SSIDList")
		{
            arrayItem = cJSON_GetObjectItem(json,"SSIDList");
            size = cJSON_GetArraySize(arrayItem);
            SL_DEBUG("SSIDList size:%d", size);	
            
            for(i = 0; i < size; i++) {
                item = cJSON_GetArrayItem(arrayItem, i);                
                if (!cJSON_HasObjectItem(item, "SSID")) {             
                    SL_DEBUG("cJSON item %d, SSID format error", i);
                    break;              
                }
                
				ssid = cJSON_GetObjectItem(item, "SSID")->valuestring;
				
				ONT_CHECK_ITEM(item, "CODE")
					code = cJSON_GetObjectItem(item, "CODE")->valuestring;
				ONT_CHECK_ITEM(item, "encryptType")
					type = cJSON_GetObjectItem(item, "encryptType")->valuestring;
				
#ifdef WIRELESS_5G
				ONT_CHECK_ITEM(item, "frequency")
					frequency = cJSON_GetObjectItem(item, "frequency")->valuestring;
				if (frequency == NULL || !strcmp(frequency, "0")) {
					nvram_id = RT2860_NVRAM;
				} else {
					nvram_id = RTDEV_NVRAM;
				}
#endif
				
				SL_DEBUG("SSID: %s, Index: %d, CODE: %s, encryptType: %s, nvram_id: %d", ssid, i,  code, type, nvram_id);
				
				//Set wifi info
				set_wifi_info(nvram_id, ssid, code, type);
				break;
			}
		}

		 //Wifi 开关状态处理
		ONT_CHECK_ITEM(json, "WifiStatus")
		{
			wifiStatus = cJSON_GetObjectItem(json, "WifiStatus")->valuestring;
			if (!strcmp(wifiStatus, "2")) { //Close Wifi
				do_system("ifconfig ra0 down");
				nvram_set(RT2860_NVRAM, "WiFiOff", "1");
			} else if (!strcmp(wifiStatus, "1")) { //Open Wifi
				do_system("ifconfig ra0 up");
				nvram_set(RT2860_NVRAM, "WiFiOff", "0");
			} else {
				//重新启动 Wifi 统一处理
			}
		 }

#ifdef WIRELESS_5G
		ONT_CHECK_ITEM(json, "5GWifiStatus")
		{
			wifiStatus = cJSON_GetObjectItem(json, "5GWifiStatus")->valuestring;
			if (!strcmp(wifiStatus, "2")) { //Close Wifi
				do_system("ifconfig rai0 down");
				nvram_set(RTDEV_NVRAM, "WiFiOff", "1");
			} else if (!strcmp(wifiStatus, "1")) { //Open Wifi
				do_system("ifconfig rai0 up");
				nvram_set(RTDEV_NVRAM, "WiFiOff", "0");
			} else {
				//重新启动 Wifi 统一处理
			}
		}
#endif			 

	}
        
    ont_wtp_set_device_wifi_rsp(dev, ONT_GET_MSGSEQ(json));

    nvram_commit(RT2860_NVRAM);
    do_system("init_system restart");
	cJSON_Delete(json);
	return ONT_CMD_OK;
}


/*
 * name: wifi accelerate rsp
 * desc: 终端响应平台或者APP的WiFi加速请求，无可变消息。
 * direction: WTP ----> AMNMP/APP
 * other: 
*/
static int ont_wtp_wifi_accelerate_rsp(ont_device_t *dev, char *msgSeq)
{
	int err = ONT_CMD_OK;
	cJSON *json = NULL;
	char *jsonValue = NULL;
	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	addRsphead(json, VERSION_N, MSG_WTP_WIFI_ACCELERATE_RSP, msgSeq, ONT_SUCCESS, "wifi accelerate rsp");

	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("rsp: %s", jsonValue);
	err = ont_edp_send_transdata(dev, SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		err = ONT_RSP_ERR;
	}
	
	ont_platform_free(jsonValue);
	cJSON_Delete(json);

	return err;
}


/*
 * name: wifi accelerate req
 * desc: 平台或者APP请求WIFI加速。无可变消息。
 * direction: WTP <---- AMNMP/APP
 * other: 
*/
int ont_wtp_wifi_accelerate_req(ont_device_t *dev, const char *data, size_t data_len)
{
	SL_DEBUG("check data: %s", data);

	cJSON *json = NULL;

	json = cJSON_Parse(data);
	if (!json) return ONT_JSON_ERR;
	
	ONT_CHECK_JSON(json) //failed
	{
		SL_DEBUG("cJSON format error");
		cJSON_Delete(json);
		return ONT_COMMON_ERR;
	}
	else
	{
		wifi_accelerate();
		ont_wtp_wifi_accelerate_rsp(dev, ONT_GET_MSGSEQ(json));
	}

	cJSON_Delete(json);

	return ONT_CMD_OK;
}

/*
 * name: device operate rsp
 * desc: 终端响应平台或者APP发起的重启等操作。无可变消息
 * direction: WTP ----> AMNMP/APP
 * other: 
*/
static int ont_wtp_device_operate_rsp(ont_device_t *dev, char *msgSeq, char *errnum)
{
	int err = ONT_CMD_OK;
	cJSON *json = NULL;
	char *jsonValue = NULL;
	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	addRsphead(json, VERSION_N, MSG_WTP_DEVICE_OPERATE_RSP, msgSeq, errnum, "device operate rsp");

	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("rsp: %s", jsonValue);
	err = ont_edp_send_transdata(dev, SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		err = ONT_RSP_ERR;
	}
	
	ont_platform_free(jsonValue);
	cJSON_Delete(json);

	return err;
}

/*
 * name: device operate req
 * desc: 平台或者APP向终端发起重启等操作
 * direction: WTP <---- AMNMP/APP
 * other:
*/
int ont_wtp_device_operate_req(ont_device_t *dev, const char *data, size_t data_len)
{
	SL_DEBUG("check data: %s", data);

	cJSON *json = NULL;
	char *opType = NULL;
	char *wifiSwitch = NULL;
	char *firewallSwitch = NULL;

	json = cJSON_Parse(data);
	if (!json) return ONT_JSON_ERR;
	ONT_CHECK_JSON(json) //failed
	{
		SL_DEBUG("cJSON format error");
		cJSON_Delete(json);
		return ONT_COMMON_ERR;
	}
	else
	{
		if (ONT_ERR_OK == ont_wtp_device_operate_rsp(dev, ONT_GET_MSGSEQ(json), ONT_SUCCESS))
		{
			opType = cJSON_GetObjectItem(json, "opType")->valuestring;

			switch(atoi(opType))
			{
				/*WiFi reboot*/
				case 1:
					SL_DEBUG("wifi reboot");
					ont_wifi_reboot(dev);
					break;

				/*factory reset*/
				case 3:
					SL_DEBUG("ont factory reset");
					Load_default();
					/* Should reboot after reset */
					/* !NO BREAK!!*/
					//break;

				/*device reboot*/
				case 2:
					SL_DEBUG("ont reboot");
					ont_sys_reboot(dev);
					break;

				/*AP WiFi switch*/
				case 4:
					SL_DEBUG("ont wifi ON/OFF");
					if (cJSON_HasObjectItem(json, "opPara"))
					{
						/*1:on, 2:off*/
						wifiSwitch = cJSON_GetObjectItem(json, "opPara")->valuestring;
						if (1 == atoi(wifiSwitch))
						{
							Wireless_on();
						}
						else if (2 == atoi(wifiSwitch))
						{
							Wireless_off();
						}
					}
					else
						return ONT_COMMON_ERR;

					break;

				/*WiFi accelarate*/
				case 5:
					SL_DEBUG("wifi accelarate");
					wifi_accelerate();
						
					break;

				/*firewall ON/OFF*/
				case 6:
					SL_DEBUG("firewall ON/OFF");
					if (cJSON_HasObjectItem(json, "opPara"))
					{
						/*1:on, 2:off*/
						firewallSwitch = cJSON_GetObjectItem(json, "opPara")->valuestring;
						SL_DEBUG("firewallSwitch value [%s]", firewallSwitch);
						if (!strcmp(firewallSwitch, "1") || !strcmp(firewallSwitch, "2")) {
							router_firewallSetting(firewallSwitch);
						}
					}
					else
						return ONT_COMMON_ERR;

					break;

				default:
					SL_DEBUG("what do you want to do?");
					break;
			}
		}
	}

	cJSON_Delete(json);

	return ONT_CMD_OK;
}


void ont_wifi_reboot(ont_device_t *dev){

	#if defined (RT2860_MBSS_SUPPORT) || defined (RTDEV_MBSS_SUPPORT)
		int bssid_num = strtol(nvram_bufget(nvram, "BssidNum"), NULL, 10);
		do {
			bssid_num--;
			do_system("ifconfig ra%d down", bssid_num);
		} while (bssid_num > 0);
	#else
		do_system("ifconfig ra0 down");
	#endif

	ont_platform_sleep(5000);

	#if defined (RT2860_MBSS_SUPPORT) || defined (RTDEV_MBSS_SUPPORT)
		int idx = 0;
		int bssid_num = strtol(nvram_bufget(nvram, "BssidNum"), NULL, 10);
		do {
			do_system("ifconfig ra%d up", idx);
			idx++;
		} while (idx < bssid_num);
	#else
		do_system("ifconfig ra0 up");
	#endif


	#if 0
	system("ifconfig ra0 down");
	ont_platform_sleep(5000);
	system("ifconfig ra0 up");
	#endif
}

void ont_sys_reboot(ont_device_t *dev)
{
	const ont_device_rt_t *rt;
	rt = ont_device_get_rt(dev->type);

	/* should disconnect before reboot */
	ont_platform_sleep(500);
	rt->destroy(dev);
	ont_platform_sleep(1000);
	/* ugly */
	system("reboot");

	return ;
}


/*CMD test*/
int ont_cmd_test(void)
{
	SL_DEBUG();

	return 0;
}


int ont_cmd_factory_reset(void)
{
	SL_DEBUG();
    // TODO:  factory reset
    
	//CFG_init_prof();
	//CFG_save(0);
	//diag_printf("ONT!!, Reset the default configuration to flash!\n"); 

	return 0;
}

int ont_cmd_reboot(void)
{
	SL_DEBUG();
	ont_platform_sleep(100);
	reboot(RB_AUTOBOOT);

	return 0;
}


int ont_wtp_get_route_overview_req(ont_device_t *dev, const char *data, size_t data_len)
{
	SL_DEBUG("check data: %s", data);

	cJSON *json = NULL;

	json = cJSON_Parse(data);
	if (!json) {
        responseRequestParseError(dev);
        return ONT_JSON_ERR;
    }
    
	ONT_CHECK_JSON(json) //failed
	{
		SL_DEBUG("cJSON format error");
		cJSON_Delete(json);
        responseRequestParseError(dev);
		return ONT_COMMON_ERR;
	}

	ont_wtp_get_route_overview_rsp(dev, ONT_GET_MSGSEQ(json));
	cJSON_Delete(json);
	return ONT_CMD_OK;
}

int ont_wtp_get_route_overview_rsp(ont_device_t *dev, char *msgSeq)
{
	char sDeviceName[64] = {0};
	char sWifiChannel[4] = {0};
	char sOnlineTime[32] = {0};
	char sFirewareStatus[2] = {0};
	char sSSID[32] = {0};
	char sWifiStatus[4] = {0};
#ifdef WIRELESS_5G
	char sWifi5GStatus[4] = {0};
	cJSON *array_5g = NULL;
#endif
	char wanIp[20] = {0};

	int err = ONT_CMD_OK;
	cJSON *json = NULL,  *array = NULL, *dhcp_client = NULL;
	char *jsonValue = NULL;
	
	json = cJSON_CreateObject();
	if (!json){
        responseSystemError(dev, MSG_WTP_GET_ROUTE_OVERVIEW_RSP, msgSeq);
		SL_DEBUG("create json object error!");
		return ONT_JSON_ERR;
	}
	addRsphead(json, VERSION_N, MSG_WTP_GET_ROUTE_OVERVIEW_RSP, msgSeq, ONT_SUCCESS, "router overview resp");

	//1. get router base info
	router_getWifiChannel(sWifiChannel, sizeof(sWifiChannel)-1);
	router_getDeviceName(sDeviceName, sizeof(sDeviceName)-1);	
	router_getOnlineDuration(sOnlineTime);
	router_getLinkSSID(sSSID, sizeof(sSSID)-1);
	router_getWifiStatus(RT2860_NVRAM, sWifiStatus, sizeof(sWifiStatus) -1);
#ifdef WIRELESS_5G
	//TODO should use the new nvram of 5G wifi off
	router_getWifiStatus(RTDEV_NVRAM, sWifi5GStatus, sizeof(sWifi5GStatus) -1);
#endif

	router_getFilewareStatus(sFirewareStatus, sizeof(sFirewareStatus)-1);

	//2. package in json
	router_getWanIpAddr(wanIp, sizeof(wanIp) -1);
	GET_DEV_TYPE(devType, g_sn); //设备型号
	cJSON_AddItemToObject(json, "type", cJSON_CreateString(devType));
	cJSON_AddItemToObject(json, "MAC", cJSON_CreateString(g_mac));
	cJSON_AddItemToObject(json, "IP", cJSON_CreateString(wanIp));
	cJSON_AddItemToObject(json, "channel", cJSON_CreateString(sWifiChannel));
	cJSON_AddItemToObject(json, "softwareVersion", cJSON_CreateString(fw_version));
	cJSON_AddItemToObject(json, "onlineDuration", cJSON_CreateString(sOnlineTime));	
	char *upLinkSSID = (char *)nvram_bufget(RT2860_NVRAM, "ApCliSsid");
	cJSON_AddItemToObject(json, "upLinkSSID", cJSON_CreateString(upLinkSSID));
    //char upLinkSignal[2] = {0};
    //router_getUpLinkSignal(upLinkSSID, upLinkSignal);
	cJSON_AddItemToObject(json, "upLineSignal", cJSON_CreateString("2"));
	cJSON_AddItemToObject(json, "firewallStatus", cJSON_CreateString(sFirewareStatus));
	cJSON_AddItemToObject(json, "WifiStatus", cJSON_CreateString(sWifiStatus));
#ifdef WIRELESS_5G
	cJSON_AddItemToObject(json, "5GWifiStatus", cJSON_CreateString(sWifi5GStatus));
#else
	cJSON_AddItemToObject(json, "5GWifiStatus", cJSON_CreateString("0"));
#endif
	char upLinkRate[32]={0},downLinkRate[32]={0};
    router_getUpAndDownLinkRate(upLinkRate, downLinkRate);
	cJSON_AddItemToObject(json, "downlinkRate", cJSON_CreateString(downLinkRate)); //TODO
	cJSON_AddItemToObject(json, "uplinkRate", cJSON_CreateString(upLinkRate)); //TODO
	cJSON_AddItemToObject(json, "connectDevices", array = cJSON_CreateArray());
	struct RouterClientList clientHead;
	SLIST_INIT(&clientHead);

	router_getConnectDevices(&clientHead);
	
	recRouterClient *prClientNode;
	SLIST_FOREACH(prClientNode,&clientHead,next)
	{
		cJSON_AddItemToObject(array, "", dhcp_client= cJSON_CreateObject());
		cJSON_AddItemToObject(dhcp_client, "clientDownlinkRate", cJSON_CreateString(prClientNode->clientDownlinkRate)); //TODO
		cJSON_AddItemToObject(dhcp_client, "clientUplinkRate", cJSON_CreateString(prClientNode->clientUplinkRate)); //TODO  
		cJSON_AddItemToObject(dhcp_client, "deviceStatus", cJSON_CreateString("2")); //1: 连接. 2: 活跃
		cJSON_AddItemToObject(dhcp_client, "MAC", cJSON_CreateString(prClientNode->macAddr));
		cJSON_AddItemToObject(dhcp_client, "IP", cJSON_CreateString(prClientNode->ipAddr));
		cJSON_AddItemToObject(dhcp_client, "name", cJSON_CreateString(prClientNode->name));
		cJSON_AddItemToObject(dhcp_client, "onlineDuration", cJSON_CreateString(prClientNode->onlineDuration));
		cJSON_AddItemToObject(dhcp_client, "accessInternet", cJSON_CreateString(prClientNode->accessInernet));
		cJSON_AddItemToObject(dhcp_client, "qosSwitch", cJSON_CreateString(prClientNode->qosSwitch));     //TODO
		cJSON_AddItemToObject(dhcp_client, "maxUplinkRate", cJSON_CreateString(prClientNode->maxUplinkRate));  //TODO
		cJSON_AddItemToObject(dhcp_client, "maxDownlinkRate", cJSON_CreateString(prClientNode->maxDownlinkRate)); //TODO
		cJSON_AddItemToObject(dhcp_client, "SSID", cJSON_CreateString(sSSID));
		SL_DEBUG("+++++++zero_debug+++++macaddr=%s,qosSwitch=%s,upRate=%s,downRate=%s\n",prClientNode->macAddr,prClientNode->qosSwitch,
													prClientNode->maxUplinkRate,prClientNode->maxDownlinkRate);
		free(prClientNode);
	}

#ifdef WIRELESS_5G
	cJSON_AddItemToObject(json, "connect5GDevices", array_5g = cJSON_CreateArray());

	SLIST_INIT(&clientHead);
	router_getConnect_5G_Devices(&clientHead);
	
	SLIST_FOREACH(prClientNode,&clientHead,next)
	{
		cJSON_AddItemToObject(array_5g, "", dhcp_client= cJSON_CreateObject());
		cJSON_AddItemToObject(dhcp_client, "clientDownlinkRate", cJSON_CreateString(prClientNode->clientDownlinkRate)); //TODO
		cJSON_AddItemToObject(dhcp_client, "clientUplinkRate", cJSON_CreateString(prClientNode->clientUplinkRate)); //TODO  
		cJSON_AddItemToObject(dhcp_client, "deviceStatus", cJSON_CreateString("2")); //1: 连接. 2: 活跃
		cJSON_AddItemToObject(dhcp_client, "MAC", cJSON_CreateString(prClientNode->macAddr));
		cJSON_AddItemToObject(dhcp_client, "IP", cJSON_CreateString(prClientNode->ipAddr));
		cJSON_AddItemToObject(dhcp_client, "name", cJSON_CreateString(prClientNode->name));
		cJSON_AddItemToObject(dhcp_client, "onlineDuration", cJSON_CreateString(prClientNode->onlineDuration));
		cJSON_AddItemToObject(dhcp_client, "accessInternet", cJSON_CreateString(prClientNode->accessInernet));
		cJSON_AddItemToObject(dhcp_client, "qosSwitch", cJSON_CreateString(prClientNode->qosSwitch));     //TODO
		cJSON_AddItemToObject(dhcp_client, "maxUplinkRate", cJSON_CreateString(prClientNode->maxUplinkRate));  //TODO
		cJSON_AddItemToObject(dhcp_client, "maxDownlinkRate", cJSON_CreateString(prClientNode->maxDownlinkRate)); //TODO
		cJSON_AddItemToObject(dhcp_client, "SSID", cJSON_CreateString(sSSID));
		SL_DEBUG("+++++++zero_debug+++++macaddr=%s,qosSwitch=%s,upRate=%s,downRate=%s\n",prClientNode->macAddr,prClientNode->qosSwitch,
													prClientNode->maxUplinkRate,prClientNode->maxDownlinkRate);
		free(prClientNode);
	}
#endif

	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("rsp: %s", jsonValue);
	err = ont_edp_send_transdata(dev, SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		err = ONT_RSP_ERR;
	}

	ont_platform_free(jsonValue);
	cJSON_Delete(json);	
	return err;
}


/* 获取上网信息请求 **/
int ont_wtp_get_boardband_setting_req(ont_device_t *dev, const char *data, size_t data_len)
{	
	/***
		如果路由器的上网方式不是以下几种:
		1、PPPoE, 2、DHCP, 3、Static IP. 那么统一给用户应答为未设置上网方式， 由用户来在这三种
		上网方式中选择进行设置。
	**/

	SL_DEBUG("check data: %s", data);

	cJSON *json = NULL;

	json = cJSON_Parse(data);
	if (!json) return ONT_JSON_ERR;
	
	ONT_CHECK_JSON(json) //failed
	{
		SL_DEBUG("cJSON format error");
		cJSON_Delete(json);
		return ONT_COMMON_ERR;
	}

	ont_wtp_get_boardband_setting_rsp(dev, ONT_GET_MSGSEQ(json));
	cJSON_Delete(json);
	return ONT_CMD_OK;

}


/* 获取上网信息应答 **/
int ont_wtp_get_boardband_setting_rsp(ont_device_t *dev, char *msgSeq)
{

	int err = ONT_CMD_OK;
	cJSON *json = NULL;
	char *jsonValue = NULL;
	char sType[2] = {0};
	
	json = cJSON_CreateObject();
	if (!json){
		SL_DEBUG("create json object error!");
		return ONT_JSON_ERR;
	}
	addRsphead(json, VERSION_N, MSG_WTP_GET_BROADBAND_SETTING_RSP, 
						msgSeq, ONT_SUCCESS, "router get boardband resp");


	const char *pWanConnectMode = NULL;		
	pWanConnectMode = nvram_bufget(RT2860_NVRAM, "wanConnectionMode");	
	if (pWanConnectMode == NULL) {
		cJSON_Delete(json);
		SL_DEBUG("wan ConnectinMode format error");
		return ONT_COMMON_ERR;
	}
	
	if (!strncmp(pWanConnectMode, "PPPOE", 5)) {
		sType[0] = '1';
		const char *pPPPUserName = NULL, *pPPPUserPassword = NULL;
		pPPPUserName = nvram_bufget(RT2860_NVRAM, "wan_pppoe_user");
		pPPPUserPassword = nvram_bufget(RT2860_NVRAM, "wan_pppoe_pass");

		//encoded password
		char real_chiper[256] = {0};
		char base64_target[512] = {0};
		SL_DEBUG("ont plain password: %s", pPPPUserPassword);
		unsigned char key[17] = {0};
		ont_platform_snprintf(key, sizeof(key), ONT_KEY);
		ont_aes128_ecb_pkcs5_encrypt(key, pPPPUserPassword, real_chiper);
		SL_DEBUG("ont encode password: %s", real_chiper);
		base64_encode(real_chiper, strlen(real_chiper), base64_target, sizeof(base64_target) -1);
		SL_DEBUG("ont base64 password: %s", base64_target);

		cJSON_AddItemToObject(json, "PPPoEName", cJSON_CreateString(pPPPUserName));
		cJSON_AddItemToObject(json, "PPPoECode", cJSON_CreateString(base64_target));
		
	} else if (!strncmp(pWanConnectMode, "STATIC", 6)) {
		sType[0] = '4';
	
		char *pWanIP =NULL, *pSubMask = NULL;
		char sGateway[16] = {0};
		char sDns1[16] = {0}, sDns2[16]= {0};
		pWanIP = get_ipaddr(get_wanif_name());
		pSubMask = get_netmask(get_lanif_name());
		
		router_getGateway(sGateway, 16);
		router_getDns(1, sDns1, 16);
		router_getDns(2, sDns2, 16);

		cJSON_AddItemToObject(json, "IP", cJSON_CreateString(pWanIP));
		cJSON_AddItemToObject(json, "gatewayIP", cJSON_CreateString(sGateway));
		cJSON_AddItemToObject(json, "submask", cJSON_CreateString(pSubMask));
		cJSON_AddItemToObject(json, "DNS1", cJSON_CreateString(sDns1));
		cJSON_AddItemToObject(json, "DNS2", cJSON_CreateString(sDns2));
		
	}  else if (!strncmp(pWanConnectMode, "DHCP", 4)) {
		sType[0] = '2';
	} else {
		sType[0] = '0';
	}

	cJSON_AddItemToObject(json, "type", cJSON_CreateString(sType));
	
	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("rsp: %s", jsonValue);
	err = ont_edp_send_transdata(dev, SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		err = ONT_RSP_ERR;
	}

	ont_platform_free(jsonValue);
	cJSON_Delete(json);	
	return err;
}


int ont_set_boardband(recRouterBoardband *prBoardband)
{
	int nType = atoi(prBoardband->type);
	SL_DEBUG("prBoardband type [%d]", nType);
	if (nType == 1)
	{
		SL_DEBUG("PPPoEName [%s], PPPoECode[%s]." , prBoardband->PPPoEName, prBoardband->PPPoECode);
		nvram_bufset(RT2860_NVRAM, "wanConnectionMode", "PPPOE");
	    nvram_bufset(RT2860_NVRAM, "wan_pppoe_user", prBoardband->PPPoEName);
		nvram_bufset(RT2860_NVRAM, "wan_pppoe_pass", prBoardband->PPPoECode);

		//hemu pppoe use KeepAlive operation mode as default
		nvram_bufset(RT2860_NVRAM, "wan_pppoe_opmode", "KeepAlive");
		nvram_bufset(RT2860_NVRAM, "wan_pppoe_optime", "");
		
	} else if (nType == 4) {
		SL_DEBUG("wan_ipaddr[%s], wan_netmask[%s], wan_gateway[%s], wan_dns1[%s], wan_dns2[%s]" , 
					prBoardband->ipAddr,
					prBoardband->subMask,
					prBoardband->gateWayIP,
					prBoardband->dns1,
					prBoardband->dns2);
	    nvram_bufset(RT2860_NVRAM, "wanConnectionMode", "STATIC");
		nvram_bufset(RT2860_NVRAM, "wan_ipaddr", prBoardband->ipAddr);
		nvram_bufset(RT2860_NVRAM, "wan_netmask", prBoardband->subMask);
		nvram_bufset(RT2860_NVRAM, "wan_gateway", prBoardband->gateWayIP);
		nvram_bufset(RT2860_NVRAM, "wan_primary_dns", prBoardband->dns1);
		nvram_bufset(RT2860_NVRAM, "wan_secondary_dns", prBoardband->dns2);

	} else if (nType == 2) {
		nvram_bufset(RT2860_NVRAM, "wanConnectionMode", "DHCP");
	}
	
	nvram_commit(RT2860_NVRAM);
	do_system("init_system restart");

	return ONT_CMD_OK;

}

/* 设置上网信息请求 **/
int ont_wtp_set_boardband_setting_req(ont_device_t *dev, const char *data, size_t data_len)
{
	SL_DEBUG("check data: %s", data);
    
    char base64_target[512] = {0};
	cJSON  *json = NULL;
	char *pPassword = NULL;
	recRouterBoardband rBoardband;
	memset(&rBoardband, 0, sizeof(recRouterBoardband));

	json = cJSON_Parse(data);
	if (!json) return ONT_JSON_ERR;
	
	ONT_CHECK_JSON(json) //failed
	{
		SL_DEBUG("cJSON format error");
		cJSON_Delete(json);
		return ONT_COMMON_ERR;
	}
	else
	{
		if (!cJSON_HasObjectItem(json, "type")) {
			cJSON_Delete(json);
			SL_DEBUG("cJSON item type ont exists");
			return ONT_JSON_ERR;
		}
		
	    strcpy(rBoardband.type, cJSON_GetObjectItem(json, "type")->valuestring);
        
		int nType = atoi(rBoardband.type);
		if (nType != 1 && nType != 2 && nType != 4){
			cJSON_Delete(json);
			SL_DEBUG("cJSON item type [%d] value invalid", nType);
			return ONT_JSON_ERR;
		}
		
		if (nType == 1) {
			ONT_CHECK_ITEM(json, "PPPoEName")
				 strcpy(rBoardband.PPPoEName, cJSON_GetObjectItem(json, "PPPoEName")->valuestring);

			ONT_CHECK_ITEM(json, "PPPoECode")
			{
				//Base64Decode and AES Decrypt								
				pPassword = cJSON_GetObjectItem(json, "PPPoECode")->valuestring;
				base64_decode(pPassword, base64_target, sizeof(base64_target));
				SL_DEBUG("ont decode password: %s", base64_target);
				unsigned char key[17] = {0};
				ont_platform_snprintf(key, sizeof(key), ONT_KEY);
				ont_aes128_ecb_pkcs5_decrypt(key, base64_target, rBoardband.PPPoECode);
				SL_DEBUG("ont decrypt password: %s", rBoardband.PPPoECode);
			}
		} else if (nType == 4) {
			ONT_CHECK_ITEM(json, "IP")
				strcpy(rBoardband.ipAddr, cJSON_GetObjectItem(json, "IP")->valuestring);

			ONT_CHECK_ITEM(json, "gatewayIP")
				strcpy(rBoardband.gateWayIP, cJSON_GetObjectItem(json, "gatewayIP")->valuestring);

			ONT_CHECK_ITEM(json, "submask")
				strcpy(rBoardband.subMask, cJSON_GetObjectItem(json, "submask")->valuestring);

			ONT_CHECK_ITEM(json, "DNS1")
				strcpy(rBoardband.dns1, cJSON_GetObjectItem(json, "DNS1")->valuestring);

			ONT_CHECK_ITEM(json, "DNS2")
				strcpy(rBoardband.dns2, cJSON_GetObjectItem(json, "DNS2")->valuestring);
		}

		//set boardband information
		ont_set_boardband(&rBoardband);
	
		ont_wtp_set_boardband_setting_rsp(dev, ONT_GET_MSGSEQ(json));
	}

	cJSON_Delete(json);
	return ONT_CMD_OK;

}


/* 设置上网信息应答 **/
int ont_wtp_set_boardband_setting_rsp(ont_device_t *dev, char *msgSeq)
{
	int err = ONT_CMD_OK;
	cJSON *json = NULL;
	char *jsonValue = NULL;
	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	addRsphead(json, VERSION_N, MSG_WTP_SET_BROADBAND_SETTING_RSP, msgSeq, ONT_SUCCESS, "set boardband rsp");

	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("rsp: %s", jsonValue);
	err = ont_edp_send_transdata(dev, SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		err = ONT_RSP_ERR;
	}
	
	ont_platform_free(jsonValue);
	cJSON_Delete(json);

	return err;
}

int ont_wtp_get_base_info_req(ont_device_t *dev, const char *data, size_t data_len) 
{
	SL_DEBUG("check data: %s", data);

	cJSON *json = NULL;

	json = cJSON_Parse(data);
	if (!json) return ONT_JSON_ERR;
	
	ONT_CHECK_JSON(json) //failed
	{
		SL_DEBUG("cJSON format error");
		cJSON_Delete(json);
		return ONT_COMMON_ERR;
	}

	ont_wtp_get_base_info_rsp(dev, ONT_GET_MSGSEQ(json));
	cJSON_Delete(json);
	return ONT_CMD_OK;

}


int ont_wtp_get_base_info_rsp(ont_device_t *dev, char *msgSeq)
{
	int err = ONT_CMD_OK;
	cJSON *json = NULL;
	char *jsonValue = NULL;
	char wanIp[20] = {0};
	
	json = cJSON_CreateObject();
	if (!json){
		SL_DEBUG("create json object error!");
		return ONT_JSON_ERR;
	}
	addRsphead(json, VERSION_N, MSG_WTP_GET_ROUTE_BASE_INFO_RSP, msgSeq, ONT_SUCCESS, "route base info resp");

	router_getWanIpAddr(wanIp, sizeof(wanIp) -1);
	char *lanIp = get_ipaddr(get_lanif_name());
	char *wanMAC = get_macaddr(get_wanif_name());
	//2. package in json
	cJSON_AddItemToObject(json, "deviceID", cJSON_CreateString(g_sn));
	cJSON_AddItemToObject(json, "wanMAC", cJSON_CreateString(wanMAC));
	cJSON_AddItemToObject(json, "wanIP", cJSON_CreateString(wanIp));
	cJSON_AddItemToObject(json, "lanIP", cJSON_CreateString(lanIp));
	
	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("rsp: %s", jsonValue);
	err = ont_edp_send_transdata(dev, SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		err = ONT_RSP_ERR;
	}

	ont_platform_free(jsonValue);
	cJSON_Delete(json);	
	return err;
}


int ont_wtp_get_access_control_info_req(ont_device_t *dev, const char *data, size_t data_len)
{
    SL_DEBUG("check data: %s", data);
	cJSON *json = NULL;

	json = cJSON_Parse(data);
	if (!json) return ONT_JSON_ERR;
	
	ONT_CHECK_JSON(json) //failed
	{
		SL_DEBUG("cJSON format error");
		cJSON_Delete(json);
		return ONT_COMMON_ERR;
	}

	ont_wtp_get_access_control_info_rsp(dev, ONT_GET_MSGSEQ(json));
	cJSON_Delete(json);
	return ONT_CMD_OK;
    
}


int ont_wtp_get_access_control_info_rsp(ont_device_t *dev, char *msgSeq)
{
	int i, accessFlag;
	int err = ONT_CMD_OK;
	cJSON *json = NULL, *array =NULL, *obj =NULL;
	char *jsonValue = NULL, *pTemp = NULL;
    char macAddr[32] ={0};
    char ipAddr[20] = {0};
    char accessRight[2] = {0};
    struct in_addr addr;
    
    struct dhcpOfferedAddr {
		unsigned char hostname[16];
		unsigned char mac[16];
		unsigned long ip;
		unsigned long expires;
	} lease;
    
	
	json = cJSON_CreateObject();
	if (!json){
		SL_DEBUG("create json object error!");
		return ONT_JSON_ERR;
	}
    
	addRsphead(json, VERSION_N, MSG_WTP_GET_CLIENT_ACCESS_CONTROL_INFO_RSP, msgSeq, ONT_SUCCESS, "wtp access control resp");

    cJSON_AddItemToObject(json, "clientACs", array = cJSON_CreateArray());

    //Get DHCP Users
	do_system("killall -q -USR1 udhcpd");
	FILE *fp = fopen("/var/udhcpd.leases", "r");
	if (NULL == fp) {
		SL_DEBUG("dhcp file not exists.");
        cJSON_Delete(json);
		return -1;
	}
    
	while (fread(&lease, 1, sizeof(lease), fp) == sizeof(lease)) {
                
         //1. get macAddr
         memset(macAddr, 0, sizeof(macAddr));
         pTemp = macAddr;
         sprintf(pTemp, "%02X", lease.mac[0]);
         pTemp += 2;         
         for (i = 1; i < 6; i++) {
             sprintf(pTemp, ":%02X", lease.mac[i]);
             pTemp += 3;
         }

         //2. get ip Addr
         memset(ipAddr, 0, sizeof(ipAddr));
         addr.s_addr = lease.ip;
         strcpy(ipAddr, inet_ntoa(addr));

         //3. get access Right
         memset(accessRight, 0, sizeof(accessRight));
         accessFlag = router_getClientAccessRight(macAddr);
         sprintf(accessRight, "%d", accessFlag);

         cJSON_AddItemToObject(array, "", obj= cJSON_CreateObject());
         cJSON_AddItemToObject(obj, "clientMAC", cJSON_CreateString(macAddr ));
         cJSON_AddItemToObject(obj, "clientIP", cJSON_CreateString(ipAddr));
         cJSON_AddItemToObject(obj, "clientName", cJSON_CreateString(lease.hostname));
         cJSON_AddItemToObject(obj, "accessRight", cJSON_CreateString(accessRight));

    }

	fclose(fp);
    
	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("rsp: %s", jsonValue);
	err = ont_edp_send_transdata(dev, SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		err = ONT_RSP_ERR;
	}

	ont_platform_free(jsonValue);
	cJSON_Delete(json);	
	return err;
}


int ont_wtp_set_acccess_control_req(ont_device_t *dev, const char *data, size_t data_len)
{
	SL_DEBUG("check data: %s", data);
    cJSON *json = NULL;
    char *clientMAC = NULL, *accessRight = NULL;
    int accessFlag = 0;

	json = cJSON_Parse(data);
	if (!json) return ONT_JSON_ERR;
	
	ONT_CHECK_JSON(json) //failed
	{
		SL_DEBUG("cJSON format error");
		cJSON_Delete(json);
		return ONT_COMMON_ERR;
	}
	else
	{
		if (!cJSON_HasObjectItem(json, "clientMAC") || !cJSON_HasObjectItem(json, "accessRight")) {
			cJSON_Delete(json);
			SL_DEBUG("cJSON don't has clientMAC/accessRight item.");
			return ONT_JSON_ERR;
		}

        clientMAC = cJSON_GetObjectItem(json, "clientMAC")->valuestring;
        accessRight = cJSON_GetObjectItem(json, "accessRight")->valuestring;
        accessFlag = atoi(accessRight);
        SL_DEBUG("clientMAC :%s, accessRight:%d", clientMAC, accessFlag);
        if (strlen(clientMAC) == 0 || (accessFlag != 1 && accessFlag !=2)) {
            cJSON_Delete(json);
			SL_DEBUG("clientMAC/accessRight value invalid.");
			return ONT_JSON_ERR;
        }

        //set access Right
		router_setClientAccessRight(clientMAC, accessFlag);
        
		ont_wtp_set_acccess_control_rsp(dev, ONT_GET_MSGSEQ(json));
	}

	cJSON_Delete(json);
	return ONT_CMD_OK;

}


int ont_wtp_set_acccess_control_rsp(ont_device_t *dev, char *msgSeq)
{
	int err = ONT_CMD_OK;
	cJSON *json = NULL;
	char *jsonValue = NULL;
	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	addRsphead(json, VERSION_N, MSG_WTP_SET_CLIENT_ACCESS_CONTORL_RSP, msgSeq, ONT_SUCCESS, "set access control rsp");

	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("rsp: %s", jsonValue);
	err = ont_edp_send_transdata(dev, SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		err = ONT_RSP_ERR;
	}
	
	ont_platform_free(jsonValue);
	cJSON_Delete(json);

	return err;
}

int ont_wtp_set_qos_req(ont_device_t *dev, const char *data, size_t data_len)
{
	SL_DEBUG("check data: %s", data);
	cJSON *json = NULL;
	char *clientMAC = NULL, *qosSwitch = NULL;
	char *maxUplinkRate = NULL,*maxDownlinkRate = NULL;
	int qosSwitchFlag = 0;

	json = cJSON_Parse(data);
	if (!json) return ONT_JSON_ERR;
	
	ONT_CHECK_JSON(json) //failed
	{
		SL_DEBUG("cJSON format error");
		cJSON_Delete(json);
		return ONT_COMMON_ERR;
	}
	else
	{
		if (!cJSON_HasObjectItem(json, "clientMAC") || !cJSON_HasObjectItem(json, "qosSwitch")) {
			cJSON_Delete(json);
			SL_DEBUG("cJSON don't has clientMAC/qosSwitch item.");
			return ONT_JSON_ERR;
		}

		clientMAC = cJSON_GetObjectItem(json, "clientMAC")->valuestring;
		qosSwitch = cJSON_GetObjectItem(json, "qosSwitch")->valuestring;
		maxUplinkRate = cJSON_GetObjectItem(json, "maxUplinkRate")->valuestring;//单位是KB/s
		maxDownlinkRate = cJSON_GetObjectItem(json, "maxDownlinkRate")->valuestring;////单位是KB/s
		qosSwitchFlag = atoi(qosSwitch);
		
		SL_DEBUG("clientMAC :%s, accessRight:%d,maxUplinkRate:%s,maxDownlinkRate:%s",clientMAC, qosSwitchFlag,maxUplinkRate,maxDownlinkRate);
		if (strlen(clientMAC) == 0 || (qosSwitchFlag != 0 && qosSwitchFlag !=1) )
		{
			cJSON_Delete(json);
			SL_DEBUG("clientMAC/qosSwitchFlag value invalid.");
			return ONT_JSON_ERR;
		}

		//set access Right
		router_setQosRule(clientMAC, qosSwitchFlag,maxUplinkRate,maxDownlinkRate);
				
		ont_wtp_set_qos_rsp(dev, ONT_GET_MSGSEQ(json));
	}

	cJSON_Delete(json);
	return ONT_CMD_OK;

}

int ont_wtp_set_qos_rsp(ont_device_t *dev, char *msgSeq)
{
	int err = ONT_CMD_OK;
	cJSON *json = NULL;
	char *jsonValue = NULL;
	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	addRsphead(json, VERSION_N, MSG_WTP_SET_QOS_RSP, msgSeq, ONT_SUCCESS, "set access control rsp");

	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("rsp: %s", jsonValue);
	err = ont_edp_send_transdata(dev, SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		err = ONT_RSP_ERR;
	}
	
	ont_platform_free(jsonValue);
	cJSON_Delete(json);

	return err;
}


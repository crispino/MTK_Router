/* andlink-resource
 *
 * Copyright (C) 2017 zowee technology Co., Ltd
 *
 * This file is a list of andlink server resouces.
 * 
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include "andlink.h"
#include "router_cfg.h"
#include "andlink_business.h"

/*
 * desc: 验证网关下发的鉴权信息。
 *
 * return: 0: Auth_success, else: Auth_failed
 */
int check_gw_auth_info(cJSON *json)
{
	char *deviceId = NULL;
	deviceId = cJSON_GetObjectItem(json, "deviceId")->valuestring;
	coap_printf("deviceId: %s", deviceId);

	if ((NULL == deviceId) || (NULL == g_deviceId))
		return -1;

	return (strcmp(deviceId, g_deviceId));
}

/* my question. the options are right or not. */
void andlink_add_option(coap_pdu_t **response)
{
	unsigned char buf[3];
	coap_pdu_t *pdu = *response;
	pdu->hdr->code = COAP_RESPONSE_CODE(205);

	/* option of CONTENT TYPE */	
	coap_add_option(pdu, COAP_OPTION_CONTENT_TYPE,
	coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
	
	coap_add_option(pdu, COAP_OPTION_MAXAGE,
	coap_encode_var_bytes(buf, 0x2ffff), buf);

	return ;
}

/*
 * desc: 网关收到设备广播请求后的响应。
 * type: POST-CON
 * form: 
 	{
		"searchAck":"ANDLINK-GW", //my question.参数有问题吧
		"andlinkVersion":"V2"
 	}
 * TIPS: my question.该接口需要沟通。理解应该是返回GWIP。
 * 
 */
void hnd_qlink_searchack(coap_context_t *ctx, struct coap_resource_t *resource, 
		coap_address_t *peer, coap_pdu_t *request, str *token, coap_pdu_t *response)
{
	unsigned char *data;
	size_t size;

	coap_get_data(request, &size, &data);
	VERIFY_DATA(size)
	else
	{
		andlink_add_option(&response);
		coap_printf("data: %s, len: %d\n", data, size);
		cJSON *json = NULL;
		json = cJSON_Parse(data);
		if (!json) INVALID_VAL_RET;

		/* do something. get json args to judge authorization is OK or not */
		//if (AL_SUCCESS == auth)
		{
			cJSON_Delete(json);
			SUCCESS_RET;
		}
		//else
		{
			cJSON_Delete(json);
			AUTH_FAILED_RET;
		}
	}

	return ;
}

void save_repeater_paras(char *channel, char *SSID, char *encrypt, char *type, char *password)
{
	char encrypt_b[32] = {0};

	/* 网关下发的加密模式: OPEN/WEP/WPAPSK/WPAPSK2/MIXED-WPAPSK2。为空表示MIXED-WPAPSK2 */
	/* 中继需要的加密模式: OPEN/SHARED/WPAPSK/WPA2PSK */
	if (encrypt == NULL || 0 == strlen(encrypt) || !strcmp(encrypt, "WPAPSK2")
			|| !strcmp(encrypt, "MIXED-WPAPSK2"))
		strcpy(encrypt_b, "WPA2PSK");
	else if (!strcmp(encrypt, "OPEN"))
		strcpy(encrypt_b, "OPEN");
	else if (!strcmp(encrypt, "WEP"))
	{
		coap_printf("EncryptType is WEP!");
		strcpy(encrypt_b, "SHARED");
	}	
	else if (!strcmp(encrypt, "WPAPSK"))
		strcpy(encrypt_b, "WPAPSK");
	else
		strcpy(encrypt_b, "WPA2PSK");
	
	/* Q-LINK 4: 组网设备切换上网通道。 */
	nvram_bufset(RT2860_NVRAM, "ApCliSsid", SSID);
	nvram_bufset(RTDEV_NVRAM, "ApCliEnable", "0");
	nvram_bufset(RT2860_NVRAM, "Channel", channel);
	nvram_bufset(RT2860_NVRAM, "AutoChannelSelect", "0");
	nvram_bufset(RT2860_NVRAM, "ApCliEnable", "1");
	nvram_bufset(RT2860_NVRAM, "ApCliAuthMode", encrypt_b);
	/* default is AES */
	if (type == NULL || 0 == strlen(type))
		strcpy(type, "AES");
	if (!strcmp(encrypt_b, "OPEN"))
		nvram_bufset(RT2860_NVRAM, "ApCliEncrypType", "NONE");
	else
		nvram_bufset(RT2860_NVRAM, "ApCliEncrypType", type);
	nvram_bufset(RT2860_NVRAM, "ApCliWPAPSK", password);
	nvram_bufset(RT2860_NVRAM, "ApCliBssid", "");
	nvram_bufset(RT2860_NVRAM, "ApCliDefaultKeyID", "1");
	nvram_bufset(RT2860_NVRAM, "ApCliKey1Type", "1");
	nvram_bufset(RT2860_NVRAM, "ApCliKey2Type", "");
	nvram_bufset(RT2860_NVRAM, "ApCliKey3Type", "");
	nvram_bufset(RT2860_NVRAM, "ApCliKey4Type", "");
	nvram_bufset(RT2860_NVRAM, "ApCliKey1Str", password);
	nvram_bufset(RT2860_NVRAM, "ApCliKey2Str", "");
	nvram_bufset(RT2860_NVRAM, "ApCliKey3Str", "");
	nvram_bufset(RT2860_NVRAM, "ApCliKey4Str", "");

	nvram_bufset(RT2860_NVRAM, "Q_LINK_OK", "1");

	return ;
}

void save_wifi_paras(char *channel, char *SSID, char *encrypt, char *type, char *password)
{
	char encrypt_b[32] = {0};

	/* 网关下发的加密模式: OPEN/WEP/WPAPSK/WPAPSK2/MIXED-WPAPSK2。为空表示MIXED-WPAPSK2 */
	/* 本地使用的加密模式: OPEN/WEP/WPAPSK/WPA2PSK/WPAPSKWPA2PSK。为空表示WPAPSKWPA2PSK */
	if (encrypt == NULL || 0 == strlen(encrypt) || !strcmp(encrypt, "WPAPSK2")
			|| !strcmp(encrypt, "MIXED-WPAPSK2"))
		strcpy(encrypt_b, "WPAPSKWPA2PSK");
	else if (!strcmp(encrypt, "OPEN"))
		strcpy(encrypt_b, "OPEN");
	else if (!strcmp(encrypt, "WEP"))
		strcpy(encrypt_b, "WEP");
	else if (!strcmp(encrypt, "WPAPSK"))
		strcpy(encrypt_b, "WPAPSK");
	else if (!strcmp(encrypt, "WPAPSK2"))
		strcpy(encrypt_b, "WPA2PSK");
	else
		strcpy(encrypt_b, "WPAPSKWPA2PSK");

	/* Q-LINK 4: 若自身为中继设备，则配置自身接入点与网关一致。 */
	nvram_bufset(RT2860_NVRAM, "SSID1", SSID);
	nvram_bufset(RT2860_NVRAM, "WPAPSK1", password);
	set_nth_value_flash(RT2860_NVRAM, 0, "AuthMode", encrypt_b);
	/* default is AES */
	if (type == NULL || 0 == strlen(type))
		strcpy(type, "AES");
	if (!strcmp(encrypt_b, "OPEN"))
		set_nth_value_flash(RT2860_NVRAM, 0, "EncrypType", "NONE");
	else
		set_nth_value_flash(RT2860_NVRAM, 0, "EncrypType", type);

	return ;
}

/*
 * desc: 网关通过此接口发送正式上网通道给设备。
 * type: POST-CON
 * form: 
 	{
		"SSID":"",
		"password":"",
		"encrypt":""
 	}
 * TIPS: 
 	1、此接口在设备未注册前以明文发送。设备注册后，会加密发送。//my question.太复杂
	2、秘钥为注册获取到deviceID。
 * 
 */
/* Q-LINK 3: 网关发送上网SSID和密码。 */
void hnd_qlink_netinfo(coap_context_t *ctx, struct coap_resource_t *resource, 
		coap_address_t *peer, coap_pdu_t *request, str *token, coap_pdu_t *response)
{
	unsigned char *data;
	size_t size;

	coap_get_data(request, &size, &data);
	VERIFY_DATA(size)
	else
	{
		andlink_add_option(&response);

		coap_printf("data: %s, len: %d\n", data, size);
		cJSON *json = NULL;
		json = cJSON_Parse(data);
		if (!json) INVALID_VAL_RET;

		if (!cJSON_HasObjectItem(json, "SSID") || !cJSON_HasObjectItem(json, "password")
				|| !cJSON_HasObjectItem(json, "encrypt"))
		{
			cJSON_Delete(json);
			LACK_ARG_RET;
		}

		SUCCESS_NO_RET;
		coap_send(ctx, peer, response);

		//为APP配网流程
		if (cJSON_HasObjectItem(json, "CGW"))
		{
			cJSON *cgw = NULL;
			cgw = cJSON_GetObjectItem(json, "CGW");
			if (!cJSON_HasObjectItem(cgw, "user_key") || !cJSON_HasObjectItem(cgw, "gwAddress"))
			{
				cJSON_Delete(json);
				LACK_ARG_RET;
			}

			char *user_key=NULL, *gwAddress=NULL;

			user_key = cJSON_GetObjectItem(cgw, "user_key")->valuestring;
			gwAddress = cJSON_GetObjectItem(cgw, "gwAddress")->valuestring;
			
			coap_printf("user_key: %s, gwAddress: %s", user_key, gwAddress);

			nvram_bufset(RT2860_NVRAM, "andlink_userKey", user_key);
			nvram_bufset(RT2860_NVRAM, "andlink_udpServer", gwAddress);
			nvram_commit(RT2860_NVRAM);
			cJSON_Delete(json);
			return;
		}

		char *SSID = NULL, *encrypt = NULL, *password = NULL;
		SSID = cJSON_GetObjectItem(json, "SSID")->valuestring;
		encrypt = cJSON_GetObjectItem(json, "encrypt")->valuestring;
		password = cJSON_GetObjectItem(json, "password")->valuestring;

		/* Apcli scan, get encryptType */
		FILE *pp = NULL;
		char cmd[64] = {0};
		char buf[256] = {0};
		char ssid_s[32] = {0}, security_s[32] = {0}, channel[16] = {0}, type[16] = {0};
		int found = 0;
		do_system("iwpriv ra0 set SiteSurvey=1");
		sleep(3);
		sprintf(cmd, "iwpriv ra0 get_site_survey");
		pp = popen(cmd, "r");
		if (pp)
		{
			fgets(buf, sizeof(buf), pp);
			fgets(buf, sizeof(buf), pp);
			while (fgets(buf, sizeof(buf), pp))
			{
				if (3 == sscanf(buf, "%s%s%*s%s", channel, ssid_s, security_s))
				{
					if (!strcmp(SSID, ssid_s))
					{
						coap_printf("Found SSID!");
						found = 1;
						break;
					}
				}
			}

			if (found)
			{
				if (!strcmp(security_s, "WEP"))
					strcpy(type, "WEP");
				else if(!strcmp(security_s, "NONE"))
					strcpy(type, "NONE");
				else
				{
					sscanf(security_s, "%*[^/]/%s", type);
					/* If not AES, it's TKIP. */
					if (0 != strcmp(type, "TKIP"))
						strcpy(type, "AES");
				}
			}
		}
		coap_printf("channel: %s, ssid_s: %s, security_s: %s, type: %s",
				channel, ssid_s, security_s, type);

		qlink_success(g_deviceId, gwip);
		//router_stopLedBlink();
		sleep(1);

		save_repeater_paras(channel, SSID, encrypt, type, password);
		//save_wifi_paras(channel, SSID, encrypt, type, password);
		nvram_commit(RT2860_NVRAM);
		do_system("init_system restart");
		cJSON_Delete(json);
	}

	return ;
}

#if 0
/*
 * desc: 响应网关/device/command/control的控制消息。
 * type: POST-CON
 * form: 
 	{
		"deviceId":"",
		"childDeviceId":"", //为空则说明是控制设备，不是控制子设备。
		//各个事件的数据值
		"data":{
			"params": [
				{"paramCode": "SystemControl", "paramValue": "Reboot"}, //两个例子，分别控制重启和复位。
				{"paramCode": "SystemControl", "paramValue": "Reset"} //两个例子
			]
		}
 	}
 * TIPS: 说明响应格式，
 		1、命令已接受并处理完成，只返回状态码，没有后续响应。如:{"respCode":1 或其他非0值，1000鉴权异常等}
 		2、命令已接受并等待处理，后续处理结果通过上报消息接口(device_inform_data)异步返回。{"respCode":0}
 		3、命令已接受并处理完成，同步返回更新后的数据，则设备应返回如下form中格式的数据。
 
 * 
 */
void hnd_regist_dev_control(coap_context_t *ctx, struct coap_resource_t *resource, 
		coap_address_t *peer, coap_pdu_t *request, str *token, coap_pdu_t *response)
{
	unsigned char *data;
	size_t size;

	coap_get_data(request, &size, &data);
	VERIFY_DATA(size)
	else
	{
		andlink_add_option(&response);
		coap_printf("data: %s, len: %d\n", data, size);
		cJSON *json = NULL;
		json = cJSON_Parse(data);
		if (!json) INVALID_VAL_RET;

		/* do something. params中的参数是一个数组形式。 */
		char *params = NULL;
		//params = cJSON_GetArrayItem(json, "params"); //json该如何解析数组。
		/*
		 ...
		 */
		//char *paramCode = NULL;
		//char *paramValue = NULL;
		//paramCode = cJSON_GetObjectItem(params, "paramCode")->valuestring;
		//paramValue = cJSON_GetObjectItem(params, "paramValue")->valuestring;
		//strcmp("Reboot", paramValue);

		cJSON_Delete(json);
		SUCCESS_RET;
	}

	return ;
}

/*
 * desc: 触发设备上报当前所有数据，RESERVED
 * type: POST-CON
 * form: 
 	{
		"deviceId":"",
		"childDeviceId":"", //为空则说明是触发设备，而不是子设备
		"data":{ //可选，若不为空则上报指定参数值。
			"paramCodes": [
				{"paramCode": "PARAMSCODE1"}, //根据Code上报参数值
				{"paramCode": "PARAMSCODE2"}, //根据Code上报参数值
				...
			]
		}
 	}
 * TIPS: 说明响应格式，
 		1、命令解析异常，返回{"respCode":>1}的异常码
 		2、命令已接受等待(device_inform_data)异步返回。返回{"respCode":0}
 		3、命令已接受并处理完成，同步返回更新后的数据，则设备应返回如下form中格式的数据。
 * 
 */
void hnd_regist_get_dev_data(coap_context_t *ctx, struct coap_resource_t *resource, 
		coap_address_t *peer, coap_pdu_t *request, str *token, coap_pdu_t *response)
{
	unsigned char *data;
	size_t size;

	coap_get_data(request, &size, &data);
	VERIFY_DATA(size)
	else
	{
		andlink_add_option(&response);
		coap_printf("data: %s, len: %d\n", data, size);
		cJSON *json = NULL;
		json = cJSON_Parse(data);
		if (!json) INVALID_VAL_RET;

		/* do something. params中的参数是一个数组形式。 */
		char *params = NULL;
		//params = cJSON_GetArrayItem(json, "params"); //json该如何解析数组。
		/*
		 ...
		 */
		//char *paramCode = NULL;
		//paramCode = cJSON_GetObjectItem(params, "paramCode")->valuestring;
		//看是通过nvram取paramcode的值，还是其他方式。
		//get_paramcode_value();

		cJSON_Delete(json);
		SUCCESS_RET;
	}

	return ;
}
#endif

/*
 * desc: 接触设备在网关上的绑定关系，或子设备在设备上的绑定关系，
 * type: POST-CON
 * form: 
 	{
		"deviceId": "",
		"childDeviceId": "" //若无则为解绑设备。
 	}
 
 * TIPS: 解绑时，设备具体要做哪些事，需要沟通，this is my question, do something
 * 
 */
void hnd_regist_dev_unbind(coap_context_t *ctx, struct coap_resource_t *resource, 
		coap_address_t *peer, coap_pdu_t *request, str *token, coap_pdu_t *response)
{
	unsigned char *data;
	size_t size;

	coap_get_data(request, &size, &data);
	VERIFY_DATA(size)
	else
	{
		andlink_add_option(&response);
		coap_printf("data: %s, len: %d\n", data, size);
		cJSON *json = NULL;
		json = cJSON_Parse(data);
		if (!json) INVALID_VAL_RET;

		if (!cJSON_HasObjectItem(json, "deviceId"))
		{
			cJSON_Delete(json);
			LACK_ARG_RET;
		}

		if (check_gw_auth_info(json))
		{
			cJSON_Delete(json);
			AUTH_FAILED_RET;
		}

		/* do something */
		nvram_bufset(RT2860_NVRAM, "andlink_deviceId", "");
		nvram_bufset(RT2860_NVRAM, "andlink_deviceToken", "");
		nvram_bufset(RT2860_NVRAM, "andlink_andlinkToken", "");
		nvram_bufset(RT2860_NVRAM, "andlink_timestamp", "");
		nvram_bufset(RT2860_NVRAM, "andlink_heartbeatTime", "");
		nvram_bufset(RT2860_NVRAM, "Q_LINK_OK", "0");

		nvram_commit(RT2860_NVRAM);

		cJSON_Delete(json);
		SUCCESS_RET;
		return ;
	}

	return ;
}

/*
 * desc: 设备文件操作
 * type: POST-CON
 * form: 
 	{
		"deviceId": "",
		"childDeviceId": "", //可选，若无则是针对设备的文件操作。
		"fileMode": "", //DOWN，下载文件。UP，上传文件
		"fileType": "", //可选，如FIRM固件，APP应用，LOG日志文件，CFG配置文件。what the fuck!
		"fileUrl": "http://xxx/xxx", //设备文件下载/上传路径。
		"downMode": 1 //若为DOWN升级，1立即升级，0下次重启升级。
 	}

	//andlink2.3
 	{
		"deviceId": "CMCC-30103-000C43E1762A",
		"childDeviceId": "",
		"fileMode": "DOWN",
		"fileType": "FIRM",
		"fileUrl": "ftp://192.168.0.106/root_uImage",
		"downMode": 1
 	}
 * TIPS: 
 * 
 */
void hnd_regist_dev_file(coap_context_t *ctx, struct coap_resource_t *resource, 
		coap_address_t *peer, coap_pdu_t *request, str *token, coap_pdu_t *response)
{
	unsigned char *data;
	size_t size;

	coap_get_data(request, &size, &data);
	VERIFY_DATA(size)
	else
	{
		andlink_add_option(&response);
		coap_printf("data: %s, len: %d\n", data, size);
		cJSON *json = NULL;
		json = cJSON_Parse(data);
		if (!json) INVALID_VAL_RET;

		if (!cJSON_HasObjectItem(json, "deviceId") || !cJSON_HasObjectItem(json, "fileMode") || 
				!cJSON_HasObjectItem(json, "fileType") || !cJSON_HasObjectItem(json, "fileUrl") || 
				!cJSON_HasObjectItem(json, "downMode"))
		{
			cJSON_Delete(json);
			LACK_ARG_RET;
		}

		if (check_gw_auth_info(json))
		{
			cJSON_Delete(json);
			AUTH_FAILED_RET;
		}

		/* do something */

		char *fileMode = NULL, *fileType = NULL, *fileUrl = NULL;
		fileMode = cJSON_GetObjectItem(json, "fileMode")->valuestring;
		fileType = cJSON_GetObjectItem(json, "fileType")->valuestring;
		fileUrl = cJSON_GetObjectItem(json, "fileUrl")->valuestring; //没有使用coap协议下载文件，使用的HTTP。
		/* do something about download and upgrade */
		int downMode = cJSON_GetObjectItem(json, "downMode")->valueint;

		coap_printf("fileMode: %s, fileType: %s, fileUrl: %s, downMode: %d", 
				fileMode, fileType, fileUrl, downMode);

		/* DOWN / UP */
		if (!strcmp(fileMode, "DOWN"))
		{
			if (!strcmp(fileType, "FIRM"))
			{
				FILE_SUCCESS_NO_RET;
				coap_send(ctx, peer, response);
				sleep(2);
				/* Download file */
				if (UPGRADE_SUCCESS == andlink_download_upgrade(fileUrl))
				{
					//if (!strcmp(downMode, "1"))
						device_inform_data(g_deviceId, gwip, EVENT_DEVICE_FILE, FILE_UPGRADE_SUCCESS, "FILE_UPGRADE_SUCCESS");						
						if (downMode == 1) 
						{
							usleep(500*1000);
							do_system("reboot");
						}
				}
				else
				{
					device_inform_data(g_deviceId, gwip, EVENT_DEVICE_FILE, FILE_TRANSFER_INTERRUPT, "FILE_TRANSFER_INTERRUPT");
					coap_printf("Download or upgrade failed!");
					//do_system("reboot");
				}
				cJSON_Delete(json);
				return ;
			}
			else
			{
				coap_printf("RESERVED!");
			}
		}
		//RESERVED
		else if (!strcmp(fileMode, "UP"))
		{
			coap_printf("RESERVED!");
		}
		else
		{
			cJSON_Delete(json);
			INVALID_CMD_RET;
		}

		cJSON_Delete(json);
		FILE_SUCCESS_RET;
	}

	return ;
}

/*
 * desc: 网关将WiFi配置信息下发给组网设备
 * type: POST-CON
 * form: 
 	{
		"data": 
		{
			"SyncCode": "105",
			"Configurations":
			[
				{
					"Radio": "2.4G",
					"Index": 1,
					"Enable": 1,
					"SSID": "LY_TEST_andlink_12",
					"SecurityMode": "WPA-PSK2",
					"Pwd": "12341234",
					"MaxAssociateNum": 20,
					"SSIDAdvertisementEnabled": 1
				},
				{
					"Radio": "5G",
					"Index": 1,
					"Enable": 1,
					"SSID": "CMCC_XX_5G",
					"SecurityMode": "WPA-PSK2",
					"Pwd": "12345678",
					"MaxAssociateNum": 10,
					"SSIDAdvertisementEnabled": 1
				}
			]
		},
		"deviceId": "CMCC-30103-000C43E1762A"
 	}
 * TIPS: 
 * 
 */
void hnd_control_wifi_para_sync(coap_context_t *ctx, struct coap_resource_t *resource, 
		coap_address_t *peer, coap_pdu_t *request, str *token, coap_pdu_t *response)
{
	unsigned char *data;
	size_t size;

	coap_get_data(request, &size, &data);
	VERIFY_DATA(size)
	else
	{
		andlink_add_option(&response);
		coap_printf("data: %s, len: %d\n", data, size);
		cJSON *json = NULL;
		json = cJSON_Parse(data);
		if (!json) INVALID_VAL_RET;

		if (!cJSON_HasObjectItem(json, "deviceId") || !cJSON_HasObjectItem(json, "data"))
		{
			cJSON_Delete(json);
			LACK_ARG_RET;
		}

		if (check_gw_auth_info(json))
		{
			cJSON_Delete(json);
			AUTH_FAILED_RET;
		}

		/* data */
		cJSON *Jdata = NULL;
		Jdata = cJSON_GetObjectItem(json, "data");

		if (!cJSON_HasObjectItem(Jdata, "SyncCode") || !cJSON_HasObjectItem(Jdata, "Configurations"))
		{
			cJSON_Delete(json);
			LACK_ARG_RET;
		}

		char *SyncCode = NULL;
		SyncCode = cJSON_GetObjectItem(Jdata, "SyncCode")->valuestring;
		coap_printf("SyncCode: %s", SyncCode);
		nvram_bufset(RT2860_NVRAM, "SyncCode", SyncCode);

		SUCCESS_NO_RET;
		coap_send(ctx, peer, response);

		char eth[4 + 1] = {0};
		char tmp[64] = {0};
		char sMode[64] = {0};
		char sEnable[2] = {0};
		cJSON *Kdata = NULL;
		int Index = -1, Enable = -1, MaxAssociateNum = -1, BEnabled = -1;
		char *Radio = NULL, *SSID = NULL, *SecurityMode = NULL, *Pwd = NULL;
		Kdata = cJSON_GetObjectItem(Jdata, "Configurations");
		/* ==================  2.4G Maybe  ================== */
		Kdata = Kdata->child;
		while (Kdata) {
			if (!cJSON_HasObjectItem(Kdata, "Index") || !cJSON_HasObjectItem(Kdata, "Enable") ||
					!cJSON_HasObjectItem(Kdata, "Radio") || !cJSON_HasObjectItem(Kdata, "SSID") ||
					!cJSON_HasObjectItem(Kdata, "SecurityMode") || !cJSON_HasObjectItem(Kdata, "Pwd"))
			{
				cJSON_Delete(json);
				LACK_ARG_RET;
			}

			Index = cJSON_GetObjectItem(Kdata, "Index")->valueint;
			Enable = cJSON_GetObjectItem(Kdata, "Enable")->valueint;
			if (cJSON_HasObjectItem(Kdata, "MaxAssociateNum"))
				MaxAssociateNum = cJSON_GetObjectItem(Kdata, "MaxAssociateNum")->valueint;
			else /* default */
				MaxAssociateNum = 30;

			if (cJSON_HasObjectItem(Kdata, "SSIDAdvertisementEnabled"))
				BEnabled = cJSON_GetObjectItem(Kdata, "SSIDAdvertisementEnabled")->valueint;
			else /* default */
				BEnabled = 0;

			Radio = cJSON_GetObjectItem(Kdata, "Radio")->valuestring;
			SSID = cJSON_GetObjectItem(Kdata, "SSID")->valuestring;
			SecurityMode = cJSON_GetObjectItem(Kdata, "SecurityMode")->valuestring;
			Pwd = cJSON_GetObjectItem(Kdata, "Pwd")->valuestring;
			coap_printf("%s %s %s %s, %d %d %d %d", Radio, SSID, SecurityMode, Pwd,
					Index, Enable, MaxAssociateNum, BEnabled);

			int nvram_id = -1;
			if (!strcmp(Radio, "2.4G"))
			{
				nvram_id = RT2860_NVRAM;
			}
#if defined (CONFIG_PRODUCT_341AA)
			else if (!strcmp(Radio, "5G"))
			{
				nvram_id = RTDEV_NVRAM;
				if (Index > 4) Index -= 4;
			}
#endif

			if (nvram_id == -1) {
				Kdata = Kdata->next;
				continue;
			}

			memset(eth, 0, sizeof(eth));
			if (RT2860_NVRAM == nvram_id) {
				sprintf(eth, "ra%d", Index - 1);
			} else {
				sprintf(eth, "rai%d", Index - 1);
			}

			//MaxAssociateNum
			char MaxNum_tmp[8] = {0};
			sprintf(MaxNum_tmp, "%d", MaxAssociateNum);
			nvram_set(nvram_id, "MaxStaNum", MaxNum_tmp);

			//SSIDAdvertisementEnabled
			char Broadcast_tmp[8] = {0};
			sprintf(Broadcast_tmp, "%d", REVISE_VAL(BEnabled));
			set_nth_value_flash(nvram_id, Index -1, "HideSSID", Broadcast_tmp);

			//1. Set SSID
			memset(tmp, 0x0, sizeof(tmp));
			sprintf(tmp, "SSID%d", Index);
			nvram_bufset(nvram_id, tmp, SSID);

			//2. Set Enable
			memset(sEnable, 0x0, sizeof(sEnable));
			memset(tmp, 0x0, sizeof(tmp));
			sprintf(tmp, "Enable_SSID%d", Index);
			sprintf(sEnable, "%d", Enable);
			nvram_bufset(nvram_id, tmp, sEnable);
			if (Enable) {
				do_system("ifconfig %s up", eth);
			} else {
				do_system("ifconfig %s down", eth);
			}

			//3. Set SecurityMode /Pwd
			memset(tmp, 0x0, sizeof(tmp));
			if(!strcmp(SecurityMode, "WPA-Personal")) {
				strcpy(tmp, "WPAPSK");
			} else if (!strcmp(SecurityMode, "WPA2-Personal")) {
				strcpy(tmp, "WPA2PSK");
			} else if (!strcmp(SecurityMode, "MIXED-WPAPSK2")) {
				strcpy(tmp, "WPAPSKWPA2PSK");
			}

			do_system("iwpriv %s set SSID=%s", eth,  SSID);

			if (strlen(tmp) > 0) {
				//1. AuthMode
				set_nth_value_flash(nvram_id, Index -1, "AuthMode", tmp);
				do_system("iwpriv %s set AuthMode=%s", eth, tmp);
				//default encrypt type
				set_nth_value_flash(nvram_id, Index -1, "EncrypType", "AES");
				do_system("iwpriv %s set EncrypType=%s", eth, "AES");
				//2. Pwd
				memset(sMode, 0, sizeof(sMode));
				sprintf(sMode, "WPAPSK%d", Index);
				nvram_bufset(nvram_id,	sMode, Pwd);
				do_system("iwpriv %s set WPAPSK=%s", eth, Pwd);
				
				//3. Set Other Configuration Default Value
				set_nth_value_flash(nvram_id, Index -1, "RekeyMethod", "TIME");
				set_nth_value_flash(nvram_id, Index -1, "IEEE8021X", "0");
				set_nth_value_flash(nvram_id, Index -1, "DefaultKeyID", "2");
				set_nth_value_flash(nvram_id, Index -1, "EncrypType", "AES");
				set_nth_value_flash(nvram_id, Index -1, "RekeyInterval", "3600");
				
			} else { //Not Set Password  
				set_nth_value_flash(nvram_id, Index -1, "AuthMode", "OPEN");
				do_system("iwpriv %s set AuthMode=%s", eth, "OPEN");
				set_nth_value_flash(nvram_id, Index -1, "EncrypType", "NONE");
				do_system("iwpriv %s set EncrypType=%s", eth, "NONE");
			}

			/* 放在最后，让WiFi参数快速生效。 */
			do_system("iwpriv %s set SSID=%s", eth, SSID);

			nvram_commit(nvram_id);
			Kdata = Kdata->next;

			usleep(200*1000);
		}

		//do_system("init_system restart");
		cJSON_Delete(json);
		return ;
	}
	
	return ;
}

/*
 * desc: 网关控制组网设备的WiFi开关
 * type: POST-CON
 * form: 
 	{
		"data": 
		{
			"Radio": "all",
			"Enable": 1
		},
		"deviceId": "CMCC-30103-000C43E1762A"
 	}
 * TIPS: 
 * 
 */
void hnd_control_wifi_swtich(coap_context_t *ctx, struct coap_resource_t *resource, 
		coap_address_t *peer, coap_pdu_t *request, str *token, coap_pdu_t *response)
{
	unsigned char *data;
	size_t size;

	coap_get_data(request, &size, &data);
	VERIFY_DATA(size)
	else
	{
		andlink_add_option(&response);
		coap_printf("data: %s, len: %d\n", data, size);
		cJSON *json = NULL;
		json = cJSON_Parse(data);
		if (!json) INVALID_VAL_RET;

		if (!cJSON_HasObjectItem(json, "deviceId") || !cJSON_HasObjectItem(json, "data"))
		{
			cJSON_Delete(json);
			LACK_ARG_RET;
        }
        
		if (check_gw_auth_info(json))
		{
			cJSON_Delete(json);
			AUTH_FAILED_RET;
		}

		cJSON *Jdata = NULL;
		Jdata = cJSON_GetObjectItem(json, "data");

		if (!cJSON_HasObjectItem(Jdata, "Radio") || !cJSON_HasObjectItem(Jdata, "Enable"))
		{
			cJSON_Delete(json);
			LACK_ARG_RET;
		}

		char *Radio = NULL;
		int Enable;
		Radio = cJSON_GetObjectItem(Jdata, "Radio")->valuestring;
		Enable = cJSON_GetObjectItem(Jdata, "Enable")->valueint;
		coap_printf("Radio: %s, Enable: %d", Radio, Enable);

		if (!strcmp("all", Radio))
		{
			coap_printf("");
            if (Enable == 0) { //Close Wifi
				close_wifi("2.4G");
#if defined (CONFIG_PRODUCT_341AA)
				close_wifi("5G");
#endif
				SUCCESS_NO_RET;
				coap_send(ctx, peer, response);
				do_system("init_system restart");
				cJSON_Delete(json);
				return ;
            } else { //Open Wifi
				open_wifi("2.4G");
#if defined (CONFIG_PRODUCT_341AA)
				open_wifi("5G");
#endif
				SUCCESS_NO_RET;
				coap_send(ctx, peer, response);
                do_system("insmod -q rt2860v2_ap");
				do_system("init_system restart");
				cJSON_Delete(json);
				return ;
            }
		}
		else if (!strcmp("2.4G", Radio))
		{
			coap_printf("");
            if (Enable == 0) { //Close Wifi                
				close_wifi(Radio);
				SUCCESS_NO_RET;
				coap_send(ctx, peer, response);
				do_system("init_system restart");
				cJSON_Delete(json);
				return ;	 
            } else { //Open Wifi
				open_wifi(Radio);
				SUCCESS_NO_RET;
				coap_send(ctx, peer, response);
                do_system("insmod -q rt2860v2_ap");
				do_system("init_system restart");
				cJSON_Delete(json);
				return ;	 
            }
        }
#if defined (CONFIG_PRODUCT_341AA)
		else if (!strcmp("5G", Radio))
		{
			coap_printf("");
            if (Enable == 0) { //Close Wifi                
				close_wifi(Radio);
				SUCCESS_NO_RET;
				coap_send(ctx, peer, response);
				do_system("init_system restart");
				cJSON_Delete(json);
				return ;	 
            } else { //Open Wifi
				open_wifi(Radio);
				SUCCESS_NO_RET;
				coap_send(ctx, peer, response);
                do_system("insmod -q rt2860v2_ap");
				do_system("init_system restart");
				cJSON_Delete(json);
				return ;	 
            }
		}
#endif
		else
		{
			cJSON_Delete(json);
			UNSUPP_ARG_RET;
		}

		cJSON_Delete(json);
		SUCCESS_RET;
	}

	return ;
}

/*
 * desc: 网关控制组网设备的WiFi功率
 * type: POST-CON
 * form: 
 	{
		"data": 
		{
			"Radio": "all",
			"TransmitPower": 75"
		},
		"deviceId": "CMCC-30103-000C43E1762A"
 	}
 * TIPS: 
 * 
 */
void hnd_control_wifi_radio(coap_context_t *ctx, struct coap_resource_t *resource, 
		coap_address_t *peer, coap_pdu_t *request, str *token, coap_pdu_t *response)
{
	unsigned char *data;
	size_t size;

	coap_get_data(request, &size, &data);
	VERIFY_DATA(size)
	else
	{
		andlink_add_option(&response);
		coap_printf("data: %s, len: %d\n", data, size);
		cJSON *json = NULL;
		json = cJSON_Parse(data);
		if (!json) INVALID_VAL_RET;

		if (!cJSON_HasObjectItem(json, "deviceId") || !cJSON_HasObjectItem(json, "data"))
		{
			cJSON_Delete(json);
			LACK_ARG_RET;
        }

		if (check_gw_auth_info(json))
		{
			cJSON_Delete(json);
			AUTH_FAILED_RET;
		}

		cJSON *Jdata = NULL;
		Jdata = cJSON_GetObjectItem(json, "data");

		if (!cJSON_HasObjectItem(Jdata, "Radio") || !cJSON_HasObjectItem(Jdata, "TransmitPower")) {
            cJSON_Delete(json);
			LACK_ARG_RET;
        }
        
		char *Radio = NULL;
		char *TransmitPower = NULL;
		Radio = cJSON_GetObjectItem(Jdata, "Radio")->valuestring;
		TransmitPower = cJSON_GetObjectItem(Jdata, "TransmitPower")->valuestring;
		coap_printf("Radio: %s, TransmitPower: %s", Radio, TransmitPower);

		if (!strcmp("all", Radio))
		{
			wifi_power("2.4G", TransmitPower);
#if defined (CONFIG_PRODUCT_341AA)
			wifi_power("5G", TransmitPower);
#endif
			SUCCESS_NO_RET;
			coap_send(ctx, peer, response);
            do_system("init_system restart");
			cJSON_Delete(json);
			return ;     
		}
		else if (!strcmp("2.4G", Radio))
		{
			wifi_power(Radio, TransmitPower);
			SUCCESS_NO_RET;
			coap_send(ctx, peer, response);
            do_system("init_system restart");
			cJSON_Delete(json);
			return ;
        }
#if defined (CONFIG_PRODUCT_341AA)
		else if (!strcmp("5G", Radio))
		{
			wifi_power(Radio, TransmitPower);
			SUCCESS_NO_RET;
			coap_send(ctx, peer, response);
            do_system("init_system restart");
			cJSON_Delete(json);
			return ;
		}
#endif
		else
		{
			cJSON_Delete(json);
			UNSUPP_ARG_RET;
		}
        
		cJSON_Delete(json);
		SUCCESS_RET;
	}

	return ;
}

/*
 * desc: 网关激活组网设备的WPS
 * type: POST-CON
 * form: 
 	{
		"data": 
		{
			"Radio": "all"
		},
		"deviceId": "CMCC-30103-000C43E1762A"
 	}
 * TIPS: 
 * 
 */
void hnd_control_wifi_wps(coap_context_t *ctx, struct coap_resource_t *resource, 
		coap_address_t *peer, coap_pdu_t *request, str *token, coap_pdu_t *response)
{
	unsigned char *data;
	size_t size;

	coap_get_data(request, &size, &data);
	VERIFY_DATA(size)
	else
	{
		andlink_add_option(&response);
		coap_printf("data: %s, len: %d\n", data, size);
		cJSON *json = NULL;
		json = cJSON_Parse(data);
		if (!json) INVALID_VAL_RET;

		if (!cJSON_HasObjectItem(json, "deviceId") || !cJSON_HasObjectItem(json, "data"))
		{
			cJSON_Delete(json);
			LACK_ARG_RET;
		}

		if (check_gw_auth_info(json))
		{
			cJSON_Delete(json);
			AUTH_FAILED_RET;
		}

		cJSON *Jdata = NULL;
		Jdata = cJSON_GetObjectItem(json, "data");

		if (!cJSON_HasObjectItem(Jdata, "Radio"))
		{
			cJSON_Delete(json);
			LACK_ARG_RET;
		}

		char *Radio = NULL;
		Radio = cJSON_GetObjectItem(Jdata, "Radio")->valuestring;
		coap_printf("Radio: %s", Radio);

		//do something. about WPS
		if (!strcmp("all", Radio))
		{
			int nd_pid;
			if (-1 != (nd_pid = getPidByName("nvram_daemon")))
				kill(nd_pid, SIGUSR1);
		}
		else if (!strcmp("2.4G", Radio))
		{
			int nd_pid;
			if (-1 != (nd_pid = getPidByName("nvram_daemon")))
				kill(nd_pid, SIGUSR1);
		}
		//RESERVED
		else if (!strcmp("5G", Radio))
		{
			//WPS("5G");
		}
		else
		{
			coap_printf("");
			cJSON_Delete(json);
			UNSUPP_ARG_RET;
		}

		cJSON_Delete(json);
		SUCCESS_RET;
	}

	return ;
}

/*
 * desc: 网关下发黑白名单给组网设备
 * type: POST-CON
 * form: 
 	{
		"data":
		{
			"MacFilterEnable": 1,
			"MacFilterPolicy": 0,
			"MacFilterEntries": "7C:04:D0:6E:27:58/LY_iphone,c8:25:e1:8c:35:1e/android"
		},
		"deviceId": "CMCC-30103-000C43E1762A"
 	}
 * TIPS: 可以看到MacFilterEntries的格式，每条规则包含MAC、设备名。多条规则间以逗号隔开。
 * 
 */
void hnd_control_mac_filter(coap_context_t *ctx, struct coap_resource_t *resource, 
		coap_address_t *peer, coap_pdu_t *request, str *token, coap_pdu_t *response)
{
	unsigned char *data;
	size_t size;

	coap_get_data(request, &size, &data);
	VERIFY_DATA(size)
	else
	{
		andlink_add_option(&response);
		coap_printf("data: %s, len: %d\n", data, size);
		cJSON *json = NULL;
		json = cJSON_Parse(data);
		if (!json) INVALID_VAL_RET;

		if (!cJSON_HasObjectItem(json, "deviceId") || !cJSON_HasObjectItem(json, "data"))
		{
			cJSON_Delete(json);
			LACK_ARG_RET;
		}

		if (check_gw_auth_info(json))
		{
			cJSON_Delete(json);
			AUTH_FAILED_RET;
		}

		cJSON *Jdata = NULL;
		Jdata = cJSON_GetObjectItem(json, "data");

		if (!cJSON_HasObjectItem(Jdata, "MacFilterEnable") || !cJSON_HasObjectItem(Jdata, "MacFilterPolicy") || 
				!cJSON_HasObjectItem(Jdata, "MacFilterEntries"))
		{
			cJSON_Delete(json);
			LACK_ARG_RET;
		}

		/* do something about MACFILTER */
		int MacFilterEnable, MacFilterPolicy;
		char *MacFilterEntries = NULL;
		MacFilterEnable = cJSON_GetObjectItem(Jdata, "MacFilterEnable")->valueint;
		MacFilterPolicy = cJSON_GetObjectItem(Jdata, "MacFilterPolicy")->valueint;
		/* MacFilterEntries Example
		 * "00:11:22:33:44:55/pro,80:71:7a:33:cc:f3/android" */
		MacFilterEntries = cJSON_GetObjectItem(Jdata, "MacFilterEntries")->valuestring;

		coap_printf("MacFilterEnable: %d, MacFilterPolicy: %d, MacFilterEntries: %s", 
				MacFilterEnable, MacFilterPolicy, MacFilterEntries);

        //Disable the first SSID
        if (MacFilterEnable == 0) {
            nvram_bufset(RT2860_NVRAM, "AccessPolicy0", "0");
            nvram_bufset(RT2860_NVRAM, "AccessControlList0", "");
            
        } else {
            char macAddr[32] = {0};
            char macList[1024] = {0};
            int rule_num = 0;
            char *start = NULL;
            char sPolicy[2] = {0};

            char *pTok = strtok(MacFilterEntries, ",");
            while(NULL != pTok)
            {
                memset(macAddr, 0, sizeof(macAddr));
                strcpy(macAddr, pTok);
                start = strchr(macAddr, '/');
                if (NULL != start) *start = '\0';
                
                if(strlen(macList)) {
                	sprintf(macList, "%s;%s", macList, macAddr);
                } else {
                	sprintf(macList, "%s", macAddr);
                }
                rule_num += 1;
                pTok = strtok(NULL, ",");     
            }
            coap_printf("last Entry: %s", macList);
    		coap_printf("rules num is: %d", rule_num);
        
            if (MacFilterPolicy == 1) { //White List
                sPolicy[0] = '1';
            } else {  //Black List
                sPolicy[0] = '2';
            }
            nvram_bufset(RT2860_NVRAM, "AccessPolicy0", sPolicy);
            nvram_bufset(RT2860_NVRAM, "AccessControlList0", macList);

        }
        #if 0
        char macAddr[32] = {0};
        char macList[1024] = {0};
		int rule_num = 0;
		char *start = NULL;
		//start = MacFilterEntries;
		//rule = strstr(MacFilterEntries, ",");
		//coap_printf("start: %s, rule: %s", start, rule);
        //char buf[128] = {0};     
        
		while(NULL != rule)
		{
			memset(buf, 0, sizeof(buf));
			*rule = '\0';
			strncpy(buf, start, strlen(start));

            pPosition = strchr(buf, '/');
            *pPosition = '\0';
            if(strlen(macList)) {
        		sprintf(macList, "%s;%s", macList, buf);
            } else {
        		sprintf(macList, "%s", buf);
            }
			coap_printf("one Entry: %s", buf);
			start = rule+1; //dangerous!
			rule = strstr(start, ",");
			rule_num+=1;
            
		}
		coap_printf("last Entry: %s", start);
		coap_printf("rules num is: %d", rule_num+1);
        #endif
        
		nvram_commit(RT2860_NVRAM);

		SUCCESS_NO_RET;
		coap_send(ctx, peer, response);
        do_system("init_system restart");
		cJSON_Delete(json);
		return ;
	}

	return ;
}

/*
 * desc: 网关控制组网设备重启或者触发ACSD或者其他。
 * type: POST-CON
 * form: 
 	{
		"data":
		{
			"ControlType": "Reboot"
		},
		"deviceId": "CMCC-30103-000C43E1762A"
 	}
 * TIPS: 
 * 
 */
void hnd_control_reboot(coap_context_t *ctx, struct coap_resource_t *resource, 
	      coap_address_t *peer, coap_pdu_t *request, str *token,
	      coap_pdu_t *response)
{
	unsigned char *data;
	size_t size;

	coap_get_data(request, &size, &data);
	VERIFY_DATA(size)
	else
	{
		andlink_add_option(&response);

		coap_printf("data: %s, len: %d\n", data, size);

		coap_printf("data: %s", data);
		cJSON *json = NULL;
		json = cJSON_Parse(data);
		if (!json) INVALID_VAL_RET;

		if (!cJSON_HasObjectItem(json, "deviceId") || !cJSON_HasObjectItem(json, "data"))
		{
			cJSON_Delete(json);
			LACK_ARG_RET;
		}

		if (check_gw_auth_info(json))
		{
			cJSON_Delete(json);
			AUTH_FAILED_RET;
		}

		cJSON *Jdata = NULL;
		Jdata = cJSON_GetObjectItem(json, "data");

		if (!cJSON_HasObjectItem(Jdata, "ControlType"))
		{
			cJSON_Delete(json);
			LACK_ARG_RET;
		}
		char *ControlType = NULL;
		ControlType = cJSON_GetObjectItem(Jdata, "ControlType")->valuestring;
		coap_printf("ControlType: %s", ControlType);
		if (!strcmp("Reboot", ControlType))
		{
			SUCCESS_NO_RET;
			//DO NOT USE THIS HERE
			//coap_send(ctx, peer, response);
			do_system("reboot");

			cJSON_Delete(json);
			return ;
		}
		/* ACSD */
		else if (!strcmp("2.4GChanReSelect", ControlType) || !strcmp("5GChanReSelect", ControlType))
		{
			/* do something about ACSD */
			SUCCESS_NO_RET;
			coap_send(ctx, peer, response);
			do_system("init_system restart");

			cJSON_Delete(json);
			return ;
		}
		else
		{
			cJSON_Delete(json);
			INVALID_CMD_RET;
		}

		cJSON_Delete(json);
		SUCCESS_RET;
	}

	return ;
}

/*
 * desc: 网关控制组网设备的指示灯开关
 * type: POST-CON
 * form: 
 	{
		"data":
		{
			"LEDOnOff": 1
		},
		"deviceId": "CMCC-30103-000C43E1762A"
 	}
 * TIPS: 
 * 
 */
void hnd_control_led_control(coap_context_t *ctx, struct coap_resource_t *resource, 
		coap_address_t *peer, coap_pdu_t *request, str *token, coap_pdu_t *response)
{
	unsigned char *data;
	size_t size;

	coap_get_data(request, &size, &data);
	VERIFY_DATA(size)
	else
	{
		andlink_add_option(&response);
		coap_printf("data: %s, len: %d\n", data, size);
		cJSON *json = NULL;
		json = cJSON_Parse(data);
		if (!json) INVALID_VAL_RET;

		if (!cJSON_HasObjectItem(json, "deviceId") || !cJSON_HasObjectItem(json, "data"))
		{
			cJSON_Delete(json);
			LACK_ARG_RET;
		}

		if (check_gw_auth_info(json))
		{
			cJSON_Delete(json);
			AUTH_FAILED_RET;
		}

		cJSON *Jdata = NULL;
		Jdata = cJSON_GetObjectItem(json, "data");

		if (!cJSON_HasObjectItem(Jdata, "LEDOnOff"))
		{
			cJSON_Delete(json);
			LACK_ARG_RET;
		}
		int LEDOnOff = -1;		
		LEDOnOff = cJSON_GetObjectItem(Jdata, "LEDOnOff")->valueint;
		coap_printf("ControlType: %d", LEDOnOff);

		//do something. Turn on/off LED. 1: on, 0: off
		if (0 == LEDOnOff)
		{
            nvram_bufset(RT2860_NVRAM, "LEDOnOff", "0");
			do_system("factory led off");
		}
		else if(1 == LEDOnOff)
		{
            nvram_bufset(RT2860_NVRAM, "LEDOnOff", "1");
			do_system("factory led recover");
		}
		else
		{
			coap_printf("");
			cJSON_Delete(json);
			UNSUPP_ARG_RET;
		}

		nvram_commit(RT2860_NVRAM);

		cJSON_Delete(json);
		SUCCESS_RET;
	}

	return ;
}

/*
 * desc: 网关控制组网设备的STA漫游功能
 * type: POST-CON
 * form: 
 	{
		"data": 
		{
			"RoamingSwitch": 1,
			"LowRSSI2.4G": -80,
			"LowRSSI5G": -80
		},
		"deviceId": "CMCC-30103-000C43E1762A"
 	}
 * TIPS: 
 * 
 */
void hnd_control_sta_roaming(coap_context_t *ctx, struct coap_resource_t *resource, 
		coap_address_t *peer, coap_pdu_t *request, str *token, coap_pdu_t *response)
{
	unsigned char *data;
	size_t size;

	coap_get_data(request, &size, &data);
	VERIFY_DATA(size)
	else
	{
		andlink_add_option(&response);
		coap_printf("data: %s, len: %d\n", data, size);
		cJSON *json = NULL;
		json = cJSON_Parse(data);
		if (!json) INVALID_VAL_RET;

		if (!cJSON_HasObjectItem(json, "deviceId") || !cJSON_HasObjectItem(json, "data"))
		{
			cJSON_Delete(json);
			LACK_ARG_RET;
		}

		if (check_gw_auth_info(json))
		{			
			cJSON_Delete(json);
			AUTH_FAILED_RET;
		}

		cJSON *Jdata = NULL;
		Jdata = cJSON_GetObjectItem(json, "data");

		if (!cJSON_HasObjectItem(Jdata, "RoamingSwitch"))
		{
			cJSON_Delete(json);
			LACK_ARG_RET;
		}

		int RoamingSwitch;
		RoamingSwitch = cJSON_GetObjectItem(Jdata, "RoamingSwitch")->valueint;
		char R_S_buf[16] = {0};
		sprintf(R_S_buf, "%d", RoamingSwitch);
		nvram_bufset(RT2860_NVRAM, "RoamingSwitch", R_S_buf);

		/* 2.4G */
		HAS_XXX(Jdata, "LowRSSI2.4G")
		{
			int LowRSSI24G;
			char L_2_buf[16] = {0};
			LowRSSI24G = cJSON_GetObjectItem(Jdata, "LowRSSI2.4G")->valueint;
			sprintf(L_2_buf, "%d", LowRSSI24G);
			nvram_bufset(RT2860_NVRAM, "LowRSSI24G", L_2_buf);
		}
		nvram_commit(RT2860_NVRAM);

#if defined (CONFIG_PRODUCT_341AA)
		/* 5G */
		HAS_XXX(Jdata, "LowRSSI5G")
		{
			int LowRSSI5G;
			char L_5_buf[16] = {0};
			LowRSSI5G = cJSON_GetObjectItem(Jdata, "LowRSSI5G")->valueint;
			sprintf(L_5_buf, "%d", LowRSSI5G);
			nvram_bufset(RTDEV_NVRAM, "LowRSSI5G", L_5_buf);
		}
		nvram_commit(RTDEV_NVRAM);
#endif

		coap_printf("RoamingSwitch: %d", 
				RoamingSwitch);

		SUCCESS_NO_RET;
		coap_send(ctx, peer, response);
		cJSON_Delete(json);
		do_system("init_system restart");

		return ;
	}

	return ;
}

/*
 * desc: 网关查询组网设备的配置信息
 * type: GET-CON
 * form: 
 {
    "deviceId": "DeviceID",
    "data": {
        "LEDOnOff": 1,
        "RoamingSwitch": 1,
        "LowRSSI2.4G": -80,
        "LowRSSI5G": -80,
        "SyncCode": "24"
        "Radios": [
            {
                "Radio": "2.4G",
                "Enable": 1,
                "TransmitPower": "100",
                "Channel": 6
            },
			{
				"Radio": "5G",
				"Enable": 1,
				"TransmitPower": "100",
				"Channel": 36
			}
        ],
        "Configurations": [
            {
                "Radio": "2.4G",
                "Index": 1,
                "Enable": 1,
                "SSID": "CMCC-2341",
                "SecurityMode": "WPA-WPA2-Personal",
                "Pwd": "12345678",
                "MaxAssociateNum": 10,
                "SSIDAdvertisementEnabled": 1
            },
            {
                "Radio": "5G",
                "Index": 1,
                "Enable": 1,
                "SSID": "CMCC-2341-5G",
                "SecurityMode": "WPA-WPA2-Personal",
                "Pwd": "12345678",
                "MaxAssociateNum": 10,
                "SSIDAdvertisementEnabled": 1
            }
        ]
    }
}
 * TIPS: 详见<集采组网设备接口>文档
 * 
 */

void hnd_query_cfg_info(coap_context_t *ctx, struct coap_resource_t *resource, 
		coap_address_t *peer, coap_pdu_t *request, str *token, coap_pdu_t *response)
{
	andlink_add_option(&response);

	char *jsonValue = get_ApConfigInfo(HOME_GATEWAY, NULL);
	if (!jsonValue) UNKNOWN_ERR_RET;
	
	/* support Block-Wise */
	coap_payload_t *test_payload;
	test_payload = coap_new_payload(strlen(jsonValue)+1);
	if (!test_payload)
		coap_printf("cannot allocate resource");
	else {
		test_payload->length = strlen(jsonValue);
		memcpy(test_payload->data, jsonValue, test_payload->length);
		test_payload->flags |= REQUIRE_ETAG;
		coap_add_payload(resource->key, test_payload, NULL);
	}

	coap_block_wise_resource(ctx, resource, peer, request, token, response);
	coap_delete_payload(test_payload);
	//coap_add_data(response, strlen(jsonValue), (unsigned char *)jsonValue);

	free(jsonValue);
	return ;
}

/*
 * desc: 网关查询组网设备的上联状态
 * type: GET-CON
 * form: 
 {
    "deviceId": "",
    "data": {
        "UplinkType": "Ethernet",
        "MacAddress": "F4:4C:70:51:1A:16",
        "Radio": "2.4G",
        "SSID": "CMCC-4321",
        "Channel": 6,
        "Noise": "-90",
        "SNR": "35",
		"RSSI": "-55",
		"RxRate": "130",
		"TxRate": "130"
    }
}
 * TIPS: 详见<集采组网设备接口>文档
 * 
 */
void hnd_query_ap_uplink(coap_context_t *ctx, struct coap_resource_t *resource, 
		coap_address_t *peer, coap_pdu_t *request, str *token, coap_pdu_t *response)
{
	 andlink_add_option(&response);
 
 	char *jsonValue = get_UplinkStatus(HOME_GATEWAY, NULL);
 	if (!jsonValue) UNKNOWN_ERR_RET;

	/* support Block-Wise */
	coap_payload_t *test_payload;
	test_payload = coap_new_payload(strlen(jsonValue)+1);
	if (!test_payload)
		coap_printf("cannot allocate resource");
	else {
		test_payload->length = strlen(jsonValue);
		memcpy(test_payload->data, jsonValue, test_payload->length);
		test_payload->flags |= REQUIRE_ETAG;
		coap_add_payload(resource->key, test_payload, NULL);
	}

	coap_block_wise_resource(ctx, resource, peer, request, token, response);
	coap_delete_payload(test_payload);
	//coap_add_data(response, strlen(jsonValue), (unsigned char *)jsonValue);
 
	 free(jsonValue);
	 return;
}


/*
 * desc: 网关查询组网设备的下挂设备信息列表。
 * type: GET-CON
 * form: 
 {
	 "deviceId": "",
	 "data": {
		 "Devices": [
		 	{
		 		"MacAddress": "F4:4C:70:51:1A:16",
		 		"VMacAddress": "F4:4C:70:51:1A:16",
		 		"UpTime": "60",
		 		"Radio": "2.4G",
		 		"SSID": "CMCC-4321",
		 		"RSSI": "-60",
		 		"RxRate": "0",
		 		"TxRate": "0"
		 	},
		 	.
		 	.
		 	.
		 ]
	 }
 }
 * TIPS: 
 	1、详见<集采组网设备接口>文档
 	2、coap默认最大传输数据限制为1024个字节，目前还不支持块传输，所以多几个客户端这里就有问题。
 * 
 */
void hnd_query_sta_info(coap_context_t *ctx, struct coap_resource_t *resource, 
		coap_address_t *peer, coap_pdu_t *request, str *token, coap_pdu_t *response)
{
	andlink_add_option(&response);
	
 	char *jsonValue = get_STAInfo(HOME_GATEWAY, NULL);
 	if (!jsonValue) UNKNOWN_ERR_RET;

	/* support Block-Wise */
	coap_payload_t *test_payload;
	test_payload = coap_new_payload(strlen(jsonValue)+1);
	if (!test_payload)
		coap_printf("cannot allocate resource");
	else {
		test_payload->length = strlen(jsonValue);
		memcpy(test_payload->data, jsonValue, test_payload->length);
		test_payload->flags |= REQUIRE_ETAG;
		coap_add_payload(resource->key, test_payload, NULL);
	}

	coap_block_wise_resource(ctx, resource, peer, request, token, response);
	coap_delete_payload(test_payload);
	//coap_add_data(response, strlen(jsonValue), (unsigned char *)jsonValue);

    free(jsonValue);
	return ;
}


/*
 * desc: 网关查询组网设备的WiFi报文收发
 * type: GET-CON
 * form: 
 {
	 "deviceId": "",
	 "data": {
		 "Stats": [
			 {
				 "Radio": "2.4G",
				 "Index": 1,
				 "SSID": "CMCC-s3As",
				 "TotalBytesSent": 6391,
				 "TotalBytesReceived": 39556,
				 "TotalPacketsSent": 74,
				 "TotalPacketsReceived": 485,
				 "ErrorsSent": 0,
				 "ErrorsReceived": 0,
				 "DiscardPacketsSent": 0,
				 "DiscardPacketsReceived": 0
			 },
			 {
				 "Radio": "5G",
				 "Index": 1,
				 "SSID": "CMCC-s3As-5G",
				 "TotalBytesSent": 6391,
				 "TotalBytesReceived": 39556,
				 "TotalPacketsSent": 74,
				 "TotalPacketsReceived": 485,
				 "ErrorsSent": 0,
				 "ErrorsReceived": 0,
				 "DiscardPacketsSent": 0,
				 "DiscardPacketsReceived": 0
			 }
		 ]
	 }
 }
 * TIPS: 详见<集采组网设备接口>文档
 * 
 */
void hnd_query_dev_statis(coap_context_t *ctx, struct coap_resource_t *resource, 
		coap_address_t *peer, coap_pdu_t *request, str *token, coap_pdu_t *response)
{
	andlink_add_option(&response);

 	char *jsonValue = get_WiFIStats(HOME_GATEWAY, NULL);
 	if (!jsonValue) UNKNOWN_ERR_RET;

	/* support Block-Wise */
	coap_payload_t *test_payload;
	test_payload = coap_new_payload(strlen(jsonValue)+1);
	if (!test_payload)
		coap_printf("cannot allocate resource");
	else {
		test_payload->length = strlen(jsonValue);
		memcpy(test_payload->data, jsonValue, test_payload->length);
		test_payload->flags |= REQUIRE_ETAG;
		coap_add_payload(resource->key, test_payload, NULL);
	}

	coap_block_wise_resource(ctx, resource, peer, request, token, response);
	coap_delete_payload(test_payload);
	//coap_add_data(response, strlen(jsonValue), (unsigned char *)jsonValue);

	free(jsonValue);
	return ;
}


/*
 *  Creat new resource, and regist methods, handlers for URI.
 * 
 *  return: NULL
 */
void andlink_new_resource(coap_context_t **ctx,
		const unsigned char *uri, size_t ulen,
		unsigned char method, coap_method_handler_t handler,
		const unsigned char *a_name, size_t a_nlen)
{
	coap_resource_t *r;
	r = coap_resource_init((unsigned char *)uri, ulen, 0);
	coap_register_handler(r, method, handler);
	coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)a_name, a_nlen, 0);
	coap_add_resource(*ctx, r);
}

void init_andlink_resources(coap_context_t **ctx)
{
	/* qlink */
	andlink_new_resource(ctx, QLINK_SEARCHACK, strlen(QLINK_SEARCHACK), COAP_REQUEST_POST, 
		hnd_qlink_searchack, QLINK_SEARCHACK_M, strlen(QLINK_SEARCHACK_M));
	andlink_new_resource(ctx, QLINK_NETINFO, strlen(QLINK_NETINFO), COAP_REQUEST_POST, 
		hnd_qlink_netinfo, QLINK_NETINFO_M, strlen(QLINK_NETINFO_M));
	/* regist */
	/*
	andlink_new_resource(ctx, DEV_CONTROL, strlen(DEV_CONTROL), COAP_REQUEST_POST, 
		hnd_regist_dev_control, DEV_CONTROL_M, strlen(DEV_CONTROL_M));
	andlink_new_resource(ctx, DEV_DATA, strlen(DEV_DATA), COAP_REQUEST_GET, 
		hnd_regist_get_dev_data, DEV_DATA_M, strlen(DEV_DATA_M));
	*/
	andlink_new_resource(ctx, DEV_UNBIND, strlen(DEV_UNBIND), COAP_REQUEST_POST, 
		hnd_regist_dev_unbind, DEV_UNBIND_M, strlen(DEV_UNBIND_M));
	andlink_new_resource(ctx, DEV_FILE, strlen(DEV_FILE), COAP_REQUEST_POST, 
		hnd_regist_dev_file, DEV_FILE_M, strlen(DEV_FILE_M));
	/* control */
	andlink_new_resource(ctx, WIFI_PARA_SYNC, strlen(WIFI_PARA_SYNC), COAP_REQUEST_POST, 
		hnd_control_wifi_para_sync, WIFI_PARA_SYNC_M, strlen(WIFI_PARA_SYNC_M));
	andlink_new_resource(ctx, WIFI_SWTICH, strlen(WIFI_SWTICH), COAP_REQUEST_POST, 
		hnd_control_wifi_swtich, WIFI_SWTICH_M, strlen(WIFI_SWTICH_M));
	andlink_new_resource(ctx, WIFI_RADIO, strlen(WIFI_RADIO), COAP_REQUEST_POST, 
		hnd_control_wifi_radio, WIFI_RADIO_M, strlen(WIFI_RADIO_M));
	andlink_new_resource(ctx, WIFI_WPS, strlen(WIFI_WPS), COAP_REQUEST_POST, 
		hnd_control_wifi_wps, WIFI_WPS_M, strlen(WIFI_WPS_M));
	andlink_new_resource(ctx, MAC_FILTER, strlen(MAC_FILTER), COAP_REQUEST_POST, 
		hnd_control_mac_filter, MAC_FILTER_M, strlen(MAC_FILTER_M));
	andlink_new_resource(ctx, DEV_REBOOT, strlen(DEV_REBOOT), COAP_REQUEST_POST, 
		hnd_control_reboot, DEV_REBOOT_M, strlen(DEV_REBOOT_M));
	andlink_new_resource(ctx, LED_CONTROL, strlen(LED_CONTROL), COAP_REQUEST_POST, 
		hnd_control_led_control, LED_CONTROL_M, strlen(LED_CONTROL_M));
	andlink_new_resource(ctx, STA_ROAMING, strlen(STA_ROAMING), COAP_REQUEST_POST, 
		hnd_control_sta_roaming, STA_ROAMING_M, strlen(STA_ROAMING_M));

	andlink_new_resource(ctx, AP_CFG_INFO, strlen(AP_CFG_INFO), COAP_REQUEST_GET, 
		hnd_query_cfg_info, AP_CFG_INFO_M, strlen(AP_CFG_INFO_M));
	andlink_new_resource(ctx, AP_UPLINK, strlen(AP_UPLINK), COAP_REQUEST_GET, 
		hnd_query_ap_uplink, AP_UPLINK_M, strlen(AP_UPLINK_M));
	andlink_new_resource(ctx, STA_INFO, strlen(STA_INFO), COAP_REQUEST_GET, 
		hnd_query_sta_info, STA_INFO_M, strlen(STA_INFO_M));
	andlink_new_resource(ctx, DEV_STATIS, strlen(DEV_STATIS), COAP_REQUEST_GET, 
		hnd_query_dev_statis, DEV_STATIS_M, strlen(DEV_STATIS_M));
}


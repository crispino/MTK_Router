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
 * desc: ��֤�����·��ļ�Ȩ��Ϣ��
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
 * desc: �����յ��豸�㲥��������Ӧ��
 * type: POST-CON
 * form: 
 	{
		"searchAck":"ANDLINK-GW", //my question.�����������
		"andlinkVersion":"V2"
 	}
 * TIPS: my question.�ýӿ���Ҫ��ͨ������Ӧ���Ƿ���GWIP��
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

	/* �����·��ļ���ģʽ: OPEN/WEP/WPAPSK/WPAPSK2/MIXED-WPAPSK2��Ϊ�ձ�ʾMIXED-WPAPSK2 */
	/* �м���Ҫ�ļ���ģʽ: OPEN/SHARED/WPAPSK/WPA2PSK */
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
	
	/* Q-LINK 4: �����豸�л�����ͨ���� */
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

	/* �����·��ļ���ģʽ: OPEN/WEP/WPAPSK/WPAPSK2/MIXED-WPAPSK2��Ϊ�ձ�ʾMIXED-WPAPSK2 */
	/* ����ʹ�õļ���ģʽ: OPEN/WEP/WPAPSK/WPA2PSK/WPAPSKWPA2PSK��Ϊ�ձ�ʾWPAPSKWPA2PSK */
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

	/* Q-LINK 4: ������Ϊ�м��豸�����������������������һ�¡� */
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
 * desc: ����ͨ���˽ӿڷ�����ʽ����ͨ�����豸��
 * type: POST-CON
 * form: 
 	{
		"SSID":"",
		"password":"",
		"encrypt":""
 	}
 * TIPS: 
 	1���˽ӿ����豸δע��ǰ�����ķ��͡��豸ע��󣬻���ܷ��͡�//my question.̫����
	2����ԿΪע���ȡ��deviceID��
 * 
 */
/* Q-LINK 3: ���ط�������SSID�����롣 */
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

		//ΪAPP��������
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
		coap_printf(
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
 * desc: ��Ӧ����/device/command/control�Ŀ�����Ϣ��
 * type: POST-CON
 * form: 
 	{
		"deviceId":"",
		"childDeviceId":"", //Ϊ����˵���ǿ����豸�����ǿ������豸��
		//�����¼�������ֵ
		"data":{
			"params": [
				{"paramCode": "SystemControl", "paramValue": "Reboot"}, //�������ӣ��ֱ���������͸�λ��
				{"paramCode": "SystemControl", "paramValue": "Reset"} //��������
			]
		}
 	}
 * TIPS: ˵����Ӧ��ʽ��
 		1�������ѽ��ܲ�������ɣ�ֻ����״̬�룬û�к�����Ӧ����:{"respCode":1 ��������0ֵ��1000��Ȩ�쳣��}
 		2�������ѽ��ܲ��ȴ������������������ͨ���ϱ���Ϣ�ӿ�(device_inform_data)�첽���ء�{"respCode":0}
 		3�������ѽ��ܲ�������ɣ�ͬ�����ظ��º�����ݣ����豸Ӧ��������form�и�ʽ�����ݡ�
 
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

		/* do something. params�еĲ�����һ��������ʽ�� */
		char *params = NULL;
		//params = cJSON_GetArrayItem(json, "params"); //json����ν������顣
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
 * desc: �����豸�ϱ���ǰ�������ݣ�RESERVED
 * type: POST-CON
 * form: 
 	{
		"deviceId":"",
		"childDeviceId":"", //Ϊ����˵���Ǵ����豸�����������豸
		"data":{ //��ѡ������Ϊ�����ϱ�ָ������ֵ��
			"paramCodes": [
				{"paramCode": "PARAMSCODE1"}, //����Code�ϱ�����ֵ
				{"paramCode": "PARAMSCODE2"}, //����Code�ϱ�����ֵ
				...
			]
		}
 	}
 * TIPS: ˵����Ӧ��ʽ��
 		1����������쳣������{"respCode":>1}���쳣��
 		2�������ѽ��ܵȴ�(device_inform_data)�첽���ء�����{"respCode":0}
 		3�������ѽ��ܲ�������ɣ�ͬ�����ظ��º�����ݣ����豸Ӧ��������form�и�ʽ�����ݡ�
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

		/* do something. params�еĲ�����һ��������ʽ�� */
		char *params = NULL;
		//params = cJSON_GetArrayItem(json, "params"); //json����ν������顣
		/*
		 ...
		 */
		//char *paramCode = NULL;
		//paramCode = cJSON_GetObjectItem(params, "paramCode")->valuestring;
		//����ͨ��nvramȡparamcode��ֵ������������ʽ��
		//get_paramcode_value();

		cJSON_Delete(json);
		SUCCESS_RET;
	}

	return ;
}
#endif

/*
 * desc: �Ӵ��豸�������ϵİ󶨹�ϵ�������豸���豸�ϵİ󶨹�ϵ��
 * type: POST-CON
 * form: 
 	{
		"deviceId": "",
		"childDeviceId": "" //������Ϊ����豸��
 	}
 
 * TIPS: ���ʱ���豸����Ҫ����Щ�£���Ҫ��ͨ��this is my question, do something
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
 * desc: �豸�ļ�����
 * type: POST-CON
 * form: 
 	{
		"deviceId": "",
		"childDeviceId": "", //��ѡ��������������豸���ļ�������
		"fileMode": "", //DOWN�������ļ���UP���ϴ��ļ�
		"fileType": "", //��ѡ����FIRM�̼���APPӦ�ã�LOG��־�ļ���CFG�����ļ���what the fuck!
		"fileUrl": "http://xxx/xxx", //�豸�ļ�����/�ϴ�·����
		"downMode": 1 //��ΪDOWN������1����������0�´�����������
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
		fileUrl = cJSON_GetObjectItem(json, "fileUrl")->valuestring;
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
 * desc: ���ؽ�WiFi������Ϣ�·��������豸
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

			/* ���������WiFi����������Ч�� */
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
 * desc: ���ؿ��������豸��WiFi����
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
 * desc: ���ؿ��������豸��WiFi����
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
 * desc: ���ؼ��������豸��WPS
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
 * desc: �����·��ڰ������������豸
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
 * TIPS: ���Կ���MacFilterEntries�ĸ�ʽ��ÿ���������MAC���豸��������������Զ��Ÿ�����
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
 * desc: ���ؿ��������豸�������ߴ���ACSD����������
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
 * desc: ���ؿ��������豸��ָʾ�ƿ���
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
 * desc: ���ؿ��������豸��STA���ι���
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
 * desc: ���ز�ѯ�����豸��������Ϣ
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
 * TIPS: ���<���������豸�ӿ�>�ĵ�
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
 * desc: ���ز�ѯ�����豸������״̬
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
 * TIPS: ���<���������豸�ӿ�>�ĵ�
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
 * desc: ���ز�ѯ�����豸���¹��豸��Ϣ�б���
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
 	1�����<���������豸�ӿ�>�ĵ�
 	2��coapĬ���������������Ϊ1024���ֽڣ�Ŀǰ����֧�ֿ鴫�䣬���Զ༸���ͻ�������������⡣
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
 * desc: ���ز�ѯ�����豸��WiFi�����շ�
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
 * TIPS: ���<���������豸�ӿ�>�ĵ�
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

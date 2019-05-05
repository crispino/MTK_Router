/* andlink-request -- andlink client request interface
 *
 * Copyright (C) 2017 zowee technology Co., Ltd
 *
 * This file is some request interface for andlink plugin in DEVICE.
 * 
 * error code:
 	1:		Success
 	0:		Failed
 	1000:	Authorization failed
 	1001:	Invalid argument
 	1002:	Lack of argument
 	1003:	Invalid value
 	1004:	Invalid command
 *
 *
 */

#include "andlink.h"
#include "router_cfg.h"

int quick_repeater(char *ssid, char *encrypt, char *type, char *passwd)
{
	int ret = 1;

	do_system("iwpriv apcli0 set ApCliEnable=%d", 0);
	do_system("iwpriv apcli0 set ApCliBssid=");
	do_system("iwpriv apcli0 set ApCliSsid=%s", ssid);
	do_system("iwpriv apcli0 set ApCliAuthMode=%s", encrypt);
	/* Not OPEN mode */
	if (0 != strcmp(encrypt, "OPEN"))
	{
		/* How can I get the EncrypType, default is AES */
		do_system("iwpriv apcli0 set ApCliEncrypType=%s", type);
		do_system("iwpriv apcli0 set ApCliWPAPSK=%s", passwd);
	}
	do_system("iwpriv apcli0 set ApCliEnable=%d", 1);
	do_system("iwpriv apcli0 set ApCliAutoConnect=1");

	return ret;
}

/* my question: After request, ignore all of the response from GW, is that OK? */
/* do something. take response if you need, So change response as a ARG */
static inline int parse_response(const char *url, char *data)
{
	char *response = NULL;
	int replen;
	if (AL_ERROR != Qlink_SendCoapRequest(url, COAP_REQUEST_POST, data, &response, &replen))
	{
		coap_printf("response: %s", response);
		if (response)
		{
			coap_printf("url: %s, response: %s", url, response);
			cJSON *json = NULL;
			json = cJSON_Parse(response);
			if (!json)
			{
				free(response);
				return AL_JSON_ERR;
			}

			AL_RESP_SUCC(json)
			{
				free(response);
				cJSON_Delete(json);
				return AL_SUCCESS;
			}
			else
			{
				int ret = AL_FAILED;
				if(cJSON_HasObjectItem(json, "respCode")) {
					ret = cJSON_GetObjectItem(json, "respCode")->valueint;
				}
				free(response);
				cJSON_Delete(json);
				return ret;
			}
		}
	}

	if (response) free(response);
	return AL_ERROR;
}

/*
 * desc: 设备广播请求网关IP地址。
 * type: POST-NON
 * form: 
 	{
		"searchKey":"ANDLINK-DEVICE", //广播发现的key，约定为该值
		"andlinkVersion":"V2"
 	}
 * TIPS: 
 	1、设备上电后，发现本地没有保存网关IP地址，则调用本接口。且在获取到网关IP地址后，保存到本地。
 	2、该消息类型为POST-NON。所以网关无需响应，网关识别合法后会通过coap://deviceIP/qlink/searchack接口异步响应
 * 
 */
int qlink_search_gw(const char *deviceId, const char *gwip)
{
	/* my question. How to get broadcast IP */
	char broadcast_ip[32] = {0};
	//get_gw_broadcast_ip(gwip, broadcast_ip);
	CREATE_URL(broadcast_ip, QLINK_SEARCHGW);

	int ret = -1;
	cJSON *json = NULL;
	char *data = NULL;
	json = cJSON_CreateObject();
	if (!json) return AL_JSON_ERR;

	cJSON_AddItemToObject(json, "searchKey", cJSON_CreateString("ANDLINK-DEVICE"));
	cJSON_AddItemToObject(json, "andlinkVersion", cJSON_CreateString("V2"));

	data = cJSON_PrintUnformatted(json);
	coap_printf("%s", data);

	ret = parse_response(url, data);

	free(data);
	cJSON_Delete(json);
	return ret;
}

/*
 * desc: 使用AP+Client 连接上级的方法，连接引导通道。默认SSID: CMCC_QLINK，无密码。
 * type: 无
 * form: 无
 * TIPS: 
	1、设备应先执行本接口建立通道，再执行网关发现接口。
 	my question 现在软件还不支持AP+Client
 * 
 */
void qlink_join_default_ssid(void)
{
	

	return ;
}

/*
 * desc: 设备向网关请求获取入网配置信息
 * type: POST-CON
 * form: 
 	{
 		"deviceMac":"",
 		"deviceType":"" //开发者在平台注册的设备类型码
 	}
 * TIPS: 设备连接引导通道成功后请求入网配置信息
 * response: 
 	{
		"respCode": xxx, //1表示成功，否则表示其他错误
		"respCont": "invalid argument" //respCode=1时忽略
 	}
 *
 * 
 */
int qlink_request(const char *deviceId, const char *gwip)
{
	CREATE_URL(gwip, QLINK_REQUEST);

	int ret = -1;
	cJSON *json = NULL;
	char *data = NULL;
	json = cJSON_CreateObject();
	if (!json) return AL_JSON_ERR;

	cJSON_AddItemToObject(json, "deviceMac", cJSON_CreateString(g_dev_mac));
	cJSON_AddItemToObject(json, "deviceType", cJSON_CreateString(g_dev_type));

	data = cJSON_PrintUnformatted(json);

	coap_printf("%s", data);
	ret = parse_response(url, data);
	
	free(data);
	cJSON_Delete(json);
	return ret;
}

/*
 * desc: 使用AP+Client 连接上级的方法，连接网关通过coap://deviceIP/qlink/netinfo接口下发的SSID。
 * type: 无
 * form: 无
 * TIPS: 
 * 
 */
void qlink_join_ssid(void)
{
	

	return ;
}

/*
 * desc: 设备入网成功，通过网关
 * type: POST-CON
 * form: 
	{
		"deviceMac":"",
		"deviceType":"" //开发者在平台注册的设备类型码
	}
 * TIPS: 在设备入网成功后，应该开始设备注册(若已注册，则应该开始设备上线)
 * Just response error code.
 * 
 */
int qlink_success(const char *deviceId, const char *gwip)
{
	CREATE_URL(gwip, QLINK_SUCCESS);

	int ret = -1;
	cJSON *json = NULL;
	char *data = NULL;
	json = cJSON_CreateObject();
	if (!json) return AL_JSON_ERR;

	cJSON_AddItemToObject(json, "deviceMac", cJSON_CreateString(g_dev_mac));
	cJSON_AddItemToObject(json, "deviceType", cJSON_CreateString(g_dev_type));

	data = cJSON_PrintUnformatted(json);
	coap_printf("%s", data);

	ret = parse_response(url, data);
	
	free(data);
	cJSON_Delete(json);
	return ret;
}

/*
 * desc: 设备注册
 * type: POST-CON
 * form: 
 	{
		"deviceMac":"", //若为普通上线则不填
		"deviceType":"", //
		"productToken":"" //
		"timestamp":""
 	}
 * TIPS: 
 	1、子设备注册请求格式略有区别，多了childDeviceId的参数。
 	2、设备上线时，或WiFi快连配置成功后，检测到没有deviceId和deviceToken，则执行首次上线注册。
 * 
 */
int device_inform_regist(const char *gwip)
{
	CREATE_URL(gwip, DEV_REGIST);

	cJSON *json = NULL;
	cJSON *XData = NULL;
	char *data = NULL;
	json = cJSON_CreateObject();
	if (!json) return AL_JSON_ERR;

	cJSON_AddItemToObject(json, "deviceMac", cJSON_CreateString(g_dev_mac)); //设备MAC与deviceId一一对应
	cJSON_AddItemToObject(json, "deviceType", cJSON_CreateString(g_dev_type));
	cJSON_AddItemToObject(json, "productToken", cJSON_CreateString(nvram_get(RT2860_NVRAM, "andlink_productToken")));
	cJSON_AddItemToObject(json, "timestamp", cJSON_CreateNumber(router_getCompileTimeStamp()));

	data = cJSON_PrintUnformatted(json);
	coap_printf("%s", data);

	int ret = -1;
	char *response = NULL;
	int replen;
	/* Send successful */
	ret = Qlink_SendCoapRequest(url, COAP_REQUEST_POST, data, &response, &replen);
	coap_printf("response: %s", response);
	free(data);
	cJSON_Delete(json);
	json = NULL;

	if (AL_ERROR != ret)
	{
		if (response)
		{
			json = cJSON_Parse(response);
			if (!json)
			{
				free(response);
				return AL_JSON_ERR;
			}

			AL_RESP_FAILED(json)
			{
				if (response) free(response);
				cJSON_Delete(json);
				return AL_ERROR;
			}
			else
			{
				char *device_id = NULL, *device_token = NULL, *andlink_token = NULL, *gw_token = NULL;
				int timestamp, heartbeat;
				/* do something. Config these args */
				HAS_XXX(json, "deviceId")
				{
					device_id = cJSON_GetObjectItem(json, "deviceId")->valuestring;
					nvram_bufset(RT2860_NVRAM, "andlink_deviceId", device_id);
				}
				HAS_XXX(json, "deviceToken")
				{
					device_token = cJSON_GetObjectItem(json, "deviceToken")->valuestring;
					nvram_bufset(RT2860_NVRAM, "andlink_deviceToken", device_token);
				}
				HAS_XXX(json, "andlinkToken")
				{
					/* do something check value */
					andlink_token = cJSON_GetObjectItem(json, "andlinkToken")->valuestring;
					nvram_bufset(RT2860_NVRAM, "andlink_andlinkToken", andlink_token);
				}
				HAS_XXX(json, "timestamp")
				{
					timestamp = cJSON_GetObjectItem(json, "timestamp")->valueint;
					char timestamp_buf[32] = {0};
					sprintf(timestamp_buf, "%d", timestamp);
					//nvram_bufset(RT2860_NVRAM, "andlink_timestamp", timestamp_buf);
				}
				HAS_XXX(json, "heartbeatTime")
				{
					heartbeat = cJSON_GetObjectItem(json, "heartbeatTime")->valueint;
					char heartbeat_buf[32] = {0};
					sprintf(heartbeat_buf, "%d", heartbeat);
					nvram_bufset(RT2860_NVRAM, "andlink_heartbeatTime", heartbeat_buf);
				}
				/* 用于加密上行数据的秘钥。 */
				HAS_XXX(json, "gwToken")
				{
					gw_token = cJSON_GetObjectItem(json, "gwToken")->valuestring;
					nvram_bufset(RT2860_NVRAM, "andlink_gwToken", gw_token);
				}

				nvram_commit(RT2860_NVRAM);
				cJSON_Delete(json);
			}
		}
	}

	if (response) free(response);
	return ret;
}

/*
 * desc: 设备/子设备上线
 * type: POST-CON
 * form: 
 	{
		"deviceId":"",
		"childDeviceId":"", //非子设备则不填
		"deviceType":"",
		"firmwareVersion":"",
		"softwareVersion":"",
		"ipAddress":"", //没有则不填
		"timestamp": ,
		"XData": //扩展属性
		{
			"deviceVendor": "ZOWEE",
			"deviceSn": "",
			"apUplinkType": "" //上行接入方式: wireless,ethernet，PoE，PLC，Cable
			"radio5": "0", //是否支持5G
			"SyncCode": * //配置同步码。my question, what is this mean.
		}
 	}
 * TIPS: response 中有个参数决定后续的数据包是否加密，注意。
 * response: 
 	{
		"respCode": 1, //参考error code table
		"encrypt": 0 //可选，后续报文是否加密，默认为1加密。
 	}
 *
 * 
 */
int device_inform_online(const char *deviceId, const char *gwip)
{
	CREATE_URL(gwip, DEV_ONLINE);
	
	cJSON *json = NULL;
	cJSON *XData = NULL;
	char *data = NULL;
	json = cJSON_CreateObject();
	if (!json) return AL_JSON_ERR;

	cJSON_AddItemToObject(json, "deviceId", cJSON_CreateString(deviceId)); //do something. Regist success, we will get deviceId.
	cJSON_AddItemToObject(json, "deviceMac", cJSON_CreateString(g_dev_mac)); //设备MAC与deviceId一一对应
	//cJSON_AddItemToObject(json, "childDeviceId", cJSON_CreateString("")); //没有则不填
	cJSON_AddItemToObject(json, "deviceType", cJSON_CreateString(g_dev_type));
	
	FILE *fp = fopen("/etc_ro/version", "r");
	char version[64] = {S_VERSION};
	if (fp != NULL) {fgets(version, sizeof(version), fp); version[strlen(version)-1] = '\0'; fclose(fp);}
	cJSON_AddItemToObject(json, "firmwareVersion", cJSON_CreateString(H_VERSION));
	cJSON_AddItemToObject(json, "softwareVersion", cJSON_CreateString(version)); //多个版本号则用分号隔开?
	/* 确保IP地址准确 */
	char *ifname = NULL;
	ifname = get_ifname();
	cJSON_AddItemToObject(json, "ipAddress", cJSON_CreateString(get_ipaddr("br0")));
	cJSON_AddItemToObject(json, "timestamp", cJSON_CreateNumber(router_getCompileTimeStamp()));
	cJSON_AddItemToObject(json, "XData", XData = cJSON_CreateObject());
	cJSON_AddItemToObject(XData, "deviceVendor", cJSON_CreateString(nvram_get(RT2860_NVRAM, "andlink_deviceVendor")));
#if defined (CONFIG_PRODUCT_341AA)
	cJSON_AddItemToObject(XData, "deviceModel", cJSON_CreateString("HN120"));
#elif defined (CONFIG_PRODUCT_340AA)
	cJSON_AddItemToObject(XData, "deviceModel", cJSON_CreateString("HN100"));
#elif defined (CONFIG_PRODUCT_339AA)
	cJSON_AddItemToObject(XData, "deviceModel", cJSON_CreateString("HN350"));
#elif defined (CONFIG_PRODUCT_331AA)
	cJSON_AddItemToObject(XData, "deviceModel", cJSON_CreateString("HN330"));
#else
	cJSON_AddItemToObject(XData, "deviceModel", cJSON_CreateString("HN100"));
#endif

	char deviceSn[18] = "131118050000001"; //default
	router_getDevSn(deviceSn, sizeof(deviceSn));
	cJSON_AddItemToObject(XData, "deviceSn", cJSON_CreateString(deviceSn));

	/* Wireless、Ethernet、PLC、Cable */
	/* Reserved for PLC product */
#if defined (CONFIG_PRODUCT_XXXAA)
	cJSON_AddItemToObject(XData, "apUplinkType", cJSON_CreateString("PLC"));
#else
	if (3 == atoi(nvram_get(RT2860_NVRAM, "OperationMode")))
		cJSON_AddItemToObject(XData, "apUplinkType", cJSON_CreateString("Wireless"));
	else
		cJSON_AddItemToObject(XData, "apUplinkType", cJSON_CreateString("Ethernet"));
#endif

#if defined (CONFIG_PRODUCT_341AA)
	cJSON_AddItemToObject(XData, "radio5", cJSON_CreateNumber(1));
#else
	cJSON_AddItemToObject(XData, "radio5", cJSON_CreateNumber(0));
#endif
	cJSON_AddItemToObject(XData, "SyncCode", cJSON_CreateString(nvram_get(RT2860_NVRAM, "SyncCode")));

	data = cJSON_PrintUnformatted(json);
	coap_printf("%s", data);

	int ret = -1;
	char *response = NULL;
	int replen;
	/* Send successful */
	/* Should encrypt data by AES_CBC */
	ret = Qlink_SendCoapRequest(url, COAP_REQUEST_POST, data, &response, &replen);

	free(data);
	cJSON_Delete(json);

	if (AL_ERROR != ret)
	{
		if (response)
		{
			json = cJSON_Parse(response);
			if (!json)
			{
				free(response);
				return AL_JSON_ERR;
			}

			AL_RESP_SUCC(json)
			{
				/* my question. 该返回值需要沟通。因为响应参数中包含，是否加密，只能这么，而不能用通用的接口 */
				int encrypt;
				HAS_XXX(json, "encrypt")
				{
					encrypt = cJSON_GetObjectItem(json, "encrypt")->valueint;
					coap_printf("ENCRYPT is %d!!!", encrypt);
				}
				cJSON_Delete(json);
			}
			else
			{
				ret = AL_FAILED;
				if(cJSON_HasObjectItem(json, "respCode")) {
					ret = cJSON_GetObjectItem(json, "respCode")->valueint;
				}
				free(response);
				cJSON_Delete(json);
				return ret;
			}
		}
	}

	if (response) free(response);
	return ret;
}

/*
 * desc: 设备心跳
 * type: POST-CON
 * form: 
 	{
		"deviceId":""
 	}
 * TIPS: 设备/子设备注册的响应消息中包含心跳周期,heartbeat time
 * 
 */
int device_inform_heartbeat(const char *deviceId, const char *gwip)
{
	CREATE_URL(gwip, DEV_HEARTBEAT);

	int ret = -1;
	cJSON *json = NULL;
	char *data = NULL;
	json = cJSON_CreateObject();
	if (!json) return AL_JSON_ERR;

	cJSON_AddItemToObject(json, "deviceId", cJSON_CreateString(deviceId));

	data = cJSON_PrintUnformatted(json);
	coap_printf("%s", data);

	/* Should encrypt data by AES_CBC */
	ret = parse_response(url, data);
	
	free(data);
	cJSON_Delete(json);
	return ret;
}

/*
 * desc: 设备/子设备参数、通知、响应。
 	除了上下线、心跳消息外，设备主动上报的其他信息都通过此接口上报。
 * type: POST-CON
 * form: 
 	{
		"deviceId":"",
		"eventType":"", //事件类型
		"timestamp": 734354788, 
		"data":{ //各个事件的数据值
			"respCode": 2002, 
			"restCont": "20"
		}
 	}
 * TIP: 暂无该接口，根据需要扩展，RESERVED
 *
 */
int device_inform_data(const char *deviceId, const char *gwip, char *event_type, int event_code, char *event_cont)
{
	CREATE_URL(gwip, DEV_NOTICE);

	int ret = -1;
	cJSON *json = NULL;
	cJSON *data = NULL;
	char *jsonValue = NULL;
	json = cJSON_CreateObject();
	if (!json) return AL_JSON_ERR;

	cJSON_AddItemToObject(json, "deviceId", cJSON_CreateString(deviceId));
	cJSON_AddItemToObject(json, "eventType", cJSON_CreateString(event_type));
	cJSON_AddItemToObject(json, "timestamp", cJSON_CreateNumber(router_getCompileTimeStamp()));

	cJSON_AddItemToObject(json, "data", data = cJSON_CreateObject());
	cJSON_AddItemToObject(data, "respCode", cJSON_CreateNumber(event_code));
	cJSON_AddItemToObject(data, "restCont", cJSON_CreateString(event_cont));

	jsonValue = cJSON_PrintUnformatted(json);
	coap_printf("%s", jsonValue);

	ret = parse_response(url, jsonValue);

	free(jsonValue);
	cJSON_Delete(json);
	return ret;
}


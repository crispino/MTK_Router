/******************************************************************************
 * file:andlink_tcp_handler.cpp  
 * version: 1.0.0
 * description:- tcp message handler
 * modification history
 * --------------------
 * author: xiaoning 2018-06-16 15:06:48
 * --------------------
*******************************************************************************/


#include "andlink_tcp_handler.h"
#include "cJSON.h"
#include "debug.h"
#include "andlink.h"
#include "andlink_business.h"
#include "zlink_aes_ecb.h"
#include "router_cfg.h"

char g_DevKey[DevRND_LENGTH + 1];
extern char g_DevRND[DevRND_LENGTH + 1];

ANDLINK_HANDLE g_tcp_handle[] =
{ 
	{MSG_HeartBeat, 	&tcp_heartbeat,  "heartbeat with tcp server"},
	{MSG_Disconnect, 	&tcp_disconnect, "disconect tcp connection"},

	{MSG_WiFiParameterSync, 	&tcp_control_wifi_para_sync, "WiFi参数同步请求"},
	{MSG_WiFiSwitch, 			&tcp_control_wifi_swtich, "WiFi开关控制请求"},
	{MSG_RadioConfig, 			&tcp_control_wifi_radio,  "WiFi功率配置请求"},
	{MSG_WPS, 					&tcp_control_wifi_wps,    "WiFi WPS控制请求"},
	{MSG_MacFilter, 			&tcp_control_mac_filter,  "设置黑白名单请求"},
	{MSG_Reboot, 				&tcp_control_reboot,      "设备重启请求"},
	{MSG_LEDControl, 			&tcp_control_led_control, "指示灯开关请求"},
	{MSG_RoamingConfig, 		&tcp_control_sta_roaming, "STA漫游请求"},
	
	{MSG_ApConfigInfo, 	&tcp_query_cfg_info, "query config info"},
	{MSG_UplinkStatus, 	&tcp_query_ap_uplink, "query ap uplink info"},	
	{MSG_STAInfo, 	   	&tcp_query_sta_info, "query stat info"},	
	{MSG_WiFiStats, 	&tcp_query_dev_statis, "query dev statis info"},


	{MSG_Unbind, 	&tcp_regist_dev_unbind, "解除绑定"},	
	{MSG_File, 	&tcp_regist_dev_file, "升级请求"},
};

ANDLINK_HANDLE g_udp_handle[] =
{ 
	{MSG_HeartBeat, 		&udp_heartbeat, "heartbeat with udp server"},
	{MSG_RequestPlugin, 	&udp_request_plugin, "udp request plugin"},
};


void vUdpMessageHandler(int nSockId, struct sockaddr *sendaddr, const char* data, int data_len)
{
	int i, supported = 0;
	cJSON *json = NULL;

	coap_printf("data: %s, LEN: %d", data, data_len);
	
	json = cJSON_Parse(data);
	if (!json) {
		coap_printf("json parse error!");
		return;
	}
	
	if(!cJSON_HasObjectItem(json, "RPCMethod")) {
		cJSON_Delete(json);
		coap_printf("lack of argument[RPCMethod]!");
		return;
	}
	
	char *rpcMethod = cJSON_GetObjectItem(json, "RPCMethod")->valuestring;
	for (i = 0; i < ANDLINK_ARRAY_SIZE(g_udp_handle); i++) {
		if (!strncasecmp(rpcMethod, g_udp_handle[i].rpcMethod, strlen(g_udp_handle[i].rpcMethod))) {
			supported = 1;
			g_udp_handle[i].func(nSockId, sendaddr, json);
			break;
		}
	}
	
	if (0 == supported) {//not supported
		coap_printf("Unsupported rpcMethod: %s", rpcMethod);
	}
	
	cJSON_Delete(json);
	return;
}


void vTcpMessageHandler(int nSockId, const char* data, int data_len)
{
	int i, supported = 0;
	cJSON *json = NULL;

	coap_printf("data: %s, LEN: %d", data, data_len);
	
	json = cJSON_Parse(data);
	if (!json) {
		vTcpCommonResponse(nSockId, INVALID_ID, CLOUD_FAIL, "Json parse error");
		return;
	}
	
	if(!cJSON_HasObjectItem(json, "RPCMethod")) {
		cJSON_Delete(json);
		vTcpCommonResponse(nSockId, INVALID_ID, CLOUD_FAIL, "lack of argument");
		return;
	}
	
	char *rpcMethod = cJSON_GetObjectItem(json, "RPCMethod")->valuestring;
	for (i = 0; i < ANDLINK_ARRAY_SIZE(g_tcp_handle); i++) {
		if (!strncasecmp(rpcMethod, g_tcp_handle[i].rpcMethod, strlen(g_tcp_handle[i].rpcMethod))) {
			supported = 1;
			g_tcp_handle[i].func(nSockId, json);
			break;
		}
	}
	
	if (0 == supported) {//not supported
		coap_printf("Unsupported rpcMethod: %s", rpcMethod);
		vTcpCommonResponse(nSockId, INVALID_ID, CLOUD_FAIL, "rpcMethod not supported");
	}
	
	cJSON_Delete(json);
	return;
}

void vTcpCommonResponse(int nSockId, const char* id, int respCode, const char* respContent)
{
	cJSON *root = NULL;
	root = cJSON_CreateObject();
	if (!root) return;

	cJSON_AddItemToObject(root, "ID",  cJSON_CreateString(id));
	cJSON_AddItemToObject(root, "respCode", cJSON_CreateNumber(respCode));
	cJSON_AddItemToObject(root, "restCont", cJSON_CreateString(respContent));

	char *jsonValue = cJSON_PrintUnformatted(root);
	if (!jsonValue) return;
	
	int nLen = strlen(jsonValue);
	coap_printf("data: %s, len: %d", jsonValue, nLen);
		
	nTcpTransResponse(nSockId, jsonValue, nLen);
	cJSON_Delete(root);
	free(jsonValue);
	
	return;
}

void udp_request_plugin(int nSockId, struct sockaddr *sendaddr, cJSON* json)
{
	int nReboot =0, respCode = -1;
	
	if (!cJSON_HasObjectItem(json, "Key") || !cJSON_HasObjectItem(json, "ServerAddr") ||
		!cJSON_HasObjectItem(json, "ServerPort")) {
		coap_printf("lack of arguments");
		return;
	}

	coap_printf("current devKey:%s, devRND:%s", g_DevKey, g_DevRND);

	do {

		char *key = cJSON_GetObjectItem(json, "Key")->valuestring;		
		//1. 报文重传情形
		if (!strcmp(g_DevKey, key)) {
			respCode = 0;
			break;
		}

		strncpy(g_DevKey, key, DevRND_LENGTH);

		//2. 连接已经存在的情形
		if (!nTcpExit()) {
			vGenerateDevSN(g_DevRND);
			respCode = 0;
			break;
		}
		
		//3. 完全匹配情形
		char sequence[64] = {0};
		char localKey[DevRND_LENGTH + 1] = {0};
		char deviceSn[18] = "13111805000001"; //default
		router_getDevSn(deviceSn, sizeof(deviceSn));
		sprintf(sequence, "%s%s", g_DevRND, deviceSn);
		md5Data(sequence, localKey);
		if (!strcmp(localKey, key)) {
			vGenerateDevSN(g_DevRND);
			respCode = 0;
			
			char *serverAddr = cJSON_GetObjectItem(json, "ServerAddr")->valuestring;
			char *sPort = cJSON_GetObjectItem(json, "ServerPort")->valuestring;
			char sHost[SERVER_ADDR_LENGTH + 1] = {0};
			snprintf(sHost, SERVER_ADDR_LENGTH, "%s:%s", serverAddr, sPort);
			
			nCloudServerChanged(PROTO_TCP, sHost);
			break;			
		}

		//Reboot Udp Client
		nReboot = 1;
		respCode = -1;

	} while(0);

	//Response
	cJSON *root = cJSON_CreateObject();
	if (!root) return;

	cJSON_AddItemToObject(root, "respCode",  cJSON_CreateNumber(respCode));
	cJSON_AddItemToObject(root, "DevRND", cJSON_CreateString(g_DevRND));
	char *jsonValue = cJSON_PrintUnformatted(root);
	if (!jsonValue) return;
	
	int nLen = strlen(jsonValue);
	coap_printf("data: %s, len: %d", jsonValue, nLen);
	nUdpWrite(nSockId, sendaddr, jsonValue, nLen);
	
	cJSON_Delete(root);
	free(jsonValue);

	//Reboot Udp
	if (nReboot) rebootUdpClient();
	
	return;
}

void udp_heartbeat(int nSockId, struct sockaddr *sendaddr, cJSON* json)
{
	int nPort = 0;
	char sHost[SERVER_ADDR_LENGTH +1] = {0};
	
	if (!cJSON_HasObjectItem(json, "respCode")) return;
	int respCode = cJSON_GetObjectItem(json, "respCode")->valueint;
	
	if (respCode == CLOUD_SUCCESS) {
		if (!cJSON_HasObjectItem(json, "heartBeatTime")) return;

		//心跳发生变更处理
		int nKeepAlive = cJSON_GetObjectItem(json, "heartBeatTime")->valueint;
		nHeartBeatChanged(PROTO_UDP, nKeepAlive);
		
	} else if (respCode == CLOUD_NEED_REGISTER) {
		//重启服务器处理
		rebootUdpClient();

	} else if (respCode == CLOUD_SERVER_CHANGED) {
		if (!cJSON_HasObjectItem(json, "ServerIP")) return;

		//云服务器发生变更处理
		char *serverIP = cJSON_GetObjectItem(json, "ServerIP")->valuestring;		
		nCloudServerChanged(PROTO_UDP, serverIP);
	}
	
	return;	
}


void tcp_disconnect(int nSockId, cJSON* json)
{
	if(!cJSON_HasObjectItem(json, "ID")) {
		vTcpCommonResponse(nSockId, INVALID_ID, CLOUD_FAIL, "lack of argument ID");
		return;
	}

	char *requestId = cJSON_GetObjectItem(json, "ID")->valuestring;
	vTcpCommonResponse(nSockId, requestId, CLOUD_SUCCESS, "success");

	//Reboot Tcp Client
	rebootTcpClient();
	
	return;
}

void tcp_heartbeat(int nSockId, cJSON* json)
{
	int nPort = 0;
	char sHost[SERVER_ADDR_LENGTH +1] = {0};
	
	if (!cJSON_HasObjectItem(json, "respCode")) return;
	int respCode = cJSON_GetObjectItem(json, "respCode")->valueint;
	
	if (respCode == CLOUD_SUCCESS) {
		if (!cJSON_HasObjectItem(json, "heartBeatTime")) return;

		//心跳发生变更处理
		int nKeepAlive = cJSON_GetObjectItem(json, "heartBeatTime")->valueint;
		nHeartBeatChanged(PROTO_TCP, nKeepAlive);
		
	} else if (respCode == CLOUD_NEED_REGISTER) {
		//重启服务器处理
		rebootTcpClient();

	} else if (respCode == CLOUD_SERVER_CHANGED) {
		if (!cJSON_HasObjectItem(json, "ServerIP")) return;

		//云服务器发生变更处理
		char *serverIP = cJSON_GetObjectItem(json, "ServerIP")->valuestring;		
		nCloudServerChanged(PROTO_TCP, serverIP);
	}
	
	return;
}

void tcp_control_wifi_swtich(int nSockId, cJSON* json)
{
	if(!cJSON_HasObjectItem(json, "ID")) {
		vTcpCommonResponse(nSockId, INVALID_ID, CLOUD_FAIL, "lack of argument ID");
		return;
	}

	char *requestId = cJSON_GetObjectItem(json, "ID")->valuestring;
	if (!cJSON_HasObjectItem(json, "data")) {
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "lack of argument data");
		return;
	}

	cJSON *Jdata = NULL;
	Jdata = cJSON_GetObjectItem(json, "data");
	if (!cJSON_HasObjectItem(Jdata, "Radio") || !cJSON_HasObjectItem(Jdata, "Enable"))
	{
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "lack of argument data");
		return;
	}

	char *Radio = NULL;
	int Enable;
	Radio = cJSON_GetObjectItem(Jdata, "Radio")->valuestring;
	Enable = cJSON_GetObjectItem(Jdata, "Enable")->valueint;
	coap_printf("Radio: %s, Enable: %d", Radio, Enable);

	if (strcmp("all", Radio) && strcmp("2.4G", Radio) && strcmp("5G", Radio)) {
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "radio value invaild");
		return;
	}

	vTcpCommonResponse(nSockId, requestId, CLOUD_SUCCESS, "success");
	
	if (!strcmp("all", Radio))
	{ 
		if (Enable == 0) { //Close Wifi			
			close_wifi("2.4G");
#if defined (CONFIG_PRODUCT_341AA)
			close_wifi("5G");
#endif 
			do_system("init_system restart"); 
		} else { //Open Wifi
			open_wifi("2.4G");
#if defined (CONFIG_PRODUCT_341AA)
			open_wifi("5G");
#endif 
			do_system("insmod -q rt2860v2_ap");
			do_system("init_system restart"); 
		}
	}
	else if (!strcmp("2.4G", Radio))
	{ 
		if (Enable == 0) { //Close Wifi 			   
			close_wifi(Radio); 
			do_system("init_system restart"); 
		} else { //Open Wifi
			open_wifi(Radio); 
			do_system("insmod -q rt2860v2_ap");
			do_system("init_system restart");  
		}
	}
#if defined (CONFIG_PRODUCT_341AA)
	else if (!strcmp("5G", Radio))
	{ 
		if (Enable == 0) { //Close Wifi 			   
			close_wifi(Radio); 
			do_system("init_system restart"); 	 
		} else { //Open Wifi
			open_wifi(Radio); 
			do_system("insmod -q rt2860v2_ap");
			do_system("init_system restart");  
		}
	}
#endif
 	
	return;
}
 
void tcp_control_wifi_para_sync(int nSockId, cJSON* json)
{
	if(!cJSON_HasObjectItem(json, "ID")) {
		vTcpCommonResponse(nSockId, INVALID_ID, CLOUD_FAIL, "lack of argument ID");
		return;
	}

	char *requestId = cJSON_GetObjectItem(json, "ID")->valuestring;
	if (!cJSON_HasObjectItem(json, "data")) {
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "lack of argument data");
		return;
	}
	 
	/* data */
	cJSON *Jdata = NULL;
	Jdata = cJSON_GetObjectItem(json, "data");
	if (!cJSON_HasObjectItem(Jdata, "SyncCode") || !cJSON_HasObjectItem(Jdata, "Configurations")) { 
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "lack of argument data");
		return;
	}

	vTcpCommonResponse(nSockId, requestId, CLOUD_SUCCESS, "success");

	char *SyncCode = NULL;
	SyncCode = cJSON_GetObjectItem(Jdata, "SyncCode")->valuestring;
	coap_printf("SyncCode: %s", SyncCode);
	nvram_bufset(RT2860_NVRAM, "SyncCode", SyncCode);

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
			vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "lack of argument data");
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
	
	return; 
}
 
void tcp_control_wifi_radio(int nSockId, cJSON* json)
{
	if(!cJSON_HasObjectItem(json, "ID")) {
		vTcpCommonResponse(nSockId, INVALID_ID, CLOUD_FAIL, "lack of argument ID");
		return;
	}

	char *requestId = cJSON_GetObjectItem(json, "ID")->valuestring;
	if (!cJSON_HasObjectItem(json, "data")) {
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "lack of argument data");
		return;
	}
	
	cJSON *Jdata = NULL;
	Jdata = cJSON_GetObjectItem(json, "data");

	if (!cJSON_HasObjectItem(Jdata, "Radio") || !cJSON_HasObjectItem(Jdata, "TransmitPower")) { 
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "lack of argument data");
		return;
    }
    
	char *Radio = NULL;
	char *TransmitPower = NULL;
	Radio = cJSON_GetObjectItem(Jdata, "Radio")->valuestring;
	TransmitPower = cJSON_GetObjectItem(Jdata, "TransmitPower")->valuestring;
	coap_printf("Radio: %s, TransmitPower: %s", Radio, TransmitPower);
	
	if (strcmp("all", Radio) && strcmp("2.4G", Radio) && strcmp("5G", Radio)) {
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "radio value invalid");
		return;
	}

	vTcpCommonResponse(nSockId, requestId, CLOUD_SUCCESS, "success");

	if (!strcmp("all", Radio))
	{
		wifi_power("2.4G", TransmitPower);
#if defined (CONFIG_PRODUCT_341AA)
		wifi_power("5G", TransmitPower);
#endif 
        do_system("init_system restart");      
	}
	else if (!strcmp("2.4G", Radio))
	{
		wifi_power(Radio, TransmitPower);
        do_system("init_system restart");
    }
#if defined (CONFIG_PRODUCT_341AA)
	else if (!strcmp("5G", Radio))
	{
		wifi_power(Radio, TransmitPower); 
        do_system("init_system restart");  
	}
#endif

	return;
}
 
void tcp_control_wifi_wps(int nSockId, cJSON* json)
{
	if(!cJSON_HasObjectItem(json, "ID")) {
		vTcpCommonResponse(nSockId, INVALID_ID, CLOUD_FAIL, "lack of argument ID");
		return;
	}

	char *requestId = cJSON_GetObjectItem(json, "ID")->valuestring;
	if (!cJSON_HasObjectItem(json, "data")) {
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "lack of argument data");
		return;
	}
	
	cJSON *Jdata = NULL;
	Jdata = cJSON_GetObjectItem(json, "data");
	if (!cJSON_HasObjectItem(Jdata, "Radio"))
	{
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "lack of argument data");
		return;
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
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "argument error");
		return;
	}
 
	vTcpCommonResponse(nSockId, requestId, CLOUD_SUCCESS, "success");
	return;
}
 
void tcp_control_mac_filter(int nSockId, cJSON* json)
{
	if(!cJSON_HasObjectItem(json, "ID")) {
		vTcpCommonResponse(nSockId, INVALID_ID, CLOUD_FAIL, "lack of argument ID");
		return;
	}

	char *requestId = cJSON_GetObjectItem(json, "ID")->valuestring;
	if (!cJSON_HasObjectItem(json, "data")) {
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "lack of argument data");
		return;
	}
	
	cJSON *Jdata = NULL;
	Jdata = cJSON_GetObjectItem(json, "data");
	
	if (!cJSON_HasObjectItem(Jdata, "MacFilterEnable") || !cJSON_HasObjectItem(Jdata, "MacFilterPolicy") || 
			!cJSON_HasObjectItem(Jdata, "MacFilterEntries"))
	{
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "lack of argument data");
		return;
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

	nvram_commit(RT2860_NVRAM);
	vTcpCommonResponse(nSockId, requestId, CLOUD_SUCCESS, "success");
	return;
}
 
void tcp_control_reboot(int nSockId, cJSON* json)
{
	if(!cJSON_HasObjectItem(json, "ID")) {
		vTcpCommonResponse(nSockId, INVALID_ID, CLOUD_FAIL, "lack of argument ID");
		return;
	}

	char *requestId = cJSON_GetObjectItem(json, "ID")->valuestring;
	if (!cJSON_HasObjectItem(json, "data")) {
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "lack of argument data");
		return;
	}
	
	cJSON *Jdata = NULL;
	Jdata = cJSON_GetObjectItem(json, "data");
	
	if (!cJSON_HasObjectItem(Jdata, "ControlType"))
	{
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "lack of argument data");
		return;
	}
	char *ControlType = NULL;
	ControlType = cJSON_GetObjectItem(Jdata, "ControlType")->valuestring;
	coap_printf("ControlType: %s", ControlType);

	if (strcmp("Reboot", ControlType) && strcmp("2.4GChanReSelect", ControlType) 
			&& strcmp("5GChanReSelect", ControlType)) {
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "ControlType value invalid");
		return;
	}
	
	vTcpCommonResponse(nSockId, requestId, CLOUD_SUCCESS, "success");
	
	if (!strcmp("Reboot", ControlType))
	{ 
		do_system("reboot"); 
	}
	/* ACSD */
	else if (!strcmp("2.4GChanReSelect", ControlType) || !strcmp("5GChanReSelect", ControlType))
	{ 
		do_system("init_system restart"); 
	}

	return;
}


void tcp_control_led_control(int nSockId, cJSON* json)
{
	if(!cJSON_HasObjectItem(json, "ID")) {
		vTcpCommonResponse(nSockId, INVALID_ID, CLOUD_FAIL, "lack of argument ID");
		return;
	}

	char *requestId = cJSON_GetObjectItem(json, "ID")->valuestring;
	if (!cJSON_HasObjectItem(json, "data")) {
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "lack of argument data");
		return;
	}
	
	if (!cJSON_HasObjectItem(json, "LEDOnOff"))
	{
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "lack of argument data");
		return;
	}
	
	int LEDOnOff = -1;
	LEDOnOff = cJSON_GetObjectItem(json, "LEDOnOff")->valueint;
	coap_printf("ControlType: %d", LEDOnOff);
	
	//do something. Turn on/off LED. 1: on, 0: off
	if (0 == LEDOnOff)
	{
		nvram_bufset(RT2860_NVRAM, "LEDOnOff", "0");
		nvram_commit(RT2860_NVRAM);
		do_system("factory led off");
	}
	else if(1 == LEDOnOff)
	{
		nvram_bufset(RT2860_NVRAM, "LEDOnOff", "1");
		nvram_commit(RT2860_NVRAM);
		do_system("factory led recover");
	}
	else
	{
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "argument error");
		return;
	}
	
	vTcpCommonResponse(nSockId, requestId, CLOUD_SUCCESS, "success");
	return;
}


void tcp_control_sta_roaming(int nSockId, cJSON* json)
{
	if(!cJSON_HasObjectItem(json, "ID")) {
		vTcpCommonResponse(nSockId, INVALID_ID, CLOUD_FAIL, "lack of argument ID");
		return;
	}

	char *requestId = cJSON_GetObjectItem(json, "ID")->valuestring;
	if (!cJSON_HasObjectItem(json, "data")) {
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "lack of argument data");
		return;
	}
	
	cJSON *Jdata = NULL;
	Jdata = cJSON_GetObjectItem(json, "data");
	if (!cJSON_HasObjectItem(Jdata, "RoamingSwitch"))
	{
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "lack of argument data");
		return;
	}
	
	int RoamingSwitch;
	RoamingSwitch = cJSON_GetObjectItem(Jdata, "RoamingSwitch")->valueint;
	coap_printf("RoamingSwitch: %d", RoamingSwitch);
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

	vTcpCommonResponse(nSockId, requestId, CLOUD_SUCCESS, "success");
	do_system("init_system restart"); 
	return;
} 


void tcp_query_cfg_info(int nSockId, cJSON* json)
{
	if(!cJSON_HasObjectItem(json, "ID")) {
		vTcpCommonResponse(nSockId, INVALID_ID, CLOUD_FAIL, "lack of argument ID");
		return;
	}

	char *requestId = cJSON_GetObjectItem(json, "ID")->valuestring;
	char *jsonValue = get_ApConfigInfo(CLOUD_GATEWAY, requestId);
	if (!jsonValue) {
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "system error");		
		return;
	}

	nTcpTransResponse(nSockId, jsonValue, strlen(jsonValue));
	free(jsonValue);
	return;
}

void tcp_query_ap_uplink(int nSockId, cJSON* json)
{
	if(!cJSON_HasObjectItem(json, "ID")) {
		vTcpCommonResponse(nSockId, INVALID_ID, CLOUD_FAIL, "lack of argument ID");
		return;
	}

	char *requestId = cJSON_GetObjectItem(json, "ID")->valuestring;
	char *jsonValue = get_UplinkStatus(CLOUD_GATEWAY, requestId);
	if (!jsonValue) {
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "system error");		
		return;
	}

	nTcpTransResponse(nSockId, jsonValue, strlen(jsonValue));
	free(jsonValue);
	return;
}

void tcp_query_sta_info(int nSockId, cJSON* json)
{
	if(!cJSON_HasObjectItem(json, "ID")) {
		vTcpCommonResponse(nSockId, INVALID_ID, CLOUD_FAIL, "lack of argument ID");
		return;
	}

	char *requestId = cJSON_GetObjectItem(json, "ID")->valuestring;
	char *jsonValue = get_STAInfo(CLOUD_GATEWAY, requestId);
	if (!jsonValue) {
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "system error");		
		return;
	}

	nTcpTransResponse(nSockId, jsonValue, strlen(jsonValue));
	free(jsonValue);
	return;
}

void tcp_query_dev_statis(int nSockId, cJSON* json)
{	
	if(!cJSON_HasObjectItem(json, "ID")) {
		vTcpCommonResponse(nSockId, INVALID_ID, CLOUD_FAIL, "lack of argument ID");
		return;
	}

	char *requestId = cJSON_GetObjectItem(json, "ID")->valuestring;
	char *jsonValue = get_WiFIStats(CLOUD_GATEWAY, requestId);
	if (!jsonValue) {
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "system error");		
		return;
	}

	nTcpTransResponse(nSockId, jsonValue, strlen(jsonValue));
	free(jsonValue);
	return;
}


void tcp_regist_dev_unbind(int nSockId, cJSON* json)
{
	if(!cJSON_HasObjectItem(json, "ID")) {
		vTcpCommonResponse(nSockId, INVALID_ID, CLOUD_FAIL, "lack of argument ID");
		return;
	}

	char *requestId = cJSON_GetObjectItem(json, "ID")->valuestring;

	/* do something */
	nvram_bufset(RT2860_NVRAM, "andlink_tcpServer", "");
	nvram_bufset(RT2860_NVRAM, "andlink_udpServer", "");
	nvram_bufset(RT2860_NVRAM, "andlink_tcpHeartbeat", "");
	nvram_bufset(RT2860_NVRAM, "andlink_udpHeartbeat", "");
	nvram_bufset(RT2860_NVRAM, "andlink_andlinkToken", "");
	nvram_bufset(RT2860_NVRAM, "andlink_timestamp", "");
	nvram_bufset(RT2860_NVRAM, "andlink_heartbeatTime", "");
	nvram_bufset(RT2860_NVRAM, "Q_LINK_OK", "0");
	nvram_commit(RT2860_NVRAM);
	vTcpCommonResponse(nSockId, requestId, CLOUD_SUCCESS, "success");

	rebootTcpClient();
	rebootUdpClient();
	return;
}

void reportUpgradeProgress(int sockId, int code)
{
	int nReport = 0;
	cJSON *json, *data = NULL;
	char *jsonValue, desc = "";
	
	switch(code)
	{
		case UNKOWN_ERROR :
			nReport = 1;
			desc = "Unknow Error";
			break;
			
		case TRANSFER_INTERRUPT :
			nReport = 1;
			desc = "File Transfer interrupt";
			break;

		case TRANSFER_SUCCESS :
			nReport = 1;
			desc = "File Transfer Success";
			break;

		case UPGRADE_SUCCESS :
			nReport = 1;
			desc = "File Upgrade Complete";
			break;
			
		default :
			 break;
	}

	if (!nReport) return;
	

	json = cJSON_CreateObject();
	if (!json) return;
	
	cJSON_AddItemToObject(json, "RPCMethod", cJSON_CreateString("data"));
	cJSON_AddItemToObject(json, "MAC", cJSON_CreateString(g_dev_mac));
	cJSON_AddItemToObject(json, "eventType", cJSON_CreateString("EVENT_DEVICE_FILE"));
	cJSON_AddItemToObject(json, "timestamp", cJSON_CreateNumber(router_getCompileTimeStamp()));

	cJSON_AddItemToObject(json, "data", data = cJSON_CreateObject());
	cJSON_AddItemToObject(data, "respCode", cJSON_CreateNumber(code));
	cJSON_AddItemToObject(data, "restCont", cJSON_CreateString(desc));

	jsonValue = cJSON_PrintUnformatted(json);

	//Report Data
	nTcpPushData(sockId, jsonValue, strlen(jsonValue));
	
	free(jsonValue);
	cJSON_Delete(json);
	return;
}

void tcp_regist_dev_file(int nSockId, cJSON* json)
{
	int nStatus;	
	if(!cJSON_HasObjectItem(json, "ID")) {
		vTcpCommonResponse(nSockId, INVALID_ID, CLOUD_FAIL, "lack of argument ID");
		return;
	}

	char *requestId = cJSON_GetObjectItem(json, "ID")->valuestring;	 

	if (!cJSON_HasObjectItem(json, "fileMode") || !cJSON_HasObjectItem(json, "fileType") 
		|| !cJSON_HasObjectItem(json, "fileUrl") || !cJSON_HasObjectItem(json, "downMode"))
	{
		vTcpCommonResponse(nSockId, requestId, CLOUD_FAIL, "lack of argument data");
		return;
	}

	char *fileMode = NULL, *fileType = NULL, *fileUrl = NULL;
	fileMode = cJSON_GetObjectItem(json, "fileMode")->valuestring;
	fileType = cJSON_GetObjectItem(json, "fileType")->valuestring;
	fileUrl = cJSON_GetObjectItem(json, "fileUrl")->valuestring;
	int downMode = cJSON_GetObjectItem(json, "downMode")->valueint;

	coap_printf("fileMode: %s, fileType: %s, fileUrl: %s, downMode: %d", 
										fileMode, fileType, fileUrl, downMode);
	
	vTcpCommonResponse(nSockId, requestId, CLOUD_SUCCESS, "success");

	/* DOWN / UP */
	if (!strcmp(fileMode, "DOWN")) {
		if (!strcmp(fileType, "FIRM")) {
			/* Download file */
			int file_len;
			nStatus = andlink_download(fileUrl, &file_len);
			reportUpgradeProgress(nSockId, nStatus);

			if (nStatus == TRANSFER_SUCCESS) {
				nStatus = andlink_upgrade(file_len);
				reportUpgradeProgress(nSockId, nStatus);
			}
			
			usleep(500*1000);
			if (downMode == 1) do_system("reboot");
		}
	}
	
	return;
}





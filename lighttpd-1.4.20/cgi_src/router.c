#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>
#include <stdint.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/route.h>
#include "oid.h"
#include "router.h"
#include "aes.h"
#include "base64.h"



ONT_APP_LACAL_HANDLE ont_app_handle[] =
{
#if defined (CONFIG_PRODUCT_339AA) || defined (CONFIG_PRODUCT_339AA_JS) || defined (CONFIG_PRODUCT_331AA)
	{ MSG_APP_DEVICE_PASSWORD_REQ , &ont_get_device_password_rsp, "Get device password for AP", 0 }, //获取设备密码
#else
	{ MSG_WTP_GET_BROADBAND_SETTING_REQ , &ont_get_broadband_setting_rsp, "Get BroadBand Setting", 0 }, //获取上网设置信息
    { MSG_WTP_SET_BROADBAND_SETTING_REQ , &ont_set_broadband_setting_req, "Set BroadBand Setting", 0 }, //设置上网信息
	{ MSG_APP_DEVICE_INFO_REQ , &ont_get_device_info_rsp, "Get device info", 0 }, //获取路由器设备信息
	{ MSG_APP_DEVICE_PASSWORD_REQ , &ont_get_device_password_rsp, "Get device password for route", 0 }, //获取设备密码
#endif
};



void web_response(const char *data, int length)
{
	char formatTime[64]= {0};
	time_t mtime = time(NULL);
	strftime(formatTime, sizeof(formatTime), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&mtime));

	printf ("HTTP/1.1 %d %s\r\n", 200, "OK");
	printf ("Content-Type: application/json\r\n");
	printf ("Content-Length: %d\r\n", length);
	printf ("Date: %s\r\n", formatTime);
	printf ("Server: lighttpd/1.4.20");
	printf ("\r\n\r\n"); /* end of header */

	printf("%s", data);

	fflush(stdout);
}

void addRsphead_app(cJSON *json, char *version, char *msgType, char *msgSeq, 
		char *errcode, char *desc)
{
	cJSON_AddItemToObject(json, "version", cJSON_CreateString(version));
	cJSON_AddItemToObject(json, "msgType", cJSON_CreateString(msgType));
	if (msgSeq)
		cJSON_AddItemToObject(json, "msgSeq", cJSON_CreateString(msgSeq));
	cJSON_AddItemToObject(json, "erroCode", cJSON_CreateString(errcode));
	cJSON_AddItemToObject(json, "description", cJSON_CreateString(desc));

	return;
}

void router_getGateway(char *sGateway, size_t len)
{
	char   buff[256];
	int    nl = 0 ;
	struct in_addr dest;
	struct in_addr gw;
	int    flgs, ref, use, metric;
	unsigned long int d,g,m;
 	
	FILE *fp = fopen("/proc/net/route", "r");
	while (fgets(buff, sizeof(buff), fp) != NULL) {
		if (nl) {
			int ifl = 0;
			while (buff[ifl]!=' ' && buff[ifl]!='\t' && buff[ifl]!='\0')
				ifl++;
			buff[ifl]=0;    /* interface */
			if (sscanf(buff+ifl+1, "%lx%lx%X%d%d%d%lx",
						&d, &g, &flgs, &ref, &use, &metric, &m)!=7) {
				fclose(fp);
				//log_error_write(srv, __FILE__, __LINE__, "s","format error");
				return;
			}

			if (flgs&RTF_UP) {
				dest.s_addr = d;
				gw.s_addr   = g;
				strncpy(sGateway, (gw.s_addr==0 ? "" : inet_ntoa(gw)), len);

				if (dest.s_addr == 0) {
					break;
				}
			}
		}
		nl++;
	}
	fclose(fp);
}

void router_getDns(int index, char *sDns, size_t len)
{
	FILE *fp;
	char buf[80] = {0}, ns_str[11], dns[16] = {0};
	int i = 0;

	fp = fopen("/etc/resolv.conf", "r");
	if (NULL == fp) 
		return;
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (strncmp(buf, "nameserver", 10) != 0)
			continue;
		sscanf(buf, "%s%s", ns_str, dns);
		i++;
		if (i == index)
			break;
	}
	fclose(fp);

	strncpy(sDns, dns, len);
}

int ont_set_boardband(recRouterBoardband *prBoardband)
{
	int nType = atoi(prBoardband->type);
	DBG_MSG("prBoardband type : %d.", nType);
    
	if (nType == 1)
	{
		nvram_bufset(RT2860_NVRAM, "wanConnectionMode", "PPPOE");
	    nvram_bufset(RT2860_NVRAM, "wan_pppoe_user", prBoardband->PPPoEName);
		nvram_bufset(RT2860_NVRAM, "wan_pppoe_pass", prBoardband->PPPoECode);
		//hemu pppoe use KeepAlive operation mode as default
		nvram_bufset(RT2860_NVRAM, "wan_pppoe_opmode", "KeepAlive");
		nvram_bufset(RT2860_NVRAM, "wan_pppoe_optime", "");
		
	} else if (nType == 4) {        
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


int ont_unsupported_msgtype_rsp(char *msgType, char *msgSeq)
{
	int err;
	cJSON *json = NULL;
	char *jsonValue = NULL;
	
	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	addRsphead_app(json, VERSION_N, msgType, msgSeq, ONT_UNSUPPORT_MSGTYPE, "Unsupported msgtype");
	jsonValue = cJSON_PrintUnformatted(json);
    web_response(jsonValue, strlen(jsonValue));

	free(jsonValue);
	cJSON_Delete(json);

	return ONT_CMD_OK;
}

#if defined (CONFIG_PRODUCT_339AA) || defined (CONFIG_PRODUCT_339AA_JS) || defined (CONFIG_PRODUCT_331AA)
int ont_get_device_password_rsp(char *msgSeq, cJSON *in_json)
{
    cJSON *json = NULL;
    const char *ont_password;
    char *jsonValue = NULL;

    ont_password = nvram_bufget(RT2860_NVRAM, CFG_ONT_PASSWD);

    json = cJSON_CreateObject();
    if (!json) return -1;

	cJSON_AddItemToObject(json, "msgType", cJSON_CreateString(MSG_WTP_DEVICE_PASSWORD_RSP));
    cJSON_AddItemToObject(json, "password", cJSON_CreateString(ont_password));

    jsonValue = cJSON_PrintUnformatted(json);

	DBG_MSG("jsonValue: %s", jsonValue);
    web_response(jsonValue, strlen(jsonValue));

    free(jsonValue);
    cJSON_Delete(json);
	
	return ONT_CMD_OK;
}
#else
int ont_get_device_password_rsp(char *msgSeq, cJSON *in_json)
{
    cJSON *json = NULL;
    const char *ont_password;
    char *jsonValue = NULL;

    ont_password = nvram_bufget(RT2860_NVRAM, CFG_ONT_PASSWD);

    json = cJSON_CreateObject();
    if (!json) return -1;

    addRsphead_app(json, VERSION_N, MSG_WTP_DEVICE_PASSWORD_RSP, msgSeq, ONT_SUCCESS, "device password rsp");
    cJSON_AddItemToObject(json, "password", cJSON_CreateString(ont_password));

    jsonValue = cJSON_PrintUnformatted(json);

	DBG_MSG("jsonValue: %s", jsonValue);
    web_response(jsonValue, strlen(jsonValue));

    ont_platform_free(jsonValue);
    cJSON_Delete(json);

	return ONT_CMD_OK;
}

void router_getDevSn(char *sn, int len)
{
	char buf[32] = {0};
	char *ptr = NULL;

	FILE *pp = popen("eth_mac g sn", "r");
	if (pp)
	{
		while(fgets(buf, sizeof(buf), pp))
		{
			if (strlen(buf))
			{
				ptr = strstr(buf, "SN:");
				if (ptr && (strlen(ptr) > 4))
				{
					ptr += strlen("SN:");
					strncpy(sn, ptr, len-1);
					sn[strlen(sn) -1] = '\0'; // cut '\n'
				}
			}
		}

		pclose(pp);
	}

	return;
}

/* 获取路由器设备信息应答 **/
int ont_get_device_info_rsp(char *msgSeq, cJSON *in_json)
{
	cJSON *json = NULL;
	char *jsonValue = NULL;
	
	json = cJSON_CreateObject();
	if (!json){
		DBG_MSG("create json object error!");
		return ONT_JSON_ERR;
	}
    
	addRsphead_app(json, VERSION_N, MSG_APP_DEVICE_INFO_RSP, 
						msgSeq, ONT_SUCCESS, "router get device info resp");

    const char *host_name = NULL;
    host_name = nvram_bufget(RT2860_NVRAM, "SSID1");	
	if (host_name == NULL) {
		cJSON_Delete(json);
		DBG_MSG("SSID1 not exist!");
		return ONT_COMMON_ERR;
	}

    cJSON_AddItemToObject(json, "name", cJSON_CreateString(host_name));

    //当前云平台注册用的是br0 MAC地址， 是否应该用WAN端 eth2.2的MAC地址
    const char *mac_address = NULL;
    mac_address = get_macaddr("eth2");

    cJSON_AddItemToObject(json, "MAC", cJSON_CreateString(mac_address));

	char sn[18] = "131117100000001"; //default
	router_getDevSn(sn, sizeof(sn));

    cJSON_AddItemToObject(json, "deviceID", cJSON_CreateString(sn));
    
    jsonValue = cJSON_PrintUnformatted(json);

	DBG_MSG("rsp: %s", jsonValue);
    web_response(jsonValue, strlen(jsonValue));
    
    free(jsonValue);
    cJSON_Delete(json);
    return ONT_CMD_OK;
}

/* 设置上网信息应答 **/

int ont_set_broadband_setting_rsp(char *msgSeq)
{
	int err;
	cJSON *json = NULL;
	char *jsonValue = NULL;

	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	addRsphead_app(json, VERSION_N, MSG_WTP_SET_BROADBAND_SETTING_RSP, msgSeq, ONT_SUCCESS, "set boardband rsp");

	jsonValue = cJSON_PrintUnformatted(json);
	DBG_MSG("rsp: %s", jsonValue);
    web_response(jsonValue, strlen(jsonValue));

	free(jsonValue);
	cJSON_Delete(json);

	return ONT_CMD_OK;

}

/* 设置上网信息请求 **/

int ont_set_broadband_setting_req(char *msgSeq, cJSON *in_json)
{
    char base64_target[512] = {0};
	char *pPassword = NULL;
	recRouterBoardband rBoardband;
	memset(&rBoardband, 0, sizeof(recRouterBoardband));

	if (!in_json) return ONT_JSON_ERR;
	
	ONT_CHECK_JSON(in_json) //failed
	{
		DBG_MSG("cJSON format error");
		return ONT_COMMON_ERR;
	}
	else
	{
		if (!cJSON_HasObjectItem(in_json, "type")) {
			DBG_MSG("cJSON item type doesn't exist");
			return ONT_JSON_ERR;
		}
		
	    strncpy(rBoardband.type, cJSON_GetObjectItem(in_json, "type")->valuestring, sizeof(rBoardband.type));
        
		int nType = atoi(rBoardband.type);
		if (nType != 1 && nType != 2 && nType != 4) {
			DBG_MSG("cJSON item type[%d] value invalid ", nType);
			return ONT_JSON_ERR;
		}
		
		if (nType == 1) {
			ONT_CHECK_ITEM(in_json, "PPPoEName")
				 strncpy(rBoardband.PPPoEName, cJSON_GetObjectItem(in_json, "PPPoEName")->valuestring, sizeof(rBoardband.PPPoEName));

			ONT_CHECK_ITEM(in_json, "PPPoECode")
			{
				//Base64Decode and AES Decrypt								
				pPassword = cJSON_GetObjectItem(in_json, "PPPoECode")->valuestring;
				base64_decode(pPassword, base64_target, sizeof(base64_target));
				DBG_MSG("ont decode password: ", base64_target);
				unsigned char key[17] = {0};
				snprintf(key, sizeof(key), ONT_KEY);
				ont_aes128_ecb_pkcs5_decrypt(key, base64_target, rBoardband.PPPoECode);				
			}
			
		} else if (nType == 4) {
			ONT_CHECK_ITEM(in_json, "IP")
				strncpy(rBoardband.ipAddr, cJSON_GetObjectItem(in_json, "IP")->valuestring, sizeof(rBoardband.ipAddr));

			ONT_CHECK_ITEM(in_json, "gatewayIP")
				strncpy(rBoardband.gateWayIP, cJSON_GetObjectItem(in_json, "gatewayIP")->valuestring, sizeof(rBoardband.gateWayIP));

			ONT_CHECK_ITEM(in_json, "submask")
				strncpy(rBoardband.subMask, cJSON_GetObjectItem(in_json, "submask")->valuestring, sizeof(rBoardband.subMask));

			ONT_CHECK_ITEM(in_json, "DNS1")
				strncpy(rBoardband.dns1, cJSON_GetObjectItem(in_json, "DNS1")->valuestring, sizeof(rBoardband.dns1));

			ONT_CHECK_ITEM(in_json, "DNS2")
				strncpy(rBoardband.dns2, cJSON_GetObjectItem(in_json, "DNS2")->valuestring, sizeof(rBoardband.dns2));
		}

		//set boardband information
		ont_set_boardband(&rBoardband);
		ont_set_broadband_setting_rsp(ONT_GET_MSGSEQ(in_json));
	}

	return ONT_CMD_OK;
}


/* 获取上网信息应答 **/
int ont_get_broadband_setting_rsp(char *msgSeq, cJSON *in_json)
{

	int err;
	cJSON *json = NULL;
	char *jsonValue = NULL;
	char sType[2] = {0};
	
	json = cJSON_CreateObject();
	if (!json){
		DBG_MSG("create json object error!");
		return ONT_JSON_ERR;
	}
	addRsphead_app(json, VERSION_N, MSG_WTP_GET_BROADBAND_SETTING_RSP, 
						msgSeq, ONT_SUCCESS, "router get broadband resp");


	const char *pWanConnectMode = NULL;		
	pWanConnectMode = nvram_bufget(RT2860_NVRAM, "wanConnectionMode");	
	if (pWanConnectMode == NULL) {
		cJSON_Delete(json);
		DBG_MSG("wan ConnectinMode format error");
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
		unsigned char key[17] = {0};
		snprintf(key, sizeof key, ONT_KEY);
		ont_aes128_ecb_pkcs5_encrypt(key, pPPPUserPassword, real_chiper);
		base64_encode(real_chiper, strlen(real_chiper), base64_target, sizeof(base64_target) -1);

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
		sType[0] = '2';
	}

	cJSON_AddItemToObject(json, "type", cJSON_CreateString(sType));
	
	jsonValue = cJSON_PrintUnformatted(json);
	DBG_MSG("rsp: %s", jsonValue);

    web_response(jsonValue, strlen(jsonValue));
	
	free(jsonValue);
	cJSON_Delete(json);
	
	return ONT_CMD_OK;
}
#endif


int main(int argc, char *argv[])
{	
	char *inStr;
	long inLen;
	inLen = strtol(getenv("CONTENT_LENGTH"), NULL, 10) + 1;
	if (inLen <= 1) {
		DBG_MSG("get no data!");
		return -1;
	}
	inStr = malloc(inLen);
	memset(inStr, 0, inLen);
	fgets(inStr, inLen, stdin);
	DBG_MSG("data: %s", inStr);
	
	cJSON *json  = cJSON_Parse(inStr);
	free(inStr);
	if (!json) {
        DBG_MSG("json parse error");
        return -1;
    }
	
	if(!cJSON_HasObjectItem(json, "msgType") || !cJSON_HasObjectItem(json, "msgSeq")) {
		DBG_MSG("json not support.");
		cJSON_Delete(json);
		return -1;
	}

	char *msm_type = NULL;
	char *msg_seq = NULL;
	msm_type = cJSON_GetObjectItem(json, "msgType")->valuestring;
	msg_seq = cJSON_GetObjectItem(json, "msgSeq")->valuestring;

	if (!strcasecmp("COMMON", msm_type)) {
		DBG_MSG("json msgType not support.");
		cJSON_Delete(json);
		return -1;
	}

	nvram_init(RT2860_NVRAM);	
	int i, supported = 0;
	for (i = 0; i < ONT_ARRAY_SIZE(ont_app_handle); i++)
	{
		if (!strncmp(msm_type, ont_app_handle[i].name, strlen(ont_app_handle[i].name))) {
			supported = 1;
			DBG_MSG("Matched_handle name: %s", ont_app_handle[i].name);
			ont_app_handle[i].func(msg_seq, json);
			break;
		}
		
	}
	
	if (0 == supported) { //not supported
		DBG_MSG("Unsupported msgType: %s", msm_type);
	}

	cJSON_Delete(json);
	nvram_close(RT2860_NVRAM);
	return ONT_CMD_OK;
}


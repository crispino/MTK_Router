/******************************************************************************
 * file:andlink_business.cpp 
 * version: 1.0.0
 * description:- Business Layer
 * modification history
 * --------------------
 * author: xiaoning 2018-06-13 15:06:48
 * --------------------
*******************************************************************************/

#include "andlink_business.h"
#include "andlink.h"
#include "router_cfg.h"
#include "debug.h"

extern char *g_deviceId;
extern char *g_dev_mac;

#define TXBYTE          	0
#define TXPACKET        	1
#define RXBYTE          	2
#define RXPACKET        	3
#define TXERROR          	4
#define TXDROP				5
#define RXERROR          	6
#define RXDROP		       	7
#define PROC_IF_STATISTIC       "/proc/net/dev"

static char *get_field(char *a_line, char *delim, int count)
{
	int i=0;
	char *tok;
	tok = strtok(a_line, delim);
	while(tok){
		if(i == count)
			break;
		i++;
		tok = strtok(NULL, delim);
	}
	if(tok && isdigit(*tok))
		return tok;

	return NULL;
}

static long long getIfStatistic(char *interface, int type)
{
	int found_flag = 0;
	int skip_line = 2;
	char buf[1024], *field, *semiColon = NULL;
	FILE *fp = fopen(PROC_IF_STATISTIC, "r");
	if(!fp){
		coap_printf("no proc?");
		return -1;
	}

	while(fgets(buf, 1024, fp)){
		char *ifname;
		if(skip_line != 0){
			skip_line--;
			continue;
		}
		if(! (semiColon = strchr(buf, ':'))  )
			continue;
		*semiColon = '\0';
		ifname = buf;
		ifname = strip_space(ifname);

		if(!strcmp(ifname, interface)){
			found_flag = 1;
			break;
		}
	}
	fclose(fp);

	semiColon++;

	switch(type){
	case TXBYTE:
		if(  (field = get_field(semiColon, " ", 8))  ){
			return strtoll(field, NULL, 10);
		}
		break;
	case TXPACKET:
		if(  (field = get_field(semiColon, " ", 9))  ){
			return strtoll(field, NULL, 10);
		}
		break;
	case RXBYTE:
		if(  (field = get_field(semiColon, " ", 0))  ){
			return strtoll(field, NULL, 10);
		}
		break;
	case RXPACKET:
		if(  (field = get_field(semiColon, " ", 1))  ){
			return strtoll(field, NULL, 10);
		}
		break;
	case TXERROR:
		if(  (field = get_field(semiColon, " ", 10))  ){
			return strtoll(field, NULL, 10);
		}
		break;
	case TXDROP:
		if(  (field = get_field(semiColon, " ", 11))  ){
			return strtoll(field, NULL, 10);
		}
		break;
	case RXERROR:
		if(  (field = get_field(semiColon, " ", 2))  ){
			return strtoll(field, NULL, 10);
		}
		break;
	case RXDROP:
		if(  (field = get_field(semiColon, " ", 3))  ){
			return strtoll(field, NULL, 10);
		}
		break;
	}

	return -1;
}

char* get_ApConfigInfo(int nProtocol, const char *id)
{
	cJSON *root = NULL;
	cJSON *data = NULL;
	cJSON *obj1 = NULL;
	cJSON *obj2 = NULL;
	cJSON *array = NULL;
	char *jsonValue = NULL;
	root = cJSON_CreateObject();
	if (!root) NULL;

	/* do something about get device config info */
	if (nProtocol == CLOUD_GATEWAY && id) {
		cJSON_AddItemToObject(root, "ID", cJSON_CreateString(id));
	}
	cJSON_AddItemToObject(root, "deviceId", cJSON_CreateString(g_deviceId));
    cJSON_AddItemToObject(root, "data", data = cJSON_CreateObject());
	cJSON_AddItemToObject(data, "hardwareVersion", cJSON_CreateString(H_VERSION));	

	FILE *fp = fopen("/etc_ro/version", "r");
	char version[64] = {S_VERSION};
	if (fp != NULL) {fgets(version, sizeof(version), fp); version[strlen(version)-1] = '\0'; fclose(fp);}
	cJSON_AddItemToObject(data, "softwareVersion", cJSON_CreateString(version));
	cJSON_AddItemToObject(data, "WorkingMode", cJSON_CreateNumber(atoi(nvram_get(RT2860_NVRAM, "OperationMode"))));

    char *pPolicy = nvram_get(RT2860_NVRAM, "AccessPolicy0");
    char *pPolicyList = nvram_get(RT2860_NVRAM, "AccessControlList0");
    int nAccessEnable = 0, nPolicy = 0, nTemp = 0;
    nTemp = atoi(pPolicy);
    char macList[1024] = {0};
    if (nTemp == 1) {  //White List
        nAccessEnable = 1;
        nPolicy = 1;
    } else if (nTemp == 2) { //Black List
        nAccessEnable = 1;
        nPolicy = 2;
    } else {  //Disable
        nAccessEnable = 0;
        nPolicy = 1;
    }

    //format the Filter Entries
    if (nAccessEnable) {
        char *pTok= NULL;
        pTok = strtok(pPolicyList, ";");
        while(NULL != pTok)
        {
    		 if(strlen(macList)) {
    			sprintf(macList, "%s,%s/unkown", macList, pTok);
    		} else {
    			sprintf(macList, "%s/unkown", pTok);
    		}
            pTok = strtok(NULL, ";");	 
        }
    }

    cJSON_AddItemToObject(data, "UpTime", cJSON_CreateNumber(GetUptime()));
    cJSON_AddItemToObject(data, "MacFilterEnable", cJSON_CreateNumber(nAccessEnable));
    cJSON_AddItemToObject(data, "MacFilterPolicy", cJSON_CreateNumber(nPolicy));
    cJSON_AddItemToObject(data, "MacFilterEntries", cJSON_CreateString(macList));

	char *tmp = NULL;
	int LEDOnOff = -1;
	tmp = nvram_get(RT2860_NVRAM, "LEDOnOff");
	if (NULL == tmp || 0 == strlen(tmp))
		LEDOnOff = 1; //default
	else
		LEDOnOff = atoi(tmp);
	cJSON_AddItemToObject(data, "LEDOnOff", cJSON_CreateNumber(LEDOnOff));
	cJSON_AddItemToObject(data, "RoamingSwitch", cJSON_CreateNumber(atoi(nvram_get(RT2860_NVRAM, "RoamingSwitch"))));
	cJSON_AddItemToObject(data, "LowRSSI2.4G", cJSON_CreateNumber(atoi(nvram_get(RT2860_NVRAM, "LowRSSI24G"))));
#if defined (CONFIG_PRODUCT_341AA)
	cJSON_AddItemToObject(data, "LowRSSI5G", cJSON_CreateNumber(atoi(nvram_get(RTDEV_NVRAM, "LowRSSI5G"))));
#endif
	cJSON_AddItemToObject(data, "SyncCode", cJSON_CreateString(nvram_get(RT2860_NVRAM, "SyncCode")));
	cJSON_AddItemToObject(data, "Radios", array = cJSON_CreateArray());
	cJSON_AddItemToObject(array, "", obj1 = cJSON_CreateObject());

	int nvram_id;
	nvram_id = RT2860_NVRAM;
	/* 2.4G */
	cJSON_AddItemToObject(obj1, "Radio", cJSON_CreateString("2.4G"));
	//default index is 1
	cJSON_AddItemToObject(obj1, "Enable", cJSON_CreateNumber(REVISE_VAL(atoi(nvram_get(nvram_id, "RadioOff")))));
	cJSON_AddItemToObject(obj1, "TransmitPower", cJSON_CreateString(nvram_get(nvram_id, "TxPower")));
	//Auto channel.
	int channel;
	channel = atoi(nvram_get(nvram_id, "Channel"));
	if (0 == channel)
	{
		channel = getAutoChannel();
		coap_printf("Auto channel: %d", channel);
	}
	cJSON_AddItemToObject(obj1, "Channel", cJSON_CreateNumber(channel));

#if defined (CONFIG_PRODUCT_341AA)
	nvram_id = RTDEV_NVRAM;
	cJSON_AddItemToObject(array, "", obj1 = cJSON_CreateObject());
	/* 2.4G */
	cJSON_AddItemToObject(obj1, "Radio", cJSON_CreateString("5G"));
	//default index is 1
	cJSON_AddItemToObject(obj1, "Enable", cJSON_CreateNumber(REVISE_VAL(atoi(nvram_get(nvram_id, "RadioOff")))));
	cJSON_AddItemToObject(obj1, "TransmitPower", cJSON_CreateString(nvram_get(nvram_id, "TxPower")));
	//Auto channel.
	channel = atoi(nvram_get(nvram_id, "Channel"));
	if (0 == channel)
	{
		channel = getAutoChannel();
		coap_printf("Auto channel: %d", channel);
	}
	cJSON_AddItemToObject(obj1, "Channel", cJSON_CreateNumber(channel));
#endif

	cJSON_AddItemToObject(data, "Configurations", array = cJSON_CreateArray());

    int Index =0, nEnable =0;
    char *ssid = NULL, *pAuthMode = NULL, *pEnable = NULL;
    char sTemp[32] = {0};
    char sAuthMode[32] = {0};

	char hide_buf[4][2];
	char *ssid_hide_buf = NULL;
	nvram_id = RT2860_NVRAM;
	ssid_hide_buf = nvram_get(nvram_id, "HideSSID");
	if (4 != sscanf(ssid_hide_buf, "%s;%s;%s;%s", hide_buf[0], hide_buf[1], hide_buf[2], hide_buf[3]))
	{
		printf("HideSSID argc error\n");
	}

    for (Index = 1; Index < MAX_SSID_NUM; Index++) {   //Support 4 SSIDS     
        /* 2.4G Configurations */
        memset(sTemp, 0, sizeof(sTemp));
        sprintf(sTemp, "SSID%d", Index);
        ssid = nvram_get(nvram_id, sTemp);
        if (strlen(ssid) > 0) {
            cJSON_AddItemToObject(array, "", obj2 = cJSON_CreateObject());
            cJSON_AddItemToObject(obj2, "Radio", cJSON_CreateString("2.4G"));
            cJSON_AddItemToObject(obj2, "Index", cJSON_CreateNumber(Index));
            //1. Get Enable
            memset(sTemp, 0x0, sizeof(sTemp));
            sprintf(sTemp, "Enable_SSID%d", Index);
            pEnable = nvram_get(nvram_id, sTemp);
            if (strlen(pEnable)  == 0) {
                nEnable = 1; //Default Value
            } else  {
                nEnable = atoi(pEnable);
            }
            cJSON_AddItemToObject(obj2, "Enable", cJSON_CreateNumber(nEnable));
            
            //2. Get SSID
            cJSON_AddItemToObject(obj2, "SSID", cJSON_CreateString(ssid));
            
            //3. Get AuthMode
            pAuthMode = nvram_get(nvram_id, "AuthMode");            
            memset(sTemp, 0x0, sizeof(sTemp));
            memset(sAuthMode, 0x0, sizeof(sAuthMode));
            //Split the AuthMode like: WPAPSKWPA2PSK;WPAPSKWPA2PSK;OPEN;OPEN
            get_split_value(pAuthMode, ";", Index, sTemp);
            if (!strcmp(sTemp, "WPAPSK")) {
                strcpy(sAuthMode, "WPA-Personal");
            } else if (!strcmp(sTemp, "WPA2PSK")) {
                strcpy(sAuthMode, "WPA2-Personal");
            } else if (!strcmp(sTemp, "WPAPSKWPA2PSK")) {
                strcpy(sAuthMode, "WPA-WPA2-Personal");
            } else {
                strcpy(sAuthMode, "None");
            }
            cJSON_AddItemToObject(obj2, "SecurityMode", cJSON_CreateString(sAuthMode));
            
            //4. Get Pwd
            memset(sTemp, 0x0, sizeof(sTemp));
            sprintf(sTemp, "WPAPSK%d", Index);
            cJSON_AddItemToObject(obj2, "Pwd", cJSON_CreateString(nvram_get(nvram_id, sTemp)));
            cJSON_AddItemToObject(obj2, "MaxAssociateNum", cJSON_CreateNumber(atoi(nvram_get(nvram_id, "MaxStaNum"))));
            cJSON_AddItemToObject(obj2, "SSIDAdvertisementEnabled", cJSON_CreateNumber(REVISE_VAL(atoi(hide_buf[Index-1]))));
        }
    }

#if defined (CONFIG_PRODUCT_341AA)
	nvram_id = RTDEV_NVRAM;
	ssid_hide_buf = nvram_get(nvram_id, "HideSSID");
	if (4 != sscanf(ssid_hide_buf, "%s;%s;%s;%s", hide_buf[0], hide_buf[1], hide_buf[2], hide_buf[3]))
	{
		printf("HideSSID argc error\n");
	}

    for (Index = 1; Index < MAX_SSID_NUM; Index++) {   //Support 4 SSIDS     
        /* 2.4G Configurations */
        memset(sTemp, 0, sizeof(sTemp));
        sprintf(sTemp, "SSID%d", Index);
        ssid = nvram_get(nvram_id, sTemp);
        if (strlen(ssid) > 0) {
            cJSON_AddItemToObject(array, "", obj2 = cJSON_CreateObject());
            cJSON_AddItemToObject(obj2, "Radio", cJSON_CreateString("5G"));
            cJSON_AddItemToObject(obj2, "Index", cJSON_CreateNumber(Index));
            //1. Get Enable
            memset(sTemp, 0x0, sizeof(sTemp));
            sprintf(sTemp, "Enable_SSID%d", Index);
            pEnable = nvram_get(nvram_id, sTemp);
            if (strlen(pEnable)  == 0) {
                nEnable = 1; //Default Value
            } else  {
                nEnable = atoi(pEnable);
            }
            cJSON_AddItemToObject(obj2, "Enable", cJSON_CreateNumber(nEnable));
            
            //2. Get SSID
            cJSON_AddItemToObject(obj2, "SSID", cJSON_CreateString(ssid));
            
            //3. Get AuthMode
            pAuthMode = nvram_get(nvram_id, "AuthMode");            
            memset(sTemp, 0x0, sizeof(sTemp));
            memset(sAuthMode, 0x0, sizeof(sAuthMode));
            //Split the AuthMode like: WPAPSKWPA2PSK;WPAPSKWPA2PSK;OPEN;OPEN
            get_split_value(pAuthMode, ";", Index, sTemp);
            if (!strcmp(sTemp, "WPAPSK")) {
                strcpy(sAuthMode, "WPA-Personal");
            } else if (!strcmp(sTemp, "WPA2PSK")) {
                strcpy(sAuthMode, "WPA2-Personal");
            } else if (!strcmp(sTemp, "WPAPSKWPA2PSK")) {
                strcpy(sAuthMode, "WPA-WPA2-Personal");
            } else {
                strcpy(sAuthMode, "None");
            }
            cJSON_AddItemToObject(obj2, "SecurityMode", cJSON_CreateString(sAuthMode));
            
            //4. Get Pwd
            memset(sTemp, 0x0, sizeof(sTemp));
            sprintf(sTemp, "WPAPSK%d", Index);
            cJSON_AddItemToObject(obj2, "Pwd", cJSON_CreateString(nvram_get(nvram_id, sTemp)));
            cJSON_AddItemToObject(obj2, "MaxAssociateNum", cJSON_CreateNumber(atoi(nvram_get(nvram_id, "MaxStaNum"))));
            cJSON_AddItemToObject(obj2, "SSIDAdvertisementEnabled", cJSON_CreateNumber(REVISE_VAL(atoi(hide_buf[Index-1]))));
        }
    }
#endif

	jsonValue = cJSON_PrintUnformatted(root);
	free(root);
	coap_printf("data :%s, LEN: %d", jsonValue, strlen(jsonValue));
	return jsonValue;
}


char* get_UplinkStatus(int nProtocol, const char* id)
{
	 cJSON *root = NULL;
	 cJSON *data = NULL;
	 char *jsonValue = NULL;
	 root = cJSON_CreateObject();
	 if (!root) return NULL;
 
	 /* do something about uplink status */
	 if (nProtocol == CLOUD_GATEWAY && id) {
		cJSON_AddItemToObject(root, "ID", cJSON_CreateString(id));
	 }
	 cJSON_AddItemToObject(root, "deviceId", cJSON_CreateString(g_deviceId));
	 cJSON_AddItemToObject(root, "data", data = cJSON_CreateObject());
 
	 char *UplinkType = NULL;
	 UplinkType = nvram_get(RT2860_NVRAM, "wanConnectionMode");
	 coap_printf("UplinkType: %s", UplinkType);
 
	 /* Wireless¡¢Ethernet¡¢PLC¡¢Cable */
	 /* Reserved for PLC product */
#if defined (CONFIG_PRODUCT_XXXAA)
	 cJSON_AddItemToObject(data, "UplinkType", cJSON_CreateString("PLC"));
 
#else
	 if (3 == atoi(nvram_get(RT2860_NVRAM, "OperationMode")))
	 {
		 cJSON_AddItemToObject(data, "UplinkType", cJSON_CreateString("Wireless"));
		 char upLinkSignal[9] = {0};
		 char *upLinkSSID = nvram_get(RT2860_NVRAM, "ApCliSsid");
		 router_getUpLinkSignal(upLinkSSID, upLinkSignal);
		 cJSON_AddItemToObject(data, "Radio", cJSON_CreateString("2.4G"));
		 cJSON_AddItemToObject(data, "SSID", cJSON_CreateString(upLinkSSID));
		 cJSON_AddItemToObject(data, "RSSI", cJSON_CreateString(upLinkSignal));
		 char sWifiChannel[4] = {0};
		 router_getWifiChannel(sWifiChannel, sizeof(sWifiChannel)-1);
		 cJSON_AddItemToObject(data, "Channel", cJSON_CreateNumber(atoi(sWifiChannel))); 
		 cJSON_AddItemToObject(data, "Noise", cJSON_CreateString("-90"));
		 cJSON_AddItemToObject(data, "SNR", cJSON_CreateString("35"));
	 }
	 else //Ethernet
	 {
		 cJSON_AddItemToObject(data, "UplinkType", cJSON_CreateString("Ethernet"));
		 cJSON_AddItemToObject(data, "Radio", cJSON_CreateString(""));
		 cJSON_AddItemToObject(data, "SSID", cJSON_CreateString(""));
		 cJSON_AddItemToObject(data, "RSSI", cJSON_CreateString(""));
		 cJSON_AddItemToObject(data, "Channel", cJSON_CreateNumber(0)); 
		 cJSON_AddItemToObject(data, "Noise", cJSON_CreateString(""));
		 cJSON_AddItemToObject(data, "SNR", cJSON_CreateString(""));
	 }
#endif
	 cJSON_AddItemToObject(data, "MacAddress", cJSON_CreateString(g_dev_mac));
	 
	 char TxRate[16] = {0};
	 char RxRate[16] = {0};
	 router_getUpAndDownLinkRate(TxRate, RxRate);
	 cJSON_AddItemToObject(data, "TxRate", cJSON_CreateString(TxRate)); //Mbps
	 cJSON_AddItemToObject(data, "RxRate", cJSON_CreateString(RxRate)); //Mbps
	 char *ifName = router_getIfname();
	 cJSON_AddItemToObject(data, "IPAddress", cJSON_CreateString(router_getIpaddr(ifName)));
 
	 jsonValue = cJSON_PrintUnformatted(root);
	 cJSON_Delete(root);
	 coap_printf("%s, LEN: %d", jsonValue, strlen(jsonValue));
	 return jsonValue;
}

char* get_STAInfo(int nProtocol, const char* id)
{
	cJSON *root = NULL;
	cJSON *data = NULL;
	cJSON *array = NULL;
    cJSON *obj = NULL;
	char *jsonValue = NULL;
	root = cJSON_CreateObject();
	if (!root) return NULL;

	if (nProtocol == CLOUD_GATEWAY && id) {
	   cJSON_AddItemToObject(root, "ID", cJSON_CreateString(id));
	}
	cJSON_AddItemToObject(root, "deviceId", cJSON_CreateString(g_deviceId));
	cJSON_AddItemToObject(root, "data", data = cJSON_CreateObject());
	cJSON_AddItemToObject(data, "Devices", array = cJSON_CreateArray());
	/* do something about client devices */
    {
		struct RouterClientList clientHead;
		SLIST_INIT(&clientHead);
		
		char *operationMode = nvram_bufget(RT2860_NVRAM, "OperationMode");
		if (operationMode != NULL && operationMode[0] == '1') { //Router Mode
			router_getConnectDevices(&clientHead);
		} else {
			router_getAPConnectDevices(&clientHead);
		}

		//char TxRate[16] = {0};
		//char RxRate[16] = {0};
		//router_getUpAndDownLinkRate(TxRate, RxRate);
		
		recRouterClient *prClientNode;
		SLIST_FOREACH(prClientNode, &clientHead, next)
		{
            cJSON_AddItemToObject(array, "", obj = cJSON_CreateObject());
			char client_mac_tmp[16] = {"B0D59DAAAAAA"};
			char mac_e[6][3] = {0};
			if (6 == sscanf(prClientNode->macAddr, "%s:%s:%s:%s:%s:%s", 
					mac_e[0], mac_e[1], mac_e[2], mac_e[3], mac_e[4], mac_e[5]))
				snprintf(client_mac_tmp, "%s%s%s%s%s%s", mac_e[0], mac_e[1], mac_e[2], mac_e[3], mac_e[4], mac_e[5]);

            cJSON_AddItemToObject(obj, "MacAddress", cJSON_CreateString(client_mac_tmp));
            cJSON_AddItemToObject(obj, "VMacAddress", cJSON_CreateString(client_mac_tmp));
            cJSON_AddItemToObject(obj, "Radio", cJSON_CreateString("2.4G"));
            cJSON_AddItemToObject(obj, "SSID", cJSON_CreateString(nvram_bufget(RT2860_NVRAM, "SSID1")));
            cJSON_AddItemToObject(obj, "UpTime", cJSON_CreateString(prClientNode->onlineDuration));
            cJSON_AddItemToObject(obj, "RSSI", cJSON_CreateString(prClientNode->RSSI));
            cJSON_AddItemToObject(obj, "RxRate", cJSON_CreateString(prClientNode->clientDownlinkRate));
            cJSON_AddItemToObject(obj, "TxRate", cJSON_CreateString(prClientNode->clientUplinkRate));
            //cJSON_AddItemToObject(obj, "IPAddress", cJSON_CreateString(prClientNode->ipAddr));
			//cJSON_AddItemToObject(obj, "HostName", cJSON_CreateString(prClientNode->name));
			free(prClientNode);
		}
	}

	jsonValue = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	coap_printf("%s, LEN: %d", jsonValue, strlen(jsonValue));
	return jsonValue;
}


char* get_WiFIStats(int nProtocol, const char* id)
{
		cJSON *root = NULL;
		cJSON *data = NULL;
		cJSON *array = NULL;
		cJSON *obj = NULL;
		char *jsonValue = NULL;
		root = cJSON_CreateObject();
		if (!root) return NULL;
	
		/* do something about WiFi stats*/
		if (nProtocol == CLOUD_GATEWAY && id) {
		   cJSON_AddItemToObject(root, "ID", cJSON_CreateString(id));
		}
		cJSON_AddItemToObject(root, "deviceId", cJSON_CreateString(g_deviceId));
		cJSON_AddItemToObject(root, "data", data = cJSON_CreateObject());
		cJSON_AddItemToObject(data, "Stats", array = cJSON_CreateArray());
		/* 2.4G */
		int index;
		for (index = 1; index < MAX_SSID_NUM; index++)
		{
			char ssid_tmp[64] = {0};
			sprintf(ssid_tmp, "SSID%d", index);
			if (strlen(nvram_get(RT2860_NVRAM, ssid_tmp)) > 0)
			{
				cJSON_AddItemToObject(array, "", obj = cJSON_CreateObject());
				cJSON_AddItemToObject(obj, "Radio", cJSON_CreateString("2.4G"));
				cJSON_AddItemToObject(obj, "Index", cJSON_CreateNumber(index));
				cJSON_AddItemToObject(obj, "SSID", cJSON_CreateString(nvram_get(RT2860_NVRAM, ssid_tmp)));
				char ifname_tmp[64] = {0};
				sprintf(ifname_tmp, "ra%d", index-1);
				cJSON_AddItemToObject(obj, "TotalBytesSent", cJSON_CreateNumber(getIfStatistic(ifname_tmp, TXBYTE)));
				cJSON_AddItemToObject(obj, "TotalBytesReceived", cJSON_CreateNumber(getIfStatistic(ifname_tmp, RXBYTE)));
				cJSON_AddItemToObject(obj, "TotalPacketsSent", cJSON_CreateNumber(getIfStatistic(ifname_tmp, TXPACKET)));
				cJSON_AddItemToObject(obj, "TotalPacketsReceived", cJSON_CreateNumber(getIfStatistic(ifname_tmp, RXPACKET)));
				cJSON_AddItemToObject(obj, "ErrorsSent", cJSON_CreateNumber(getIfStatistic(ifname_tmp, TXERROR)));
				cJSON_AddItemToObject(obj, "ErrorsReceived", cJSON_CreateNumber(getIfStatistic(ifname_tmp, RXERROR)));
				cJSON_AddItemToObject(obj, "DiscardPacketsSent", cJSON_CreateNumber(getIfStatistic(ifname_tmp, TXDROP)));
				cJSON_AddItemToObject(obj, "DiscardPacketsReceived", cJSON_CreateNumber(getIfStatistic(ifname_tmp, RXDROP)));
			}
	
#if defined	(CONFIG_PRODUCT_341AA)
			memset(ssid_tmp, 0x0, sizeof(ssid_tmp));
			sprintf(ssid_tmp, "SSID%d", index);
			if (strlen(nvram_get(RTDEV_NVRAM, ssid_tmp)) > 0)
			{
				cJSON_AddItemToObject(array, "", obj = cJSON_CreateObject());
				cJSON_AddItemToObject(obj, "Radio", cJSON_CreateString("5G"));
				cJSON_AddItemToObject(obj, "Index", cJSON_CreateNumber(index));
				cJSON_AddItemToObject(obj, "SSID", cJSON_CreateString(nvram_get(RTDEV_NVRAM, ssid_tmp)));
				char ifname_tmp[64] = {0};
				sprintf(ifname_tmp, "rai%d", index-1);
				cJSON_AddItemToObject(obj, "TotalBytesSent", cJSON_CreateNumber(getIfStatistic(ifname_tmp, TXBYTE)));
				cJSON_AddItemToObject(obj, "TotalBytesReceived", cJSON_CreateNumber(getIfStatistic(ifname_tmp, RXBYTE)));
				cJSON_AddItemToObject(obj, "TotalPacketsSent", cJSON_CreateNumber(getIfStatistic(ifname_tmp, TXPACKET)));
				cJSON_AddItemToObject(obj, "TotalPacketsReceived", cJSON_CreateNumber(getIfStatistic(ifname_tmp, RXPACKET)));
				cJSON_AddItemToObject(obj, "ErrorsSent", cJSON_CreateNumber(getIfStatistic(ifname_tmp, TXERROR)));
				cJSON_AddItemToObject(obj, "ErrorsReceived", cJSON_CreateNumber(getIfStatistic(ifname_tmp, RXERROR)));
				cJSON_AddItemToObject(obj, "DiscardPacketsSent", cJSON_CreateNumber(getIfStatistic(ifname_tmp, TXDROP)));
				cJSON_AddItemToObject(obj, "DiscardPacketsReceived", cJSON_CreateNumber(getIfStatistic(ifname_tmp, RXDROP)));
			}
#endif
		}
	
		/* Statistic Ethernet */
		cJSON_AddItemToObject(array, "", obj = cJSON_CreateObject());
		cJSON_AddItemToObject(obj, "Radio", cJSON_CreateString("0"));
		cJSON_AddItemToObject(obj, "Index", cJSON_CreateNumber(1));
		cJSON_AddItemToObject(obj, "SSID", cJSON_CreateString(""));
		char ifname_tmp[64] = {0};
		memcpy(ifname_tmp, "eth2", strlen("eth2"));
		cJSON_AddItemToObject(obj, "TotalBytesSent", cJSON_CreateNumber(getIfStatistic(ifname_tmp, TXBYTE)));
		cJSON_AddItemToObject(obj, "TotalBytesReceived", cJSON_CreateNumber(getIfStatistic(ifname_tmp, RXBYTE)));
		cJSON_AddItemToObject(obj, "TotalPacketsSent", cJSON_CreateNumber(getIfStatistic(ifname_tmp, TXPACKET)));
		cJSON_AddItemToObject(obj, "TotalPacketsReceived", cJSON_CreateNumber(getIfStatistic(ifname_tmp, RXPACKET)));
		cJSON_AddItemToObject(obj, "ErrorsSent", cJSON_CreateNumber(getIfStatistic(ifname_tmp, TXERROR)));
		cJSON_AddItemToObject(obj, "ErrorsReceived", cJSON_CreateNumber(getIfStatistic(ifname_tmp, RXERROR)));
		cJSON_AddItemToObject(obj, "DiscardPacketsSent", cJSON_CreateNumber(getIfStatistic(ifname_tmp, TXDROP)));
		cJSON_AddItemToObject(obj, "DiscardPacketsReceived", cJSON_CreateNumber(getIfStatistic(ifname_tmp, RXDROP)));
	
		jsonValue = cJSON_PrintUnformatted(root);
		cJSON_Delete(root);
		coap_printf("%s, LEN: %d", jsonValue, strlen(jsonValue));
		return jsonValue;

}


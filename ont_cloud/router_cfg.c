#include "router_cfg.h"
#include "ont/platform.h"

int router_getCfgValue(const char *cmd, char *cfgValue, size_t len)
{
    FILE* fp; 
    if((fp = popen(cmd, "r")) == NULL) 
    {
        SL_DEBUG("nGetRouterCfg:popen failed.");
        return -1;
    }
	
	fgets(cfgValue, len, fp);
	
	pclose(fp);
	return 0;
}

void router_getDeviceName(char *devName, size_t len)
{
	SL_DEBUG("begin to get device name.");
	router_getCfgValue(ROUTER_DEVICE_NAME, devName, len);
	SL_DEBUG("get devName value: %s.", devName);
	return;
}

void router_getWanIpAddr(char *wanIp, size_t len)
{
	SL_DEBUG("begin to get wan ip addr.");
	router_getCfgValue(ROUTER_WAN_IP_ADDR, wanIp, len);
	SL_DEBUG("get wan ip addr value: %s.", wanIp);
	return;
}

inline int getAutoChannel(void)
{
	char buf[128];
	char *ptr = NULL;
	int ret = 1; //default

	FILE *pp = popen("iwconfig ra0", "r");
	if (pp)
	{
		while(fgets(buf, sizeof(buf), pp))
		{
			if (strlen(buf))
			{
				ptr = strstr(buf, "Channel");
				if (ptr)
				{
					ptr += strlen("Channel") + 1; //skip '='
					ret = atoi(ptr);
				}
			}
		}

		pclose(pp);
	}

	return ret;
}

void router_getWifiChannel(char *wifiChannel, size_t len)
{
	const char *channel_s;
	int channel =0;
	SL_DEBUG("begin to get wifi channel.");
	channel_s = nvram_bufget(RT2860_NVRAM, "Channel");
	channel = (channel_s == NULL) ? 1 : strtoul(channel_s, NULL, 10);

	//Auto channel.
	if (0 == channel)
	{
		channel = getAutoChannel();
	}

	//channel value check
	if (channel > 13 || channel < 0) {		
		SL_DEBUG("get wifi channel invalid : value : %d.", channel);
		channel = 1; //default value
	}
	
	snprintf(wifiChannel, len, "%d", channel);
	
	SL_DEBUG("get wifi channel value: %s.", wifiChannel);
	return;
}


void router_getOnlineDuration(char *onlineTime)
{
	SL_DEBUG("begin to get online duration.");
	char sUpTime[64] = {0};
	int pos = 0,  nOnlineTime = 0;
	router_getCfgValue("cat /proc/uptime",  sUpTime, sizeof(sUpTime) -1);
	while(pos < strlen(sUpTime)) {
		if (isspace(sUpTime[pos])) {
			sUpTime[pos] = '\0';
			break;
		}
		pos++;
	}
	
	nOnlineTime = atoi(sUpTime);
	sprintf(onlineTime, "%d", nOnlineTime);	
	SL_DEBUG("get online duration value: %s.", onlineTime);
	return;
}


void router_getLinkSSID(char *ssid, size_t len)
{
	SL_DEBUG("begin to get link SSID.");
	char * pSSID = nvram_get(RT2860_NVRAM, "SSID1");
	strncpy(ssid, pSSID, len);
	SL_DEBUG("get link SSID value: %s.", ssid);
	return;
}

void router_getFilewareStatus(char *status, size_t len)
{
	char *firewall_enable;
	SL_DEBUG("begin to get fireware status.");
	firewall_enable = (char *)nvram_bufget(RT2860_NVRAM, "IPPortFilterEnable");
	if(firewall_enable && atoi(firewall_enable)){         
        status[0] ='1';
    } else {
    	status[0] = '2';
    }

	SL_DEBUG("get firewall status value: %s.", status);
	
	return;
}


//TODO 
void router_getDownlinkRate(char *downRate)
{
	SL_DEBUG("begin to get down link rate.");
	strcpy(downRate, "10");	
	SL_DEBUG("get down link rate value: %s.", downRate);
	
	return;
}


//TODO
void router_getUplinkRate(char *upRate) {
	SL_DEBUG("begin to get uplink rate.");
	strcpy(upRate, "12");	
	SL_DEBUG("get uplink rate value: %s.", upRate);
	
	return;
}

void router_getWifiStatus(int nvram_id, char *wifiStatus, size_t len)
{
	SL_DEBUG("begin to get wifi status.");
	char *pWifi = nvram_get(nvram_id, "WiFiOff");
	if (!strcmp(pWifi, "1")) {
		strcpy(wifiStatus, "2");
	} else {
		strcpy(wifiStatus, "1");
	}
	SL_DEBUG("get wifi status value: %s.", wifiStatus);
	
	return;
}

#if defined (CONFIG_PRODUCT_339AA) || defined (CONFIG_PRODUCT_339AA_JS) || defined (CONFIG_PRODUCT_331AA)
int router_getConnectAPDevices(struct RouterClientList *prClientList)
{
	int i, j, k, fd, accessInternet;
	recRouterClient *prClientNode;
	char *pTemp;
	struct in_addr addr;
	struct iwreq iwr;
	RT_802_11_MAC_TABLE table = {0};
	//Get Online Users
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	strncpy(iwr.ifr_name, "ra0", IFNAMSIZ);

	iwr.u.data.pointer = (caddr_t) &table;
	if (fd < 0) {
		SL_DEBUG("open socket fail!");
		return -1;
	}

	if (ioctl(fd, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &iwr) < 0) {
		SL_DEBUG("ioctl -> RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT fail!");
		close(fd);
		return -1;
	}
	close(fd);

	char sWirelessClients[8192]= {0};
	char rec[128] = {0};
	char cli_mac[32] = {0};
	char cli_ip[32] = {0};
	char hostname[64] = {0};
	router_getCfgValue(ROUTER_GET_WIRELESS_CLIENTS, sWirelessClients, 4096);

	for (i = 0; i < table.Num; i++) {
			
		RT_802_11_MAC_ENTRY *pe = &(table.Entry[i]);
		
		prClientNode = malloc(sizeof(recRouterClient));
		if (prClientNode == NULL) {
			SL_DEBUG("memory is not enough.");
			return -1;
		}
		memset(prClientNode, 0, sizeof(recRouterClient));
		
		//1. macAddr
		pTemp = prClientNode->macAddr;
		sprintf(pTemp, "%02X", pe->Addr[0]);
		pTemp += 2; 	
		
		for (j = 1; j < 6; j++) {
			sprintf(pTemp, ":%02X", pe->Addr[j]);
			pTemp += 3;
		}

		//4. onlineTime
		sprintf(prClientNode->onlineDuration, "%d", pe->ConnectedTime);

		k = 0;
		/* 7C:04:D0:6E:27:58,192.168.1.15,iPhone; */
		while((get_nth_value(k++, sWirelessClients, ';', rec, sizeof(rec)) != -1) && strlen(rec)) {
			if((get_nth_value(0, rec, ',', cli_mac, sizeof(cli_mac)) == -1)){
				continue;
			}

			if (!strcmp(prClientNode->macAddr, cli_mac))
			{
				if((get_nth_value(1, rec, ',', cli_ip, sizeof(cli_ip)) == -1)){
					continue;
				}
				strcpy(prClientNode->ipAddr, cli_ip);

				if((get_nth_value(2, rec, ',', hostname, sizeof(hostname)) == -1)){
					continue;
				}
				strcpy(prClientNode->name, hostname);
			}
		}

		//add one node to list 
		SLIST_INSERT_HEAD(prClientList, prClientNode, next);
	}
	
	return 0;
}

#endif

int router_getConnectDevices(struct RouterClientList *prClientList)
{
	FILE *fp;
	struct dhcpOfferedAddr {
		unsigned char hostname[16];
		unsigned char mac[16];
		unsigned long ip;
		unsigned long expires;
	} lease;

	int i, j, fd, accessInternet;
	recRouterClient *prClientNode;
	char *pTemp;
	struct in_addr addr;
	struct iwreq iwr;
	RT_802_11_MAC_TABLE table = {0};
	//Get Online Users
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(iwr.ifr_name, "ra0", IFNAMSIZ);
	iwr.u.data.pointer = (caddr_t) &table;
	if (fd < 0) {
		SL_DEBUG("open socket fail!");
		return -1;
	}

	if (ioctl(fd, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &iwr) < 0) {
		SL_DEBUG("ioctl -> RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT fail!");
		close(fd);
		return -1;
	}
	close(fd);

	//Get DHCP Users
	do_system("killall -q -USR1 udhcpd");
	fp = fopen("/var/udhcpd.leases", "r");
	if (NULL == fp) {
		SL_DEBUG("dhcp file not exists.");
		return -1;
	}
	while (fread(&lease, 1, sizeof(lease), fp) == sizeof(lease)) {

		for (i = 0; i < table.Num; i++) {
			
			RT_802_11_MAC_ENTRY *pe = &(table.Entry[i]);
			 
			//mac compare
			if (!strncmp(pe->Addr, lease.mac, 5)) {
				
				prClientNode = malloc(sizeof(recRouterClient));
				if (prClientNode == NULL) {
					SL_DEBUG("memory is not enough.");
					return -1;
				}
				memset(prClientNode, 0, sizeof(recRouterClient));

				
				//1. macAddr
				pTemp = prClientNode->macAddr;
				sprintf(pTemp, "%02X", lease.mac[0]);
				pTemp += 2; 	
				
				for (j = 1; j < 6; j++) {
					sprintf(pTemp, ":%02X", lease.mac[j]);
					pTemp += 3;
				}

				//2. deviceName
				if (strlen(lease.hostname) > 0) {
					strcpy(prClientNode->name, lease.hostname);
				}

				//3. ipAddr
				addr.s_addr = lease.ip;
				strcpy(prClientNode->ipAddr, inet_ntoa(addr));

				//4. onlineTime
				sprintf(prClientNode->onlineDuration, "%d", pe->ConnectedTime);

                //5. lastRxRate
             //   sprintf(prClientNode->clientDownlinkRate, "%d", pe->LastRxRate/1024);
				router_getClientFlow(prClientNode->ipAddr,prClientNode->clientDownlinkRate,prClientNode->clientUplinkRate);

                //6. accessInternet
                accessInternet = router_getClientAccessRight(prClientNode->macAddr);
                if (accessInternet == 2) {
                    prClientNode->accessInernet[0] = '0';
                } else {
                    prClientNode->accessInernet[0] = '1';
                }
				
				
				//add by luozf qos rule
				router_getQosRule(prClientNode->macAddr,prClientNode->qosSwitch,\
							prClientNode->maxUplinkRate,prClientNode->maxDownlinkRate);
                
				//add one node to list 
				SLIST_INSERT_HEAD(prClientList, prClientNode, next);
				break;

			}
			
		}

	}
	
	fclose(fp);
	return 0;
}


#ifdef WIRELESS_5G
int router_getConnect_5G_Devices(struct RouterClientList *prClientList)
{
	FILE *fp;
	struct dhcpOfferedAddr {
		unsigned char hostname[16];
		unsigned char mac[16];
		unsigned long ip;
		unsigned long expires;
	} lease;

	int i, j, fd, accessInternet;
	recRouterClient *prClientNode;
	char *pTemp;
	struct in_addr addr;
	struct iwreq iwr;
	RT_802_11_MAC_TABLE table = {0};
	//Get Online Users
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(iwr.ifr_name, "rai0", IFNAMSIZ);
	iwr.u.data.pointer = (caddr_t) &table;
	if (fd < 0) {
		SL_DEBUG("open socket fail!");
		return -1;
	}

	if (ioctl(fd, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &iwr) < 0) {
		SL_DEBUG("ioctl -> RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT fail!");
		close(fd);
		return -1;
	}
	close(fd);

	//Get DHCP Users
	do_system("killall -q -USR1 udhcpd");
	fp = fopen("/var/udhcpd.leases", "r");
	if (NULL == fp) {
		SL_DEBUG("dhcp file not exists.");
		return -1;
	}
	while (fread(&lease, 1, sizeof(lease), fp) == sizeof(lease)) {

		for (i = 0; i < table.Num; i++) {
			
			RT_802_11_MAC_ENTRY *pe = &(table.Entry[i]);
			 
			//mac compare
			if (!strncmp(pe->Addr, lease.mac, 5)) {
				
				prClientNode = malloc(sizeof(recRouterClient));
				if (prClientNode == NULL) {
					SL_DEBUG("memory is not enough.");
					return -1;
				}
				memset(prClientNode, 0, sizeof(recRouterClient));

				
				//1. macAddr
				pTemp = prClientNode->macAddr;
				sprintf(pTemp, "%02X", lease.mac[0]);
				pTemp += 2; 	
				
				for (j = 1; j < 6; j++) {
					sprintf(pTemp, ":%02X", lease.mac[j]);
					pTemp += 3;
				}

				//2. deviceName
				if (strlen(lease.hostname) > 0) {
					strcpy(prClientNode->name, lease.hostname);
				}

				//3. ipAddr
				addr.s_addr = lease.ip;
				strcpy(prClientNode->ipAddr, inet_ntoa(addr));

				//4. onlineTime
				sprintf(prClientNode->onlineDuration, "%d", pe->ConnectedTime);

                //5. lastRxRate
             //   sprintf(prClientNode->clientDownlinkRate, "%d", pe->LastRxRate/1024);
				router_getClientFlow(prClientNode->ipAddr,prClientNode->clientDownlinkRate,prClientNode->clientUplinkRate);

                //6. accessInternet
                accessInternet = router_getClientAccessRight(prClientNode->macAddr);
                if (accessInternet == 2) {
                    prClientNode->accessInernet[0] = '0';
                } else {
                    prClientNode->accessInernet[0] = '1';
                }
				
				
				//add by luozf qos rule
				router_getQosRule(prClientNode->macAddr,prClientNode->qosSwitch,\
							prClientNode->maxUplinkRate,prClientNode->maxDownlinkRate);
                
				//add one node to list 
				SLIST_INSERT_HEAD(prClientList, prClientNode, next);
				break;

			}
			
		}

	}
	
	fclose(fp);
	return 0;
}
#endif

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
				SL_DEBUG("format error");
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

void router_getSplitValue(char *record, char *split, int index, char *result)
{
    int nPos = 0;
    char *pTok= NULL;
    pTok = strtok(record, split);
    while(NULL != pTok)
    {
         if (nPos + 1 == index) {
			strcpy(result, pTok);
			break;
		 }
         nPos++;
         pTok = strtok(NULL, split);	 
    }
    
    return;
}

int router_getSSIDIndex(recRouterSSIDTable *pSSIDTable, const char *ssid)
{
    int i = 0, ssidIndex = 0;
    recRouterSSIDEntry *pEntry =NULL;
    
    for(i = 0; i <= pSSIDTable->num; i++) {
        pEntry = &(pSSIDTable->Entry[i]);
        if (!strcmp(pEntry->ssid, ssid)) {
            ssidIndex = pEntry->index;
        }
    }

    return ssidIndex;
}


void router_getSSIDTable(recRouterSSIDTable *pSSIDTable)
{
    int Index = 0, num =0;
    char ssidName[10] = {0};
    char *ssid;
    
    for(Index =1; Index <= MAX_SSID_NUM; Index++) {
        
        memset(ssidName, 0, sizeof(ssidName));
        sprintf(ssidName, "SSID%d", Index);
        ssid = nvram_get(RT2860_NVRAM, ssidName);
        if (strlen(ssid) > 0) {
            num = pSSIDTable->num;
            strcpy(pSSIDTable->Entry[num].ssid, ssid);
            pSSIDTable->Entry[num].index = Index;
            pSSIDTable->num += 1;  
        }
    }       
}

/**
    Return Value  2 : Access, 1 : Reject
**/
int router_getClientAccessRight(const char* macAddr)
{
    char *rules = NULL, *firewall_enable = NULL, *default_policy = NULL;
    char rec[256] = {0};
    char sMacAddr[32] = {0};
    char action[2] = {0};
    int accessRight, i = 0;

    //1. 没有设置防火墙开关或防火墙开关为disable状态， 用户都会有访问权限
    firewall_enable = (char *)nvram_bufget(RT2860_NVRAM, "IPPortFilterEnable");
    if(!firewall_enable || !atoi(firewall_enable)) return 2; //Accept

    //2. 查看IP策略 0: Accepted, 1: Rejected
    default_policy = (char *)nvram_bufget(RT2860_NVRAM, "DefaultFirewallPolicy");
    if (!default_policy || !strlen(default_policy)) return 2; //Accept
    
    if (atoi(default_policy) ==0) {
        accessRight = 2;
    } else {
        accessRight = 1;
    }
    
    //3. 查看MAC/IP过滤规则
    rules = nvram_bufget(RT2860_NVRAM, "IPPortFilterRules");
	while (get_nth_value(i, rules, ';', rec, sizeof(rec)) != -1 && strlen(rec)) {
		SL_DEBUG("i=%d, rec=%s, strlen(rec)=%d", i, rec, strlen(rec));
        
		// get mac address
		if((get_nth_value(11, rec, ',', sMacAddr, sizeof(sMacAddr)) == -1)) {
			SL_DEBUG("macaddr[%s] continue", sMacAddr);
			i++;
			continue;
		}

        // get action
		if((get_nth_value(9, rec, ',', action, sizeof(action)) == -1)) {
			SL_DEBUG("action[%s] continue", sMacAddr);
			i++;
			continue;
		}

		SL_DEBUG("macaddr[%s], action[%s]", sMacAddr, action);

        // macaddr rules filter
        if (!strcmp(sMacAddr, macAddr)) {
            switch(atoi(action)) {
                case ACTION_DROP :
                    accessRight = 1;
                    break;
                case ACTION_ACCEPT :
                    accessRight = 2;
                    break;
            }
            break;
        }

		i++;

    }

    return accessRight;
}

void router_makeIPPortFilterRule(char *buf, int len, char *mac_address,
    char *sip_1, char *sip_2, int sprf_int, int sprt_int, 
    char *dip_1, char *dip_2, int dprf_int, int dprt_int, int proto, int action)
{
    int rc = 0;
    char *pos = buf;
    char *spifw = (char *)nvram_bufget(RT2860_NVRAM, "SPIFWEnabled");

    switch(action){
    case ACTION_DROP:
        if (atoi(spifw) == 0)
    	rc = snprintf(pos, len-rc, 
    		"iptables -A %s ", IPPORT_FILTER_CHAIN);
        else
    	rc = snprintf(pos, len-rc, 
    		"iptables -A %s -m state --state NEW,INVALID ", IPPORT_FILTER_CHAIN);
    	break;
    case ACTION_ACCEPT:
    	rc = snprintf(pos, len-rc, 
    		"iptables -A %s ", IPPORT_FILTER_CHAIN);
    	break;
    }
    pos = pos + rc;

    // write mac address
    if(mac_address && strlen(mac_address)){
    	rc = snprintf(pos, len-rc, "-m mac --mac-source %s ", mac_address);
    	pos = pos+rc;
    }

    // write source ip
    rc = snprintf(pos, len-rc, "-s %s ", sip_1);
    pos = pos+rc;

    // write dest ip
    rc = snprintf(pos, len-rc, "-d %s ", dip_1);
    pos = pos+rc;

    // write protocol type
    if(proto == PROTO_NONE){
    	rc = snprintf(pos, len-rc, " ");
    	pos = pos + rc;
    }else if(proto == PROTO_ICMP){
    	rc = snprintf(pos, len-rc, "-p icmp ");
    	pos = pos + rc;
    }else{
    	if(proto == PROTO_TCP)
    		rc = snprintf(pos, len-rc, "-p tcp ");
    	else if (proto == PROTO_UDP)
    		rc = snprintf(pos, len-rc, "-p udp ");
    	pos = pos + rc;

    	// write source port
    	if(sprf_int){
    		if(sprt_int)
    			rc = snprintf(pos, len-rc, "--sport %d:%d ", sprf_int, sprt_int);
    		else
    			rc = snprintf(pos, len-rc, "--sport %d ", sprf_int);
    		pos = pos+rc;
    	}

    	// write dest port
    	if(dprf_int){
    		if(dprt_int)
    			rc = snprintf(pos, len-rc, "--dport %d:%d ", dprf_int, dprt_int);
    		else
    			rc = snprintf(pos, len-rc, "--dport %d ", dprf_int);
    		pos = pos+rc;
    	}
    }

    switch(action){
    case ACTION_DROP:			// 1 == ENABLE--DROP mode
    	rc = snprintf(pos, len-rc, "-j DROP");
    	break;
    case ACTION_ACCEPT:			// 2 == ENABLE--ACCEPT mode
    	rc = snprintf(pos, len-rc, "-j ACCEPT");
    	break;
    }
}


void router_iptablesIPPortFilterFlush() 
{
    do_system("iptables -F %s 1>/dev/null 2>&1", IPPORT_FILTER_CHAIN);
}

void router_iptablesIPPortFilterRun()
{

    int i = 0;
    char rec[256];
    int sprf_int, sprt_int, proto, action;
    int dprf_int, dprt_int;
    char sprf[8], sprt[8], protocol[8];
    char dprf[8], dprt[8];
    char mac_address[32];
    char sip_1[32], sip_2[32], action_str[4];
    char dip_1[32], dip_2[32];
    char sCmd[256];

    char *default_policy = (char *)nvram_bufget(RT2860_NVRAM, "DefaultFirewallPolicy");
    char *spifw = (char *)nvram_bufget(RT2860_NVRAM, "SPIFWEnabled");
  
    if(atoi(default_policy) == 1) {
		//the default policy is drop
		if (atoi(spifw) == 0) {
			; //do_system("iptables -t filter -A %s -j ACCEPT", IPPORT_FILTER_CHAIN);
		} else {
			do_system("iptables -t filter -A %s -m state --state RELATED,ESTABLISHED -j ACCEPT", IPPORT_FILTER_CHAIN);
		}
	}

    char *rule = (char *)nvram_bufget(RT2860_NVRAM, "IPPortFilterRules");
    while( (get_nth_value(i++, rule, ';', rec, sizeof(rec)) != -1) ) {

        // get sip 1
        if((get_nth_value(0, rec, ',', sip_1, sizeof(sip_1)) == -1)) {
            continue;
        }
        if(!is_ipnetmask_valid(sip_1)) {
            continue;
        }

        // we dont support ip range yet.
        // get sip 2
        //if((get_nth_value(1, rec, ',', sip_2, sizeof(sip_2)) == -1))
        //  continue;
        //if(!is_ip_valid(sip_2))
        //  continue;

        // get source port range "from"
        if((get_nth_value(2, rec, ',', sprf, sizeof(sprf)) == -1)) {
            continue;
        }

        if( (sprf_int = atoi(sprf)) > 65535)
            continue;
        // get dest port range "to"
        if((get_nth_value(3, rec, ',', sprt, sizeof(sprt)) == -1)) {
            continue;
        }

        if( (sprt_int = atoi(sprt)) > 65535)
            continue;

        // Destination Part
        // get dip 1
        if((get_nth_value(4, rec, ',', dip_1, sizeof(dip_1)) == -1)) {
            continue;
        }

        if(!is_ipnetmask_valid(dip_1)){
            continue;
        }

        // we dont support ip range yet
        // get sip 2
        //if((get_nth_value(5, rec, ',', dip_2, sizeof(dip_2)) == -1))
        //    continue;
        //if(!is_ip_valid(dip_2))
        //    continue;

        // get source port range "from"
        if((get_nth_value(6, rec, ',', dprf, sizeof(dprf)) == -1)) {
            continue;
        }

        if( (dprf_int = atoi(dprf)) > 65535)
            continue;

        // get dest port range "to"
        if((get_nth_value(7, rec, ',', dprt, sizeof(dprt)) == -1)) {
            continue;
        }

        if( (dprt_int = atoi(dprt)) > 65535)
            continue;


        // get protocol
        if((get_nth_value(8, rec, ',', protocol, sizeof(protocol)) == -1))
            continue;
        proto = atoi(protocol);

        // get action
        if((get_nth_value(9, rec, ',', action_str, sizeof(action_str)) == -1)) {
            continue;
        }
        action = atoi(action_str);

        // get_nth_value(10) is "comment".

        // get mac address
        if((get_nth_value(11, rec, ',', mac_address, sizeof(mac_address)) == -1))
            continue;
        if(strlen(mac_address)){
            if(!is_mac_valid(mac_address))
                continue;
        }

        //TODO:
        // supposed to do validation here but  we didn't do it because code size.
       /*
            # iptables example
            # iptables -t nat -A POSTROUTING -o eth0  -s 10.10.10.0/24 -j MASQUERADE
            # iptables -A FORWARD -m physdev --physdev-in ra0 --physdev-out eth2 -m state --state ESTABLISHED,RELATED -j ACCEPT
            # iptables -A FORWARD -m physdev --physdev-in eth0 --physdev-out eth2 -j DROP
            # iptables -A FORWARD -i eth0 -o eth2 -j DROP
            # iptables -A FORWARD -p tcp --dport 139 -j DROP
            # iptables -A FORWARD -i eth0 -o eth2 -m state --state NEW,INVALID -p tcp --dport 80 -j DROP
        */
        //1. 设置过滤规则
        memset(sCmd, 0 ,sizeof(sCmd));
        router_makeIPPortFilterRule(sCmd, sizeof(sCmd), mac_address, sip_1, sip_2, sprf_int, sprt_int,
                                                        dip_1, dip_2, dprf_int, dprt_int, proto, action);
        do_system(sCmd);
    }

    //2. 设置过滤策略
    switch(atoi(default_policy)){
	case 0:
		do_system("iptables -t filter -A %s -j ACCEPT", IPPORT_FILTER_CHAIN);
		break;
	case 1:
		do_system("iptables -t filter -A %s -j DROP", IPPORT_FILTER_CHAIN);
		break;
	}

}

void router_getDefaultAccessValue(char *firewallFiterValue, int len, int proto, int right, const char* macAddr)
{
	snprintf(firewallFiterValue, len, "any/0,0,0,0,any/0,0,0,0,%d,%d,,%s", proto, right, macAddr);
}

/**
    accessFlag 2: Access, 1: Reject

**/

void router_setClientAccessRight(const char* macAddr, int accessFlag)
{
    char rule[8192] = {0};
    char *firewall_enable, *IPPortFilterRules;
    int accessRight = ACTION_ACCEPT;
    
    if (accessFlag == 1) accessRight = ACTION_DROP;
	
    firewall_enable = (char *)nvram_bufget(RT2860_NVRAM, "IPPortFilterEnable");
    if(!firewall_enable) {
        SL_DEBUG("Warning: can't find \"IPPortFilterEnable\" in flash.");
        return;
    }
    
    if (atoi(firewall_enable) == 0) {
        SL_DEBUG("IPPortFilterEnable =0, No need to set IPPortFilterRules");
        return;
    }
    
	IPPortFilterRules = (char *)nvram_bufget(RT2860_NVRAM, "IPPortFilterRules");
	if(!IPPortFilterRules){
		SL_DEBUG("Warning: can't find \"IPPortFilterRules\" in flash.");
		return;
	}

    if(strlen(IPPortFilterRules) == 0) {
		router_getDefaultAccessValue(rule, sizeof(rule) -1, PROTO_TCP, accessRight, macAddr);

	} else {

		//1. 避免重复设置
		char *pos =NULL;
		int len = 0;
		char sTemp[64] = {0};
		router_getDefaultAccessValue(sTemp, sizeof(sTemp) -1, PROTO_TCP, accessRight, macAddr);
		if (strstr(IPPortFilterRules, sTemp)) {
			SL_DEBUG("%s in firewall rules, not need to set.", sTemp);
			return;
		}

		strcpy(rule, IPPortFilterRules);

		//2. 删除掉有相反的设置	
		memset(sTemp, 0, sizeof(sTemp));
		router_getDefaultAccessValue(sTemp, sizeof(sTemp) -1, PROTO_TCP,  1- accessRight, macAddr);
		if (strstr(rule, sTemp)) {
			SL_DEBUG("exists anti-rule[%s] in rules[%s]", sTemp, rule);
			char *replace_result = NULL;
			replace_result = router_strReplace(rule, sTemp, "");
			if (replace_result != NULL) {
				memset(rule, 0, sizeof(rule));
				strcpy(rule, replace_result);
				free(replace_result);
			}
			
			//删除多余的";"
			replace_result = router_strReplace(rule, ";;", ";");
			if (replace_result != NULL) {
				memset(rule, 0, sizeof(rule));
				strcpy(rule, replace_result);
				free(replace_result);
			}

			//删除头部/尾部的;字符
			router_trim(rule, ';');
			
		}

		len = strlen(rule);
		if (len > 0) {
			pos = rule + len;
			memset(sTemp, 0, sizeof(sTemp));
			router_getDefaultAccessValue(sTemp, sizeof(sTemp)-1, PROTO_TCP, accessRight, macAddr);
			snprintf(pos, sizeof(rule)-len -1, ";%s", sTemp);
		} else {
			router_getDefaultAccessValue(rule, sizeof(rule) -1, PROTO_TCP, accessRight, macAddr);
		}
	}

	SL_DEBUG("final firewall rules[%s].", rule);
    nvram_set(RT2860_NVRAM, "IPPortFilterRules", rule);
    nvram_commit(RT2860_NVRAM);

    //1. 清空iptables规则表
    router_iptablesIPPortFilterFlush();

    //2. 重新设置防火墙基本规则， 进行ip和端口过滤设置
    router_iptablesIPPortFilterRun();
    
    return;
}
//add by luozf

void router_getClientFlow(const char* ipAddr,char* clientDownLinkRate,char * clientUpLinkRate)
{
	char sStatClients[4096]= {0};
	char rec[128] = {0};
	char cli_ip[16] = {0};
	char RxRate[12] = {0};
	char TxRate[12] = {0};

	router_getCfgValue(ROUTER_GET_STAT_CLIENTS, sStatClients, sizeof(sStatClients));

	int k= 0;
	/* IPAddress,in,ou;IPAddress,in,ou; */
	if (strlen(ipAddr) >0) {
		while((router_getNthValue(k++, sStatClients, ';', rec, sizeof(rec)) != -1) && strlen(rec)) {
			if((router_getNthValue(0, rec, ',', cli_ip, sizeof(cli_ip)) == -1)){
				continue;
			}
			
			if (!strcmp(ipAddr, cli_ip)) {
				//6. RxRate
				if((router_getNthValue(1, rec, ',', RxRate, sizeof(RxRate)) == -1)){
					continue;
				}
				strcpy(clientDownLinkRate,  RxRate);
	
				//7. TxRate
				if((router_getNthValue(2, rec, ',', TxRate, sizeof(TxRate)) == -1)){
					continue;
				}
				strcpy(clientUpLinkRate, TxRate);
				
				break;
			}
		}
	}
	

	return;
}

void router_getUpAndDownLinkRate(char* upLinkRate, char *downLinkRate)
{
	char buf[1024];
	
	FILE* fp=NULL;
	int upLinkSpeed = 0;
	int downLinkSpeed = 0;
	
	if(access("/tmp/speed.txt",0)==0)
	{
		fp=fopen("/tmp/speed.txt","r");
		{
			while(fgets(buf,sizeof(buf),fp))
		  	{
				sscanf(buf,"%d,%d",&upLinkSpeed,&downLinkSpeed);
				sprintf(upLinkRate,"%d",upLinkSpeed);
				sprintf(downLinkRate,"%d",downLinkSpeed);
				
				SL_DEBUG("+++zero_debug+++====upLinkRate=%s,downLinkRate=%s\n",upLinkRate,downLinkRate);
		  	}
	  	}   
	  fclose(fp);
	}

	return;
}



void router_getQosRule(const char* macAddr,char *qosEnable,char* maxUpRate,char * maxDownRate)
{
    char *rules = NULL, *qos_enable = NULL;
    char rec[512] = {0};
    char sMacAddr[32] = {0};
    char client_Enable[8] = {0};
    char client_maxUpRate[32] = {0};
    char client_maxDownRate[32] = {0};
    int  i = 0;
	int flag = 0;
    
    //3. 查看MAC/IP过滤规则
    rules = nvram_bufget(RT2860_NVRAM, "QoSRules_zw");
	while (get_nth_value(i, rules, ';', rec, sizeof(rec)) != -1 ) {
		SL_DEBUG("+++++++zero_debug+++++++i=%d, rec=%s, strlen(rec)=%d", i, rec, strlen(rec));

		//comment, status, ip_address,macAddr,max_upload_speed,max_download_speed
		memset(sMacAddr,0,sizeof(sMacAddr));
		memset(client_Enable,0,sizeof(client_Enable));		
		memset(client_maxUpRate,0,sizeof(client_maxUpRate));
		memset(client_maxUpRate,0,sizeof(client_maxUpRate));

		// get mac address
		if((get_nth_value(2, rec, ',', sMacAddr, sizeof(sMacAddr)) == -1)) {
			SL_DEBUG("macaddr[%s] continue", sMacAddr);
			i++;
			continue;
		}

        // get qosEnable
		if((get_nth_value(1, rec, ',', client_Enable, sizeof(client_Enable)) == -1)) {
			SL_DEBUG("client_Enable[%s] continue", sMacAddr);
			i++;
			continue;
		}

		 // get  client_maxUPRate
		if((get_nth_value(3, rec, ',', client_maxUpRate, sizeof(client_maxUpRate)) == -1)) {
			SL_DEBUG("client_maxUpRate[%s] continue", sMacAddr);
			i++;
			continue;
		}

		 // get client_maxDownRate
		if((get_nth_value(4, rec, ',', client_maxDownRate, sizeof(client_maxDownRate)) == -1)) {
			SL_DEBUG("client_maxDownRate[%s] continue", sMacAddr);
			i++;
			continue;
		}

		SL_DEBUG("+++++++zero_debug+++++++macaddr[%s], client_Enable[%s],client_maxUpRate[%s],client_maxDownRate[%s]", 
					sMacAddr, client_Enable,client_maxUpRate,client_maxDownRate);

        // macaddr rules filter
        if (0 == strcasecmp(sMacAddr, macAddr)) 
		{
			 flag = 1;
          	 strncpy(qosEnable,client_Enable,sizeof(client_Enable));
			 strncpy(maxUpRate,client_maxUpRate,sizeof(client_maxUpRate));
			 strncpy(maxDownRate,client_maxDownRate,sizeof(client_maxDownRate));
			 return ;
        }

		i++;

    }

	if (0 == flag)
	{
		//附初始值
		strcpy(qosEnable,"0");
		strcpy(maxUpRate,"0");
		strcpy(maxDownRate,"0");
	}
    return ;
}

static int router_getQosRuleToList(struct RouterQosRuleList *prClientList)
{
	
	int i, j, rule_count;
	char *new_rules;
	const char *rules = nvram_bufget(RT2860_NVRAM, "QoSRules_zw");
	recQosRuleClient *prClientNode;


	if(!rules || !strlen(rules) )
		return -1;

	new_rules = strdup(rules);
	if(!new_rules){		
		SL_DEBUG("memory is not enough.");
		return -1;
	}
	char *buf=new_rules;
	char *outer_ptr=NULL;
	char *inner_ptr=NULL;
	char *ptr = NULL;
	
	while((ptr=strtok_r(buf,";",&outer_ptr))!=NULL) {		
		buf=ptr;		
		int in = 0;
		prClientNode = malloc(sizeof(recQosRuleClient));
		if (prClientNode == NULL) {
			SL_DEBUG("memory is not enough.");
			return -1;
		}
		memset(prClientNode, 0, sizeof(recQosRuleClient));
		
		while((ptr=strtok_r(buf,",",&inner_ptr))!=NULL) 
		{	
			//提出rule数据填充到结构体中comment, status, ip_address,macAddr,max_upload_speed,max_download_speed
			SL_DEBUG("+++++++zero_debug+++++++%s %d  in=%d,ptr=%s\n",__func__,__LINE__,in,ptr);
			if(0 == in)
				strncpy(prClientNode->name,ptr,sizeof(prClientNode->name));
		
			if(1 == in)	
				strncpy(prClientNode->qosSwitch,ptr,sizeof(prClientNode->qosSwitch));
			
			if(2 == in)
				strncpy(prClientNode->macAddr,ptr,sizeof(prClientNode->macAddr));
			
			if(3 == in)
				strncpy(prClientNode->maxUplinkRate,ptr,sizeof(prClientNode->maxUplinkRate));
			
			if(4 == in)
				strncpy(prClientNode->maxDownlinkRate,ptr,sizeof(prClientNode->maxDownlinkRate));
			
			in++;			
			buf=NULL;		
		}		
		buf=NULL; 	
		//add one node to list 
		SLIST_INSERT_HEAD(prClientList, prClientNode, next);
	}
	
	return 0;
}


void router_setQosRule(const char* macAddr, int enable,const char* maxUpRate,const char* maxDownRate)
{
	
	char *qos_enable, *upload_bandwidth, *download_bandwidth;
	char rule[8192];
	const char *old_rule;
	char ip_address[32] = {0};
	char comment[128] = {0};
	int  mac_flag = 0;
	
	//设置总开关和总带宽
	qos_enable=nvram_bufget(RT2860_NVRAM, "QoSEnable_zw");
	upload_bandwidth=nvram_bufget(RT2860_NVRAM, "QoSUploadBandwidth_zw");
	download_bandwidth=nvram_bufget(RT2860_NVRAM, "QoSDownloadBandwidth_zw");

	//默认总开关开启
	if(!qos_enable || !strlen(qos_enable))
	{
		nvram_set(RT2860_NVRAM, "QoSEnable_zw", "1");
	}
	//默认设置网口的带宽连接速率100M
	if(!upload_bandwidth || !strlen(upload_bandwidth))
	{
		nvram_set(RT2860_NVRAM, "QoSUploadBandwidth_zw", ROUTER_PHY_RATE);
	}
	//默认设置网口的带宽连接速率100M
	if(!download_bandwidth || !strlen(download_bandwidth))
	{
		nvram_set(RT2860_NVRAM, "QoSDownloadBandwidth_zw", ROUTER_PHY_RATE);
	}

	//根据ip地址生成comment
	generate_commnet_by_macAddr(macAddr,comment);
	
	SL_DEBUG("+++++++zero_debug+++++comment=%s!",comment);
	
	old_rule = nvram_bufget(RT2860_NVRAM, "QoSRules_zw");

	//没有规则的时候
	if(!old_rule || !strlen(old_rule))
	{
		snprintf(rule, sizeof(rule), "%s,%d,%s,%s,%s", comment, enable,macAddr,maxUpRate,maxDownRate);
	}
	else
	{
		//至少包含一条规则；
		struct RouterQosRuleList clientHead;
		SLIST_INIT(&clientHead);
		router_getQosRuleToList(&clientHead);	//把现有的规则放在单链表中
		recQosRuleClient *prClientNode;
		memset(rule,0,8192);
		int i = 0;
		SLIST_FOREACH(prClientNode,&clientHead,next)
		{
			SL_DEBUG("+++++Zero_debug++++[%s %d]=====i=%d,macAddr=%s,prClientNode->macAddr=%s\n",__func__,__LINE__,
						i,macAddr,prClientNode->macAddr);
			char tempRule[256] = {0};
			//mac地址存在，则更新规则
			if(0 == strcasecmp(prClientNode->macAddr,macAddr))
			{	
				SL_DEBUG("+++++Zero_debug++++[%s %d]=====replase data\n",__func__,__LINE__);
				strncpy(prClientNode->name,comment,sizeof(prClientNode->name));
			//	strncpy(prClientNode->ipAddr,ip_address,sizeof(prClientNode->ipAddr));
				sprintf(prClientNode->qosSwitch,"%d",enable);	
				strncpy(prClientNode->maxUplinkRate,maxUpRate,sizeof(prClientNode->maxUplinkRate));
				strncpy(prClientNode->maxDownlinkRate,maxDownRate,sizeof(prClientNode->maxDownlinkRate));
				
				mac_flag = 1;//找到标志
			}
			memset(tempRule,0,256);
			snprintf(tempRule, sizeof(tempRule), "%s,%s,%s,%s,%s", \
					prClientNode->name, prClientNode->qosSwitch, \
					prClientNode->macAddr,prClientNode->maxUplinkRate,prClientNode->maxDownlinkRate);
			
			strcat(rule,tempRule);
			strcat(rule,";");//最后的数据多了一个;保存nvram的时候需要删除掉

			SL_DEBUG("+++++Zero_debug++++modify[%s %d]======rule=%s\n",__func__,__LINE__,rule);	
			i++;
			//释放router_getQosRule函数的malloc
			free(prClientNode);
		}
		//在qos规则中没有找到，则意味追加到规则
		if(mac_flag == 0)
		{
			SL_DEBUG("+++++Zero_debug++++[%s %d]======mac_flag==0 add to back\n",__func__,__LINE__);
			memset(rule,0,8192);
			snprintf(rule, sizeof(rule), "%s;%s,%d,%s,%s,%s", old_rule, comment, enable,macAddr,maxUpRate,maxDownRate);
		}
		else
		{
			//去掉尾部分号；
			if(rule[strlen(rule)-1] == ';')
				rule[strlen(rule)-1] = '\0';
		}
	}
	SL_DEBUG("+++++Zero_debug++++[%s %d]======nvarm set rule=%s\n",__func__,__LINE__,rule);

	nvram_set(RT2860_NVRAM, "QoSRules_zw", rule);
	nvram_commit(RT2860_NVRAM);
	do_system("init_system restart");
}
//end of add

void router_getUpLinkSignal(const char* upLinkSSID, char *upLinkSignal)
{
    FILE *pp;
    char cmd[CMDLEN], *ptr;
    char channel[4], ssid[186], bssid[20], security[23];
    char signal[9], mode[7], ext_ch[7], net_type[3];
    int i, space_start;
	
	SL_DEBUG("router_getUpLinkSignal begin!");
    memset(cmd, 0, sizeof(cmd));
    do_system("iwpriv ra0 set SiteSurvey=1");
    strcpy(cmd, "iwpriv ra0 get_site_survey");
    if(!(pp = popen(cmd, "r"))) {
        SL_DEBUG("execute get_site_survey fail!");
        strcpy(upLinkSignal, "3"); // 默认为: 弱
        return ;
    }

    memset(cmd, 0, sizeof(cmd));
    fgets(cmd, sizeof(cmd), pp);
    fgets(cmd, sizeof(cmd), pp);
    while (fgets(cmd, sizeof(cmd), pp)) {
		
        if (strlen(cmd) < 4) {
			SL_DEBUG("cmd exception :%s.", cmd);
			break;
        }
        ptr = cmd;
        sscanf(ptr, "%s ", channel);
        ptr += 37;
        sscanf(ptr, "%s %s %s %s %s %s", bssid, security, signal, mode, ext_ch, net_type);
        ptr = cmd + 4;
        i = space_start = 0;
        while (i < 33) {
            if ((ptr[i] == 0x20) && (i == 0 || ptr[i-1] != 0x20))
                space_start = i;
            i++;
        }
        ptr[space_start] = '\0';
        strcpy(ssid, cmd + 4);
		convert_string_display(ssid);

		SL_DEBUG("upLinkSSID:%s, ssid:%s", upLinkSSID, ssid);
        if (!strcmp(upLinkSSID, ssid)) {
            break;
        }
    }
    pclose(pp);

    int nSignal = atoi(signal);
    if (nSignal < 20) {
        strcpy(upLinkSignal, "3"); //弱
    } else if (nSignal >= 20 && nSignal < 60) {
        strcpy(upLinkSignal, "2"); //中
    } else {
        strcpy(upLinkSignal, "1"); //强
    } 
}


/* 字符串替换操作**/
char * router_strReplace(const char *original, const char *pattern,  const char *replacement) 
{
	//input check
	if (original == NULL || pattern == NULL || replacement == NULL) return NULL;
	
	size_t const replen = strlen(replacement);
	size_t const patlen = strlen(pattern);
	size_t const orilen = strlen(original);

	size_t patcnt = 0;
	const char * oriptr;
	const char * patloc;

	// find how many times the pattern occurs in the original string
	for (oriptr = original; (patloc = strstr(oriptr, pattern)); oriptr = patloc + patlen) {
		patcnt++;
	}

	// allocate memory for the new string
	size_t const retlen = orilen + patcnt * (replen - patlen);
	char * const returned = (char *) malloc( sizeof(char) * (retlen + 1) );

	if (returned != NULL) {
		// copy the original string, 
		// replacing all the instances of the pattern
		char * retptr = returned;
		for (oriptr = original; (patloc = strstr(oriptr, pattern)); oriptr = patloc + patlen)
		{
			size_t const skplen = patloc - oriptr;
			// copy the section until the occurence of the pattern
			strncpy(retptr, oriptr, skplen);
			retptr += skplen;
			// copy the replacement 
			strncpy(retptr, replacement, replen);
			retptr += replen;
		}
		// copy the rest of the string.
		strcpy(retptr, oriptr);
	}
	
	return returned;
}

/** 截去左面的空格 **/
void router_trimLeft(char *s, char c)
{
    int i = 0, j = 0;
	
     //入参检查
    if(s == NULL || !strlen(s)) return;
	
    //从头开始查找字符c, 查找到则移动下标
    while (s[i] == c && s[i] != '\0') i++;
	
    //向左移动数组
    while (s[i] != '\0') s[j++] = s[i++];
	
    s[j] = '\0';
}

/** 截去右边的空格 **/
void router_trimRight(char *s, char c)
{
    int pos;
	
    //入参检查
    if(s == NULL || !strlen(s)) return;
	
    pos = strlen(s) - 1;
	
    //从尾部开始查找字符c
    while(s[pos] == c) {
		
        s[pos--] = '\0';
        if(pos < 0) break;
    }
}

/** 去除左右的空格 **/
void router_trim(char *s, char c)
{
    router_trimLeft(s, c);
    router_trimRight(s, c);
}

void router_firewallSetting(char *opType)
{
	int nType = atoi(opType);
	SL_DEBUG("begin to set firewall, opType[%s]", opType);
	
	switch(nType) {
	case 2: //close
		nvram_set(RT2860_NVRAM, "IPPortFilterEnable", "0");
		break;
	case 1: //open
		nvram_set(RT2860_NVRAM, "IPPortFilterEnable", "1");			
		//set default DefaultFirewallPolicy[accept]
		nvram_set(RT2860_NVRAM, "DefaultFirewallPolicy", "0");
		break;
	}
	
	nvram_commit(RT2860_NVRAM);

	// flush  ipport   filter   chain
    router_iptablesIPPortFilterFlush();

	if (nType == 1) { //open
    	router_iptablesIPPortFilterRun();
	}
	
	return;	
}

int router_getNthValue(int index, char *value, char delimit, char *result, int len)
{
	int i=0, result_len=0;
	char *begin, *end;

	if(!value || !result || !len)
		return -1;

	begin = value;
	end = strchr(begin, delimit);

	while(i<index && end){
		begin = end+1;
		end = strchr(begin, delimit);
		i++;
	}

	//no delimit
	if(!end){
		if(i == index){
			end = begin + strlen(begin);
			result_len = (len-1) < (end-begin) ? (len-1) : (end-begin);
		}else
			return -1;
	}else
		result_len = (len-1) < (end-begin)? (len-1) : (end-begin);

	memcpy(result, begin, result_len );
	*(result+ result_len ) = '\0';

	return 0;
}



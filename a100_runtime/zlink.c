#include "zlink.h"
#include "zlink_utils.h"

int getAutoChannel()
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

void convert_string_display(char *str)
{
	int  len, i;
	char buffer[193];
	char *pOut;

	memset(buffer,0,193);
	len = strlen(str);
	pOut = &buffer[0];

	for (i = 0; i < len; i++) {
		switch (str[i]) {
		case 38:
			strcpy (pOut, "&amp;");		// '&'
			pOut += 5;
			break;

		case 60:
			strcpy (pOut, "&lt;");		// '<'
			pOut += 4;
			break;

		case 62:
			strcpy (pOut, "&gt;");		// '>'
			pOut += 4;
			break;

		case 34:
			strcpy (pOut, "&#34;");		// '"'
			pOut += 5;
			break;

		case 39:
			strcpy (pOut, "&#39;");		// '''
			pOut += 5;
			break;
		case 32:
			strcpy (pOut, "&nbsp;");	// ' '
			pOut += 6;
			break;

		default:
			if ((str[i]>=0) && (str[i]<=31)) {
				//Device Control Characters
				sprintf(pOut, "&#%02d;", str[i]);
				pOut += 5;
			} else if ((str[i]==39) || (str[i]==47) || (str[i]==59) || (str[i]==92)) {
				// ' / ; (backslash)
				sprintf(pOut, "&#%02d;", str[i]);
				pOut += 5;
			} else if (str[i]>=127) {
				//Device Control Characters
				sprintf(pOut, "&#%03d;", str[i]);
				pOut += 6;
			} else {
				*pOut = str[i];
				pOut++;
			}
			break;
		}
	}
	*pOut = '\0';
	strcpy(str, buffer);
}

void zlink_getWifiChannel(char *wifiChannel, size_t len)
{
	const char *channel_s;
	int channel =0;
	ZDEBUG("begin to get wifi channel.");
	channel_s = nvram_bufget(RT2860_NVRAM, "Channel");
	channel = (channel_s == NULL) ? 1 : strtoul(channel_s, NULL, 10);

	//Auto channel.
	if (0 == channel)
	{
		channel = getAutoChannel();
	}

	//channel value check
	if (channel > 13 || channel < 0) {		
		ZDEBUG("get wifi channel invalid : value : %d.", channel);
		channel = 1; //default value
	}
	
	snprintf(wifiChannel, len, "%d", channel);
	
	ZDEBUG("get wifi channel value: %s.", wifiChannel);
	return;
}

int zlink_getCfgValue(const char *cmd, char *cfgValue, size_t len)
{
    FILE* fp; 
    if((fp = popen(cmd, "r")) == NULL) {
        printf("nGetRouterCfg:popen failed.");
        return -1;
    }
	
	fgets(cfgValue, len, fp);
	
	pclose(fp);
	return 0;
}

char *zlink_getIfname(void)
{
	const char *opMode = NULL;
	static char *ifname = "br0";
	opMode = nvram_get(RT2860_NVRAM, "OperationMode");
	const char *vlanEnble, *vlanID;
	vlanEnble = nvram_bufget(RT2860_NVRAM, "VLANEnable");
	vlanID = nvram_bufget(RT2860_NVRAM, "VLANID");
	
	if (strlen(opMode))
	{
		if (!strcmp(opMode, "0")) //bridge
			ifname = "br0";
		else if (!strcmp(opMode, "1")) //gateway. do something, get UPLINK TYPE
		{
			if(0==strncmp(vlanEnble,"1",2))
			{
				static char if_name_buff[64] = {0};
				sprintf(if_name_buff,"eth2.2.%s",vlanID);
				ifname = if_name_buff;
			}
			else
			{
				ifname = "eth2.2";
			}
		}
		else if (!strcmp(opMode, "3")) //AP client
			ifname = "apcli0";
		else
			ifname = "br0";
	}

	return ifname;
}

char *zlink_getIpaddr(char *ifname)
{
	struct ifreq ifr;
	int skfd = 0;
	static char if_addr[16];

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr, "%s: open socket error\n", __func__);
		return "";
	}

	strncpy(ifr.ifr_name, ifname, IF_NAMESIZE);
	if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0) {
		close(skfd);
		fprintf(stderr, "%s: ioctl SIOCGIFADDR error for %s\n", __func__, ifname);
		return "";
	}
	strcpy(if_addr, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	close(skfd);

	return if_addr;
}

void zlink_getUpAndDownLinkRate(char* upLinkRate, char *downLinkRate)
{
	char buf[1024];
	
	FILE* fp=NULL;
	int upLinkSpeed = 0;
	int downLinkSpeed = 0;
	if(access("/tmp/speed.txt",0)==0)
	{
		fp=fopen("/tmp/speed.txt", "r");
		{
			while(fgets(buf,sizeof(buf),fp))
		  	{
				sscanf(buf,"%d,%d", &upLinkSpeed, &downLinkSpeed);
				if (upLinkSpeed == 0) upLinkSpeed = 130 * 1024;
				if (downLinkSpeed == 0) downLinkSpeed = 65 * 1024;
				
				sprintf(upLinkRate,"%d", upLinkSpeed/1024);
				sprintf(downLinkRate,"%d", downLinkSpeed/1024);
				ZDEBUG("upLinkRate=%s,downLinkRate=%s\n",upLinkRate,downLinkRate);
		  	}
			fclose(fp);
	  	}	  
	}

	return;
}

void zlink_getUpLinkSignal(const char* upLinkSSID, char *upLinkSignal)
{
	if (upLinkSSID == NULL || strlen(upLinkSSID) == 0) {
		strcpy(upLinkSignal, "-55");
        return;
	}

    FILE *pp;
    char cmd[256], *ptr;
    char channel[4], ssid[186], bssid[20], security[23];
    char signal[9], mode[7], ext_ch[7], net_type[3];
    int i, space_start;

	memset(signal, 0, sizeof(signal));
    memset(cmd, 0, sizeof(cmd));
    zlink_doSystem("iwpriv ra0 set SiteSurvey=1");
    strcpy(cmd, "iwpriv ra0 get_site_survey");
    if(!(pp = popen(cmd, "r"))) {
        ZDEBUG("execute get_site_survey fail!");
        strcpy(upLinkSignal, "-55");
        return;
    }

    memset(cmd, 0, sizeof(cmd));
    fgets(cmd, sizeof(cmd), pp);
    fgets(cmd, sizeof(cmd), pp);
    while (fgets(cmd, sizeof(cmd), pp)) {		
        if (strlen(cmd) < 4) {
			ZDEBUG("cmd exception :%s.", cmd);
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

		ZDEBUG("upLinkSSID:%s, ssid:%s", upLinkSSID, ssid);
        if (!strcmp(upLinkSSID, ssid)) {
            break;
        }
    }
    pclose(pp);

    int nSignal = atoi(signal);
    if (nSignal > 0) {
        strcpy(upLinkSignal, signal);
    } else {
        strcpy(upLinkSignal, "-55");
    }

	return;
	
}

int zlink_getAPConnectDevices(struct RouterClientList *prClientList)
{
	int i, j, fd;
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
		return -1;
	}

	if (ioctl(fd, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &iwr) < 0) {
		close(fd);
		return -1;
	}
	close(fd); 

	for (i = 0; i < table.Num; i++) {
			
		RT_802_11_MAC_ENTRY *pe = &(table.Entry[i]);
		
		prClientNode = malloc(sizeof(recRouterClient));
		if (prClientNode == NULL) {
			ZDEBUG("memory is not enough.");
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

		//2. onlineTime
		sprintf(prClientNode->onlineDuration, "%d", pe->ConnectedTime);
		
		//3. RxRate, TxRate
		strcpy(prClientNode->clientDownlinkRate, "0.0");
		strcpy(prClientNode->clientUplinkRate, "0.0");

		//4. RSSI
        sprintf(prClientNode->RSSI, "%d", (int)(pe->AvgRssi0));

		//add one node to list 
		SLIST_INSERT_HEAD(prClientList, prClientNode, next);
	}
	
	return 0;
}


int zlink_getConnectDevices(struct RouterClientList *prClientList)
{
	FILE *fp;
	struct dhcpOfferedAddr {
		unsigned char hostname[16];
		unsigned char mac[16];
		unsigned long ip;
		unsigned long expires;
	} lease;

	int i, j, fd;
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
		printf("open socket fail!");
		return -1;
	}

	if (ioctl(fd, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &iwr) < 0) {
		printf("ioctl -> RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT fail!");
		close(fd);
		return -1;
	}
	close(fd);

	//Get DHCP Users
	zlink_doSystem("killall -q -USR1 udhcpd");
	fp = fopen("/var/udhcpd.leases", "r");
	if (NULL == fp) {
		printf("dhcp file not exists.");
		return -1;
	}
	while (fread(&lease, 1, sizeof(lease), fp) == sizeof(lease)) {

		prClientNode = malloc(sizeof(recRouterClient));
		if (prClientNode == NULL) {
			printf("memory is not enough.");
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

		for (i = 0; i < table.Num; i++) {
			RT_802_11_MAC_ENTRY *pe = &(table.Entry[i]);	 
			//mac compare
			if (!strncmp(pe->Addr, lease.mac, 5)) {
				//4. onlineTime
				sprintf(prClientNode->onlineDuration, "%d", pe->ConnectedTime);
				
				//5. RSSI
        		sprintf(prClientNode->RSSI, "%d", (int)(pe->AvgRssi0));	
				break;
			}
		}
		
		//add one node to list
		SLIST_INSERT_HEAD(prClientList, prClientNode, next);

	}
	
	fclose(fp);
	return 0;
}

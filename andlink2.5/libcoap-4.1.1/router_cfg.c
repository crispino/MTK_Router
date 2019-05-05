#include "router_cfg.h"
#include <signal.h>
#include <sys/time.h>
#include "debug.h"

#define LED_BLINK_INTERVAL	(200*1000)		//microsec


void wifi_power(char *radio, char *power)
{
	int nvram_id;

	if (!strcmp(radio, "2.4G"))
	{
		nvram_id = RT2860_NVRAM;
	}
#if defined (CONFIG_PRODUCT_341AA)
	else if (!strcmp(radio, "5G"))
	{
		nvram_id = RTDEV_NVRAM;

	}
#endif

	nvram_bufset(nvram_id, "TxPower", power);			
	nvram_commit(nvram_id);

	return ;
}

void open_wifi(char *radio)
{
	int nvram_id;
	char ifname[8] = {0};

	if (!strcmp(radio, "2.4G"))
	{
		nvram_id = RT2860_NVRAM;
		strcpy(ifname, "ra0");
	}
#if defined (CONFIG_PRODUCT_341AA)
	else if (!strcmp(radio, "5G"))
	{
		nvram_id = RTDEV_NVRAM;
		strcpy(ifname, "rai0");

	}
#endif

	nvram_bufset(nvram_id, "WiFiOff", "0");
	nvram_commit(nvram_id);
	do_system("ifconfig %s up", ifname);

	return ;
}

void close_wifi(char *radio)
{
	int nvram_id;
	char ifname[8] = {0};

	if (!strcmp(radio, "2.4G"))
	{
		nvram_id = RT2860_NVRAM;
		strcpy(ifname, "ra0");
	}
#if defined (CONFIG_PRODUCT_341AA)
	else if (!strcmp(radio, "5G"))
	{
		nvram_id = RTDEV_NVRAM;
		strcpy(ifname, "rai0");

	}
#endif

	nvram_bufset(nvram_id, "WiFiOff", "1");
	nvram_commit(nvram_id);
	do_system("ifconfig %s down", ifname);

	return ;
}


/*
 * len < Max length
 * return value: 1 success, 0 failed
 */
#define BUF_MAX_LEN		32
int get_popen_str(char *cmd, char *str, char *out, int len)
{
	int ret = 0;
	char buf[BUF_MAX_LEN] = {0};
	char *ptr = NULL;

	FILE *pp = popen(cmd, "r");
	if (pp)
	{
		while(fgets(buf, sizeof(buf), pp))
		{
			if (strlen(buf))
			{
				ptr = strstr(buf, str);
				if (ptr)
				{
					ptr += strlen(str);
					memcpy(out, ptr, len);
					out[len] = '\0';
					ret = 1;
				}
			}
		}

		pclose(pp);
	}

	//coap_printf("ret: %d", ret);
	return ret;
}


void get_split_value(char *record, char *split, int index, char *result)
{
    int nPos = 0;
    char *pTmp = NULL;
    char *pTok= NULL;
    pTmp = record;
    pTok = strtok(pTmp, split);
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


int getPidByName(char *proc_name)
{
	char cmd_buf[128] = {0};
	char buf[64] = {0};

	int pid = -1;

	snprintf(cmd_buf, sizeof(cmd_buf), "ps -ef | grep %s | grep -v grep | grep -v ps", proc_name);

	FILE *pp = popen(cmd_buf, "r");
	if (pp)
	{
		while(fgets(buf, sizeof(buf), pp))
		{
			if (0 != strlen(buf) && strstr(buf, proc_name))
			{
				/* 3701 admin	  1720 S	ont_cloud */
				pid = atoi(buf);
				break;
			}
		}

		pclose(pp);
	}

	return pid;
}

char *set_nth_value(int index, char *old_values, char *new_value)
{
	int i;
	char *p, *q;
	static char ret[2048];
	char buf[8][256];

	memset(ret, 0, 2048);
	for (i = 0; i < 8; i++)
		memset(buf[i], 0, 256);

	//copy original values
	for ( i = 0, p = old_values, q = strchr(p, ';')  ;
			i < 8 && q != NULL                         ;
			i++, p = q + 1, q = strchr(p, ';')         )
	{
		strncpy(buf[i], p, q - p);
	}
	strcpy(buf[i], p); //the last one

	//replace buf[index] with new_value
	strncpy(buf[index], new_value, 256);

	//calculate maximum index
	index = (i > index)? i : index;

	//concatenate into a single string delimited by semicolons
	strcat(ret, buf[0]);
	for (i = 1; i <= index; i++) {
		strncat(ret, ";", 2);
		strncat(ret, buf[i], 256);
	}

	return ret;
}

void set_nth_value_flash(int nvram, int index, char *flash_key, char *value)
{
	char *result;
	char *tmp = (char *) nvram_bufget(nvram, flash_key);
	if(!tmp)
		tmp = "";
	result = set_nth_value(index, tmp, value);
	nvram_bufset(nvram, flash_key, result);
}


char *strip_space(char *str)
{
	while( *str == ' ')
		str++;
	return str;
}

/*
 * desc: 获取设备在线时长
 *
 * return: 在线时长，单位 秒。
 */
unsigned int GetUptime(void)
{
	struct tm *utime;
	time_t usecs;

	time(&usecs);
	utime = localtime(&usecs);

	if (utime->tm_hour > 0)
		return utime->tm_hour*60*60 + utime->tm_min*60 + utime->tm_sec;
	else if (utime->tm_min > 0)
		return utime->tm_min*60 + utime->tm_sec;
	else
		return utime->tm_sec;
}

char *get_macaddr(char *ifname)
{
	struct ifreq ifr;
	char *ptr;
	int skfd;
	static char if_hw[18] = {0};

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr, "%s: open socket error\n", __func__);
		return "";
	}
	strncpy(ifr.ifr_name, ifname, 16);
	if(ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0) {
		close(skfd);
		fprintf(stderr, "%s: ioctl fail\n", __func__);
		return "";
	}

	ptr = (char *)&ifr.ifr_addr.sa_data;
	sprintf(if_hw, "%02X%02X%02X%02X%02X%02X",
			(ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
			(ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377));
	close(skfd);

	return if_hw;
}

char *get_macaddr_colon(char *ifname)
{
	struct ifreq ifr;
	char *ptr;
	int skfd;
	static char if_hw[18] = {0};

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr, "%s: open socket error\n", __func__);
		return "";
	}
	strncpy(ifr.ifr_name, ifname, 16);
	if(ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0) {
		close(skfd);
		fprintf(stderr, "%s: ioctl fail\n", __func__);
		return "";
	}

	ptr = (char *)&ifr.ifr_addr.sa_data;
	sprintf(if_hw, "%02X:%02X:%02X:%02X:%02X:%02X",
			(ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
			(ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377));
	close(skfd);

	return if_hw;
}

char *get_ipaddr(char *ifname)
{
	struct ifreq ifr;
	int skfd = 0;
	static char if_addr[16];

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr, "%s: open socket error\n", __func__);
		return "";
	}

	strncpy(ifr.ifr_name, ifname, 16);
	if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0) {
		close(skfd);
		fprintf(stderr, "%s: ioctl SIOCGIFADDR error for %s\n", __func__, ifname);
		return "";
	}
	strcpy(if_addr, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	close(skfd);

	return if_addr;
}

char *get_boardcast_ipaddr(char *ifname)
{
	struct ifreq ifr;
	int skfd = 0;
	static char if_addr[16];

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr, "%s: open socket error\n", __func__);
		return "";
	}

	strncpy(ifr.ifr_name, ifname, 16);
	if (ioctl(skfd, SIOCGIFBRDADDR, &ifr) < 0) {
		close(skfd);
		fprintf(stderr, "%s: ioctl SIOCGIFBRDADDR error for %s\n", __func__, ifname);
		return "";
	}
	strcpy(if_addr, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	close(skfd);

	return if_addr;
}

char *get_ifname(void)
{
	char *opMode = NULL;
	static char *ifname = "br0";
	opMode = nvram_get(RT2860_NVRAM, "OperationMode");
	
	if (strlen(opMode))
	{
		if (!strcmp(opMode, "0")) //bridge
			ifname = "br0";
		else if (!strcmp(opMode, "1")) //gateway. do something, get UPLINK TYPE
			ifname = "eth2.2";
		else if (!strcmp(opMode, "3"))
			ifname = "apcli0";
		else
			ifname = "br0";
	}
	
	coap_printf("ifname: %s", ifname);

	return ifname;
}

time_t router_getCompileTimeStamp()
{
	char sTip[8] = {0};
	char sDay[4] = {0};
	char sWeek[8] = {0};
	char sMonth[8] = {0};
	char sYear[8] = {0};
	char sDateTime[16] = {0};
	char sDateFormat[8] = {0};
	char sData[128] = {0};
	char sBuf[32] = {0};
	struct tm tm;
	time_t t_compileTime = time(NULL);

	router_getCfgValue("cat /proc/version",  sData, sizeof(sData) -1);
	char *sPos = strrchr(sData, '#');	
	if (sPos != NULL && strstr(sPos, "CST") != NULL) {		
		sscanf(sPos, "%s %s %s %s %s %s %s", sTip, sWeek, sMonth, sDay, sDateTime, sDateFormat, sYear);
		sprintf(sBuf, "%s, %s %s %s %s GMT", sWeek, sDay, sMonth, sYear, sDateTime);
		if (strptime(sBuf, "%a, %d %b %Y %H:%M:%S GMT", &tm) != NULL) {
			t_compileTime = mktime(&tm);
		}
	}

	return t_compileTime;
}


void do_system(char *fmt, ...)
{
    char cmd[256] = {0};
	va_list ap;
	va_start(ap, fmt);
	vsprintf(cmd, fmt, ap);
	va_end(ap);
	sprintf(cmd, "%s 1>%s 2>&1", cmd, ANDLINK_LOGFILE);
    DEBUG("system command [%s]", cmd);
	system(cmd);

	return;
}

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

void router_getWifiChannel(char *wifiChannel, size_t len)
{
	const char *channel_s;
	int channel =0;
	DEBUG("begin to get wifi channel.");
	channel_s = nvram_bufget(RT2860_NVRAM, "Channel");
	channel = (channel_s == NULL) ? 1 : strtoul(channel_s, NULL, 10);

	//Auto channel.
	if (0 == channel)
	{
		channel = getAutoChannel();
	}

	//channel value check
	if (channel > 13 || channel < 0) {		
		DEBUG("get wifi channel invalid : value : %d.", channel);
		channel = 1; //default value
	}
	
	snprintf(wifiChannel, len, "%d", channel);
	
	DEBUG("get wifi channel value: %s.", wifiChannel);
	return;
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

int router_getCfgValue(const char *cmd, char *cfgValue, size_t len)
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

char *router_getIfname(void)
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


char *router_getIpaddr(char *ifname)
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


void router_getUpAndDownLinkRate(char* upLinkRate, char *downLinkRate)
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
				
				sprintf(upLinkRate,"%.6f", (float)upLinkSpeed*8/1024/1024);
				sprintf(downLinkRate,"%.6f", (float)downLinkSpeed*8/1024/1024);
				DEBUG("upLinkRate=%s,downLinkRate=%s\n",upLinkRate,downLinkRate);
		  	}
			fclose(fp);
	  	}	  
	}

	return;
}

void router_getUpLinkSignal(const char* upLinkSSID, char *upLinkSignal)
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
    do_system("iwpriv ra0 set SiteSurvey=1");
    strcpy(cmd, "iwpriv ra0 get_site_survey");
    if(!(pp = popen(cmd, "r"))) {
        DEBUG("execute get_site_survey fail!");
        strcpy(upLinkSignal, "-55");
        return;
    }

    memset(cmd, 0, sizeof(cmd));
    fgets(cmd, sizeof(cmd), pp);
    fgets(cmd, sizeof(cmd), pp);
    while (fgets(cmd, sizeof(cmd), pp)) {		
        if (strlen(cmd) < 4) {
			DEBUG("cmd exception :%s.", cmd);
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

		DEBUG("upLinkSSID:%s, ssid:%s", upLinkSSID, ssid);
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

int router_getAPConnectDevices(struct RouterClientList *prClientList)
{
	int i, j, k, fd;
	recRouterClient *prClientNode;
	char *pTemp;
	struct in_addr addr;
	struct iwreq iwr;
	unsigned long nTemp = 0;
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

	char sWirelessClients[4096]= {0};
	char sStatClients[4096]= {0};
	char rec[128] = {0};
	char cli_mac[32] = {0};
	char cli_ip[16] = {0};
	char hostname[64] = {0};
	char RxRate[12] = {0};
	char TxRate[12] = {0};
	router_getCfgValue(ROUTER_GET_WIRELESS_CLIENTS, sWirelessClients, sizeof(sWirelessClients));
	router_getCfgValue(ROUTER_GET_STAT_CLIENTS, sStatClients, sizeof(sStatClients));
	//printf("router_getAPConnectDevices sStatClients: %s\n", sStatClients);

	for (i = 0; i < table.Num; i++) {
			
		RT_802_11_MAC_ENTRY *pe = &(table.Entry[i]);
		
		prClientNode = malloc(sizeof(recRouterClient));
		if (prClientNode == NULL) {
			DEBUG("memory is not enough.");
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

		//3. RSSI
        sprintf(prClientNode->RSSI, "%d", (int)(pe->AvgRssi0));
		
		strcpy(prClientNode->clientDownlinkRate,  "0.00");
		strcpy(prClientNode->clientUplinkRate,  "0.00");

		k = 0;
		/* 7C:04:D0:6E:27:58,192.168.1.15,iPhone; */
		while((router_getNthValue(k++, sWirelessClients, ';', rec, sizeof(rec)) != -1) && strlen(rec)) {
			if((router_getNthValue(0, rec, ',', cli_mac, sizeof(cli_mac)) == -1)){
				continue;
			}
			
			if (!strcmp(prClientNode->macAddr, cli_mac)) {
				//4. ipAddress
				if((router_getNthValue(1, rec, ',', cli_ip, sizeof(cli_ip)) == -1)){
					continue;
				}
				strcpy(prClientNode->ipAddr, cli_ip);

				//5. hostName
				if((router_getNthValue(2, rec, ',', hostname, sizeof(hostname)) == -1)){
					continue;
				}
				strcpy(prClientNode->name, hostname);
				
				break;
			}
		}

		k= 0;
	    /* IPAddress,in,ou;IPAddress,in,ou; */
		if (strlen(prClientNode->ipAddr) >0) {
			while((router_getNthValue(k++, sStatClients, ';', rec, sizeof(rec)) != -1) && strlen(rec)) {
				if((router_getNthValue(0, rec, ',', cli_ip, sizeof(cli_ip)) == -1)){
					continue;
				}
				
				if (!strcmp(prClientNode->ipAddr, cli_ip)) {
					//6. RxRate
					if((router_getNthValue(1, rec, ',', RxRate, sizeof(RxRate)) == -1)){
						continue;
					}
					nTemp = strtoul(RxRate, NULL, 10);
					/* Mbits/s */
					sprintf(prClientNode->clientDownlinkRate,  "%.6f", (float)nTemp*8/(1024.00 * 1024.00));

					//7. TxRate
					if((router_getNthValue(2, rec, ',', TxRate, sizeof(TxRate)) == -1)){
						continue;
					}
					nTemp = strtoul(TxRate, NULL, 10);
					/* Mbits/s */
					sprintf(prClientNode->clientUplinkRate,  "%.6f", (float)nTemp*8/(1024.00 * 1024.00));
					
					break;
				}
			}
		}

		//add one node to list 
		SLIST_INSERT_HEAD(prClientList, prClientNode, next);
	}
	
	return 0;
}


int router_getConnectDevices(struct RouterClientList *prClientList)
{
	FILE *fp;
	struct dhcpOfferedAddr {
		unsigned char hostname[16];
		unsigned char mac[16];
		unsigned long ip;
		unsigned long expires;
	} lease;

	int i, j, k, fd;
	recRouterClient *prClientNode;
	char *pTemp;
	struct in_addr addr;
	struct iwreq iwr;
	RT_802_11_MAC_TABLE table = {0};
	char sStatClients[4096]= {0};
	char rec[128] = {0};
	char cli_ip[16] = {0};
	char RxRate[12] = {0};
	char TxRate[12] = {0};
	unsigned long nTemp = 0;
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
	do_system("killall -q -USR1 udhcpd");
	fp = fopen("/var/udhcpd.leases", "r");
	if (NULL == fp) {
		printf("dhcp file not exists.");
		return -1;
	}

	router_getCfgValue(ROUTER_GET_STAT_CLIENTS, sStatClients, sizeof(sStatClients));
	//printf("router_getConnectDevices sStatClients: %s\n", sStatClients);
	
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

		strcpy(prClientNode->clientDownlinkRate,  "0.00");
		strcpy(prClientNode->clientUplinkRate,  "0.00");

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

		k= 0;
	    /* IPAddress,in,ou;IPAddress,in,ou; */
		if (strlen(prClientNode->ipAddr) >0) {
			while((router_getNthValue(k++, sStatClients, ';', rec, sizeof(rec)) != -1) && strlen(rec)) {
				if((router_getNthValue(0, rec, ',', cli_ip, sizeof(cli_ip)) == -1)){
					continue;
				}
				
				if (!strcmp(prClientNode->ipAddr, cli_ip)) {
					//6. RxRate
					if((router_getNthValue(1, rec, ',', RxRate, sizeof(RxRate)) == -1)){
						continue;
					}
					nTemp = strtoul(RxRate, NULL, 10);
					/* Mbits/s */
					sprintf(prClientNode->clientDownlinkRate,  "%.6f", (float)nTemp*8/(1024.00 * 1024.00));

					//7. TxRate
					if((router_getNthValue(2, rec, ',', TxRate, sizeof(TxRate)) == -1)){
						continue;
					}
					nTemp = strtoul(TxRate, NULL, 10);
					/* Mbits/s */
					sprintf(prClientNode->clientUplinkRate,  "%.6f", (float)nTemp*8/(1024.00 * 1024.00));
					
					break;
				}
			}
		}
		
		//add one node to list
		SLIST_INSERT_HEAD(prClientList, prClientNode, next);

	}
	
	fclose(fp);
	return 0;
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

	return ;
}



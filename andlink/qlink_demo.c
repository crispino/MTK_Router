#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/wireless.h>

#include "libcoap-4.1.1/debug.h"
#include "Qlink_API.h"
#include "nvram.h"

#define GUIDE_SSID			"CMCC-QLINK"
#define GUIDE_ENCRYPT		"OPEN"
#define GUIDE_TYPE			"AES"
#define GUIDE_PASSWD		""

#define BRIDGE_MODE		0
#define ROUTER_MODE		1
#define REPEAT_MODE		3

//设备的ID
char *g_deviceId = NULL;
char *g_dev_ip = NULL;
char *g_dev_boardcast_ip = NULL;
char *g_dev_mac = NULL;
char *g_dev_up_mac = NULL;
char *g_dev_type = NULL;
int andlink_debug = 0;

char gwip[32] = {0};
int  g_repeated = 0;
volatile int g_server_started = 0;
volatile int g_found_repeater = 0;
#define LED_BLINK_INTERVAL	(200*1000)		//microsec



void waitUserConfigure()
{
	//Default Configure
	char *apCliEn = nvram_get(RT2860_NVRAM, "ApCliEnable");
	char *apSSId =	nvram_get(RT2860_NVRAM, "ApCliSsid");
	if (strncmp(apCliEn, "1", 1) || strlen(apSSId) == 0) {
		coap_printf("Default configure, need to Connect CMCC_QLINK.");
		return;
	}
	
	//User Configure, Max waitTime: 2 Minutes for Repeater uplink router
	char *connectStatus = NULL;
	int startTime = time(NULL);
	int currentTime = time(NULL);
	while (currentTime - startTime < 120) {
		connectStatus = nvram_get(RT2860_NVRAM, "apcliConnStatus");
		currentTime = time(NULL);
		if (!strcmp(connectStatus, "Connected")) {
			coap_printf("User configure, apcliConnStatus[Connected].");
			g_repeated = 1;
			break;
		}
		
		sleep(2);
	}
	
	return;

}

void ledChecker()
{
	int flag = 1;
	while (!g_found_repeater) {
		if (flag) {
			flag = 0;
			system("gpio 2");
		} else {
			flag = 1;
			system("gpio a");
		}
		usleep(LED_BLINK_INTERVAL);
	}

	return;
}


void startLedThread()
{
	pthread_t id;
	int ret = 0;
    ret = pthread_create(&id, NULL, ledChecker, NULL);
    if(ret != 0){
        coap_printf("create led thread error!\n");
        exit(1);
    }
	
	pthread_detach(id);
}

void repeaterChecker()
{
	int  connected = 0;
	char *connectStatus = NULL;
	int startTime = time(NULL);
	int currentTime = time(NULL);

	//Timeout: 2 Minutes
	while (currentTime - startTime < 120) {
		connectStatus = nvram_get(RT2860_NVRAM, "Q_LINK_OK");
		currentTime = time(NULL);
		if (!strcmp(connectStatus, "1")) {
			connected = 1;
			break;
		}
		
		sleep(2);
	}
	
	//Timeout: Close ApClientEnable
	if (!connected) {
		coap_printf("iwpriv apcli0 set ApCliEnable=0!");
		do_system("iwpriv apcli0 set ApCliEnable=0");
	}

	return;	
}

void startReapterThread()
{
	pthread_t id;
	int ret = 0;
    ret = pthread_create(&id, NULL, repeaterChecker, NULL);
    if(ret != 0){
        coap_printf("create repeater thread error!\n");
        exit(1);
    }
	
	pthread_detach(id);
}

void *StartThread(void *argc)
{    // 启动qlinkserver
    Qlink_StartCoapServer();
    return ((void *)0);
}

int startCoapServer()
{
	if (g_server_started) return;

	int ret;
    pthread_t id;
   
    ret = pthread_create(&id,NULL,StartThread,NULL);
    if(ret != 0) {
        coap_printf("create thread error!\n");
        exit(1);
    }

	coap_printf("coap server started!");
	sleep(2);
    pthread_detach(pthread_self());
	g_server_started = 1;
}

char *get_macaddr(char *ifname)
{
	struct ifreq ifr;
	char *ptr;
	int skfd;
	static char if_hw[18] = {0};

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr, "%s: open socket error\n", __func__);
		return NULL;
	}
	strncpy(ifr.ifr_name, ifname, 16);
	if(ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0) {
		close(skfd);
		fprintf(stderr, "%s: ioctl fail\n", __func__);
		return NULL;
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
		return NULL;
	}
	strncpy(ifr.ifr_name, ifname, 16);
	if(ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0) {
		close(skfd);
		fprintf(stderr, "%s: ioctl fail\n", __func__);
		return NULL;
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
	char *cm= nvram_bufget(RT2860_NVRAM, "wanConnectionMode");
	if (!strncmp(cm, "PPPOE", 6) || !strncmp(cm, "L2TP", 5) || !strncmp(cm, "PPTP", 5)
#ifdef CONFIG_USER_3G
			|| !strncmp(cm, "3G", 3)
#endif
		) {
		ifname = "ppp0";
	}
	else 
	{
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
	}
	
	coap_printf("ifname: %s", ifname);

	return ifname;
}

/*
 * return value: 1 success, 0 failed
 *
 */
static int init(void)
{
	int ret = 0;

	if (NULL == g_deviceId)
		g_deviceId = "defaultId";

	char *ifname = NULL;
	ifname = get_ifname();

	andlink_debug = atoi(nvram_get(RT2860_NVRAM, "andlink_debug"));
	//g_dev_ip = get_ipaddr(ifname);
	g_dev_ip = get_ipaddr("br0");
	g_dev_boardcast_ip = get_boardcast_ipaddr("br0");
	g_dev_mac = get_macaddr(ifname);
	g_dev_up_mac = get_macaddr_colon(ifname);
	g_dev_type = nvram_get(RT2860_NVRAM, "andlink_deviceType");
	coap_printf("g_dev_lan_ip: %s, g_dev_boardcast_ip: %s, g_dev_mac: %s, g_dev_type: %s, andlink_token: %s", 
			g_dev_ip, g_dev_boardcast_ip, g_dev_mac, g_dev_type, nvram_get(RT2860_NVRAM, "andlink_andlinkToken"));

	if (NULL == g_dev_type || 0 == strlen(g_dev_type))
		coap_printf("No dev type!");
	else
		ret = 1;

	coap_printf("ret: %d", ret);
	return ret;
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

/*
 * return value: 1 success, 0 failed
 *
 */
void init_network(void)
{
	int ret = 0;
	while(!ret)
	{
		/* do something. init network */
		if (get_popen_str("cat /etc/resolv.conf", "nameserver ", gwip, sizeof(gwip)))
		{
			//coap_printf("GWIP: %s", gwip);
			/* 不等于默认地址，则视为从上级获取的地址 */
			if (strncmp("8.8.8.8", gwip, strlen("8.8.8.8")))
			{
				ret = 1;
				g_found_repeater = 1;
			}
		}

		sleep(3);
		coap_printf("WGIP: %s, ret: %d", gwip, ret);		
	}

	return ;
}

void start_process(void)
{
	int ret = 0;

	/* do something. searchGW */

	if (!init())
		return ;

	g_deviceId = nvram_get(RT2860_NVRAM, "andlink_deviceId");
	int mode = BRIDGE_MODE;
	char *q_link_ok = NULL;

	mode = atoi(nvram_get(RT2860_NVRAM, "OperationMode"));
	q_link_ok = nvram_get(RT2860_NVRAM, "Q_LINK_OK");

	/* 中继产品/模式，未成功快连上级时，开始快连功能。 */
	if (REPEAT_MODE == mode && 
			(q_link_ok == NULL || 0 == strlen(q_link_ok) || 0 != strcmp(q_link_ok, "1")))
	{
		//检测用户的配置信息
		waitUserConfigure();
	
		/* Q-LINK 1: 连接引导SSID CMCC-QLINK */
		if (!g_repeated) {
			quick_repeater(GUIDE_SSID, GUIDE_ENCRYPT, GUIDE_TYPE, GUIDE_PASSWD);
			
			startLedThread();

			//检测repeater是否连上上级网关
			startReapterThread();
		}
		
		/* 初始化网关地址 */
		init_network();

		/* 开启Coap Server服务 */
		startCoapServer();

		/* Q-LINK 2: 发送入网请求 */
		qlink_request(g_deviceId, gwip);

		//wait for qlink ok
		while (strcmp(q_link_ok, "1")) {
			q_link_ok = nvram_get(RT2860_NVRAM, "Q_LINK_OK");
			sleep(2);
		}
		
	}
	else
	{
		/* 初始化网关地址 */
		init_network();
	}

	/* 开启Coap Server服务 */
	startCoapServer();

	while (NULL == g_deviceId || 0 == strlen(g_deviceId))
	{
		coap_printf("No g_deviceId, need bootstrap");
		/* 0:成功，-1:失败 */
		ret = device_inform_regist(gwip);
		coap_printf("regist ret: %d", ret);
		g_deviceId = nvram_get(RT2860_NVRAM, "andlink_deviceId");

		sleep(2);
	}

	/* 只要不是数据发送失败就继续 */
	if (AL_ERROR != ret)
	{
		ret = device_inform_online(g_deviceId, gwip);
		if (ret != 0) {
			coap_printf("device online error!\n");
			if (ret == AUTH_FAILED) {
				nvram_bufset(RT2860_NVRAM, "andlink_deviceId", "");
				nvram_commit(RT2860_NVRAM);
				memset(gwip, 0, sizeof(gwip));
				g_deviceId = nvram_get(RT2860_NVRAM, "andlink_deviceId");
			}
			return;
		}

		coap_printf("online ret: %d", ret);
		if (AL_ERROR != ret)
		{
			char *h_period = NULL;
			int t_period = 0;
			h_period = nvram_get(RT2860_NVRAM, "andlink_heartbeatTime");
			if (NULL == h_period || strlen(h_period) == 0)
				t_period = 30000;
			else
				t_period = atoi(h_period);

			coap_printf("Heart_beat period: %s", h_period);
			while(1)
			{
				ret = device_inform_heartbeat(g_deviceId, gwip);
				coap_printf("heartbeat ret: %d", ret);
				if (ret == AUTH_FAILED) {
					nvram_bufset(RT2860_NVRAM, "andlink_deviceId", "");
					nvram_commit(RT2860_NVRAM);
					memset(gwip, 0, sizeof(gwip));
					g_deviceId = nvram_get(RT2860_NVRAM, "andlink_deviceId");
					return;
				}
				usleep(1000 * (t_period));
			}
		}
	}

    //关闭CoapServer
    pthread_exit(NULL);

	return ;
}

int main(int argc, char* argv[])
{
	while (1)
	{
		start_process();

		sleep(5);
	}

	return 0;
}


#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/wireless.h>
#include "libcoap-4.1.1/debug.h"
#include "libcoap-4.1.1/router_cfg.h"
#include "libcoap-4.1.1/andlink_tcp_handler.h"
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
char gwip[32] = {0};
int  g_repeated = 0;
volatile int g_server_started = 0;
volatile int g_found_repeater = 0;
#define LED_BLINK_INTERVAL	(200*1000)		//microsec


volatile int g_coapExit = 1;
volatile int g_tcpExit = 1;
volatile int g_udpExit = 1;

int searchGateWay()
{
	int nProtocol, nRet;
	nProtocol = CLOUD_GATEWAY;
	int startTime = time(NULL);
	int currentTime = time(NULL);

	//超时时间: 30秒
	while (currentTime - startTime < 30) {
		/* 0:成功，-1:失败 */
		nRet = device_inform_regist(gwip);		
		coap_printf("home gateway regist ret: %d", nRet);
		g_deviceId = nvram_get(RT2860_NVRAM, "andlink_deviceId");
		if (strlen(g_deviceId) > 0) {
			nProtocol = HOME_GATEWAY;
			break;
		}

		currentTime = time(NULL);
		sleep(2);
	}

	return nProtocol;
}

void *vTcpClientCallback(void* args)
{
	recProtoClient rTcpClient;
	while(1) {
		memset(&rTcpClient, 0, sizeof(recProtoClient));
		if (g_tcpExit) nCreateTcpClient(&rTcpClient, vTcpMessageHandler);
		coap_printf("tcp client try again!");
		sleep(5);
	}

	return ((void *)0);
}

void startTcpClient(pthread_t *pTcpThread)
{
	pthread_create(pTcpThread, NULL, vTcpClientCallback, NULL);
	return;
}

void *vUdpClientCallback(void *args)
{
	recProtoClient rUdpClient;
	while(1) {
		memset(&rUdpClient, 0, sizeof(recProtoClient));
		if (g_udpExit) nCreateUdpClient(&rUdpClient, vUdpMessageHandler);
		coap_printf("udp client try again!");
		sleep(5);
	}

	return ((void *)0);
}

void startUdpClient(pthread_t *pUdpThread)
{
	pthread_create(pUdpThread, NULL, vUdpClientCallback, NULL);
	return;
}

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


void *repeaterChecker(void* args)
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
		
		sleep(10);
	}
	
	//Timeout: Close ApClientEnable
	if (!connected) {
		coap_printf("iwpriv apcli0 set ApCliEnable=0!");
		do_system("iwpriv apcli0 set ApCliEnable=0");
	}

	return ((void *)0);
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
	return;
}

void *startCoapThread(void *argc)
{    // 启动qlinkserver
    Qlink_StartCoapServer();
    return ((void *)0);
}

void startCoapServer()
{
	//防止多次重启
	if (!g_coapExit) return;

	int ret;
    pthread_t id;

    g_coapExit = 0;
    ret = pthread_create(&id, NULL, startCoapThread, NULL);
    if(ret != 0) {
        coap_printf("create coap thread error!");
        exit(1);
    }

	coap_printf("coap server started!");
	pthread_detach(id);
	sleep(2);
	return;
}

void stopCoapServer()
{
	coap_printf("coap server stopped!");
	g_coapExit = 1;
}

void startCoapHeartbeat()
{
	int nRet, i;
	char *h_period = NULL;
	int t_period = 30;
	h_period = nvram_get(RT2860_NVRAM, "andlink_heartbeatTime");
	if (strlen(h_period) > 0) t_period = atoi(h_period)/1000;
	
	while(!g_coapExit) {
		nRet = device_inform_heartbeat(g_deviceId, gwip);
		if (nRet == AUTH_FAILED) {
			nvram_bufset(RT2860_NVRAM, "andlink_deviceId", "");
			nvram_commit(RT2860_NVRAM);
			memset(gwip, 0, sizeof(gwip));
			g_deviceId = nvram_get(RT2860_NVRAM, "andlink_deviceId");
			return;
		}
		
		coap_printf("coap heartbeat: %d, ret: %d", t_period, nRet);
		for(i=0; i < t_period; i++) {
			sleep(1);
		}
	}
    
	return;
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

	//g_dev_ip = get_ipaddr(ifname);
	g_dev_ip = get_ipaddr("br0");
	g_dev_boardcast_ip = get_boardcast_ipaddr("br0");
	g_dev_mac = get_macaddr(ifname);
	g_dev_up_mac = get_macaddr_colon(ifname);
	g_dev_type = nvram_get(RT2860_NVRAM, "andlink_deviceType");
	coap_printf("g_dev_lan_ip: %s, g_dev_boardcast_ip: %s, g_dev_mac: %s, g_dev_type: %s,andlink_token: %s", 
			g_dev_ip, g_dev_boardcast_ip, g_dev_mac, g_dev_type, nvram_get(RT2860_NVRAM, "andlink_andlinkToken"));

	if (NULL == g_dev_type || 0 == strlen(g_dev_type))
		coap_printf("No dev type!");
	else
		ret = 1;

	coap_printf("ret: %d", ret);
	return ret;
}


/*
 * return value: 1 success, 0 failed
 *
 */
static void init_network(void)
{
	int ret = 0;

	while(!ret)
	{
		/* do something. init network */
		if (get_popen_str("cat /etc/resolv.conf", "nameserver ", gwip, sizeof(gwip)))
		{
			coap_printf("GWIP: %s", gwip);
			/* 不等于默认地址，则视为从上级获取的地址 */
			if (strncmp("8.8.8.8", gwip, strlen("8.8.8.8")))
			{
				ret = 1;
			}
		}

		coap_printf("ret: %d", ret);

		sleep(3);
	}

	return ;
}

void start_process(void)
{
	if (!init()) return;

	g_deviceId = nvram_get(RT2860_NVRAM, "andlink_deviceId");
	int mode = BRIDGE_MODE;
	char *q_link_ok = NULL;
	int ret, i = 0;

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

		/* Q-LINK 2: 发送入网请求 */
		qlink_request(g_deviceId, gwip);

		//wait for qlink ok
		int startTime = time(NULL);
		int currentTime = time(NULL);
		while (currentTime - startTime < 100) {
			q_link_ok = nvram_get(RT2860_NVRAM, "Q_LINK_OK");
			if (!strcmp(q_link_ok, "1")) {
				break;
			}

			currentTime = time(NULL);
			sleep(2);
		}
	}
	else
	{
		/* 初始化网关地址 */
		init_network();
	}

	//1. 启用Coap服务
	startCoapServer();

	//2. 检测网关类型
	ret = searchGateWay();

	//3. 家庭网关或云网关相关处理
	if (ret == HOME_GATEWAY) {
		coap_printf("find home gateway, deviceId:%s, gwip:%s.", g_deviceId, gwip);
		/********** 家庭网关的场景  ***********/
		/* 设备上线 */	
		ret = device_inform_online(g_deviceId, gwip);
		coap_printf("online ret: %d", ret);
		if (ret != 0) {
			coap_printf("device online error, return!\n");
	        if (ret == AUTH_FAILED) {
				nvram_bufset(RT2860_NVRAM, "andlink_deviceId", "");
				nvram_commit(RT2860_NVRAM);
				memset(gwip, 0, sizeof(gwip));
				g_deviceId = nvram_get(RT2860_NVRAM, "andlink_deviceId");
			}
			return;
		}

		/** inform gateway device online **/
		coap_printf("inform device online!");
		device_online_notify(g_deviceId, gwip);
		
		/* 开始心跳 */
		startCoapHeartbeat();
		
	} else {
		/********* 云网关的场景 *************/
		coap_printf("not found home gateway, switch to cloud gateway.");
		/** 1. 关闭coap server **/
		stopCoapServer();

		/** 2. 开启Udp 和 Tcp Client **/
		pthread_t updThread, tcpThread;
		startUdpClient(&updThread);
		
		startTcpClient(&tcpThread);

		pthread_join(updThread, NULL);
		pthread_join(tcpThread, NULL);
	}
	
	return;
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


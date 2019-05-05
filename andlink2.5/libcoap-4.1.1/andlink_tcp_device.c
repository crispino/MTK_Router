/******************************************************************************
 * file:andlink_tcp_device.cpp  
 * version: 1.0.0
 * description:- Tcp Client
 * modification history
 * --------------------
 * author: xiaoning 2018-06-12 15:06:48
 * --------------------
*******************************************************************************/


#include "andlink_tcp_device.h"
#include "router_cfg.h"
#include "andlink_tcp_utils.h"
#include "debug.h"
#include "andlink.h"
#include "zlink_aes_ecb.h"

extern volatile int g_tcpExit;
extern volatile int g_udpExit;

char g_DevRND[DevRND_LENGTH + 1] = {0};
char g_ChallengeCode[ChallengeCode_LENGTH + 1] = {0};


void vGenerateDevSN(char *devSN)
{
	struct timeval tv;

	//1. 设置随机码的种子
	memset(&tv, 0, sizeof(tv));
	gettimeofday(&tv, NULL);
	srand(tv.tv_sec + tv.tv_usec);

	//生成Md5的值
	char sequence[32] = {0};
	snprintf(sequence, sizeof(sequence)-1, "%d%s", rand(), g_dev_mac);
	md5Data(sequence, devSN);

	return;
}

int nTcpExit()
{
	return g_tcpExit;
}

int nHeartBeatChanged(int nProtocol, int nHeartBeat)
{
	if (nHeartBeat <= 0) {
		coap_printf("heartbeat [%d] format error!", nHeartBeat);
		return ANDLINK_ERROR;
	}
	
	char heartbeat_buf[16] = {0};
	sprintf(heartbeat_buf, "%d", nHeartBeat);
	if (nProtocol == PROTO_UDP) {
		nvram_bufset(RT2860_NVRAM, "andlink_udpHeartbeat", heartbeat_buf);
	} else {
		nvram_bufset(RT2860_NVRAM, "andlink_tcpHeartbeat", heartbeat_buf);
	}
	nvram_commit(RT2860_NVRAM);
	
	return ANDLINK_OK;
}

int nCloudServerChanged(int nProtocol, const char* serverIP)
{
	if (!serverIP) {
		coap_printf("server [%s] format error!", serverIP);
		return ANDLINK_ERROR;
	}
	
	if (nProtocol == PROTO_UDP) {
		nvram_bufset(RT2860_NVRAM, "andlink_udpServer", serverIP);
		nvram_commit(RT2860_NVRAM);
		rebootUdpClient();
	} else {
		nvram_bufset(RT2860_NVRAM, "andlink_tcpServer", serverIP);
		nvram_commit(RT2860_NVRAM);
		rebootTcpClient();
	}
	
	return ANDLINK_OK;
}

int nCreateUdpServer(int nPort, UdpDataCallbackFunc cbFunction)
{
	if (cbFunction == NULL || nPort <= 0) {
		coap_printf("input parameters error!");
		return -1;
	}
	
	int sockfd, n, nLength;
	char sMessage[MAX_DATA_LENGTH + 1];
	struct sockaddr_in svraddr, cliaddr;
	socklen_t len;
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (-1 == sockfd) {
		coap_printf("create udp socket failed!");
		return -1;
	}

	memset(&svraddr, 0x0, sizeof(svraddr));
	svraddr.sin_family = AF_INET;
	svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	svraddr.sin_port = htons(nPort);
	if (-1 == bind(sockfd, (struct sockaddr *)&svraddr, sizeof(svraddr))) {
		coap_printf("udp bind failed!");
		close(sockfd);
		return -1;
	}

	while(!g_udpExit) {
		nLength = nUdpRecv(sockfd, sMessage, (struct sockaddr *)&cliaddr, &len);
		if (nLength < 0) {
			coap_printf("udp server recvfrom error");
			break;
		}
		
		//callback handler
		cbFunction(sockfd, (struct sockaddr *)&cliaddr, sMessage, nLength);

		usleep(50000); //50ms
	}
	
	close(sockfd);
	return;
}

void rebootTcpClient( )
{
	coap_printf("reboot tcp client");
	g_tcpExit = 1;
	return;
}

void rebootUdpClient()
{
	coap_printf("reboot udp client");
	g_udpExit = 1;
	return;
}

int nInitTcpClient(recProtoClient *pTcpClient, TcpDataCallbackFunc cb)
{
	int nKeepAlive =0;
	
	if (pTcpClient == NULL || cb == NULL) {
		coap_printf("input parameters error!");
		return ANDLINK_ERROR;
	}

	char *serverIP = nvram_get(RT2860_NVRAM, "andlink_tcpServer");
	char *heartBeat = nvram_get(RT2860_NVRAM, "andlink_tcpHeartbeat");
	if (strlen(heartBeat) == 0) {
		nKeepAlive = 60;
	} else {
		nKeepAlive = atoi(heartBeat)/1000;
	}

	char *pos = strchr(serverIP, ':');
	if (!pos) {
		coap_printf("cloud server [%s] format error!", serverIP);
		return ANDLINK_ERROR;
	}

	strncpy(pTcpClient->sServerAddr, serverIP, pos - serverIP);
	pTcpClient->nServerPort = atoi(pos + 1);
	pTcpClient->nKeepAlive = nKeepAlive;	

	coap_printf("serverAddr:%s, port:%d, heartBeat:%d", 
		pTcpClient->sServerAddr,
		pTcpClient->nServerPort,
		pTcpClient->nKeepAlive);
	
	vSetTcpDataCallbackFunc(pTcpClient, cb);
	return ANDLINK_OK;
}

int nInitUdpClient(recProtoClient *pUdpClient, UdpDataCallbackFunc cb)
{
	int nKeepAlive;
	if (pUdpClient == NULL || cb == NULL) {
		coap_printf("input parameters error!");
		return ANDLINK_ERROR;
	}

	char *serverIP = nvram_get(RT2860_NVRAM, "andlink_udpServer");
	char *heartBeat = nvram_get(RT2860_NVRAM, "andlink_udpHeartbeat");
	if (strlen(heartBeat) == 0) {
		nKeepAlive = 60;
	} else {
		nKeepAlive = atoi(heartBeat)/1000;
	}

	char *pos = strchr(serverIP, ':');
	if (!pos) {
		coap_printf("cloud server [%s] format error!", serverIP);
		return ANDLINK_ERROR;
	}

	strncpy(pUdpClient->sServerAddr, serverIP, pos - serverIP);
	pUdpClient->nServerPort = UDP_SERVER_PORT;
	pUdpClient->nKeepAlive = nKeepAlive;

	coap_printf("serverIP:%s, port:%d, heartBeat:%d", 
		pUdpClient->sServerAddr,
		pUdpClient->nServerPort, 
		pUdpClient->nKeepAlive);

	vSetUdpDataCallbackFunc(pUdpClient, cb);
	return ANDLINK_OK;
}


int nComposeKeepAliveMessage(char *data, int *pLength)
{
	cJSON *json = NULL;
	json = cJSON_CreateObject();
	if (!json) return ANDLINK_JSON_ERR;
	
	cJSON_AddItemToObject(json, "RPCMethod", cJSON_CreateString("heartbeat"));
	cJSON_AddItemToObject(json, "Mac", cJSON_CreateString(g_dev_mac));
	cJSON_AddItemToObject(json, "IPAddr", cJSON_CreateString(g_dev_ip));

	char *pTemp = cJSON_PrintUnformatted(json);
	*pLength = strlen(pTemp);
	strncpy(data, pTemp, *pLength);

	free(pTemp);
	cJSON_Delete(json);
	return ANDLINK_OK;
}

int nComposeBootMessage(char *data, int *pLength)
{
	cJSON *json = NULL;
	cJSON *XData = NULL;
	json = cJSON_CreateObject();
	if (!json) return ANDLINK_JSON_ERR;

	vGenerateDevSN(g_DevRND);
	cJSON_AddItemToObject(json, "RPCMethod", cJSON_CreateString("boot"));
	cJSON_AddItemToObject(json, "DevRND", cJSON_CreateString(g_DevRND));
	cJSON_AddItemToObject(json, "deviceMac", cJSON_CreateString(g_dev_mac));
	cJSON_AddItemToObject(json, "deviceType", cJSON_CreateString(g_dev_type));
	
	FILE *fp = fopen("/etc_ro/version", "r");
	char version[64] = {S_VERSION};
	if (fp != NULL) {fgets(version, sizeof(version), fp); version[strlen(version)-1] = '\0'; fclose(fp);}
	cJSON_AddItemToObject(json, "firmwareVersion", cJSON_CreateString(H_VERSION));
	cJSON_AddItemToObject(json, "softwareVersion", cJSON_CreateString(version)); //多个版本号则用分号隔开?
	/* 确保IP地址准确 */
	char *ifname = NULL;
	ifname = get_ifname();
	cJSON_AddItemToObject(json, "ipAddress", cJSON_CreateString(get_ipaddr(ifname)));
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
	
	char deviceSn[18] = "13111805000001"; //default
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
	cJSON_AddItemToObject(XData, "user_key", cJSON_CreateString(nvram_get(RT2860_NVRAM, "andlink_userKey")));
	cJSON_AddItemToObject(XData, "SyncCode", cJSON_CreateString(nvram_get(RT2860_NVRAM, "SyncCode")));

	char *pTemp = cJSON_PrintUnformatted(json);
	*pLength = strlen(pTemp);
	strncpy(data, pTemp, *pLength);

	free(pTemp);
	cJSON_Delete(json);
	return ANDLINK_OK;

}

int nComposeRegisterMessage(char *data, int *pLength)
{
	cJSON *json = NULL;
	json = cJSON_CreateObject();
	if (!json) return ANDLINK_JSON_ERR;
	
	cJSON_AddItemToObject(json, "RPCMethod", cJSON_CreateString("Register"));
	cJSON_AddItemToObject(json, "MAC", cJSON_CreateString(g_dev_mac));

	char checkSN[33] = {0};
	char sequence[64] = {0};
	char deviceSn[18] = "13111805000001"; //default
	router_getDevSn(deviceSn, sizeof(deviceSn));
	snprintf(sequence, sizeof(sequence)-1, "%s%s", g_ChallengeCode, deviceSn);
	md5Data(sequence, checkSN);
	cJSON_AddItemToObject(json, "CheckSN", cJSON_CreateString(checkSN));

	char *pTemp = cJSON_PrintUnformatted(json);
	*pLength = strlen(pTemp);
	strncpy(data, pTemp, *pLength);

	free(pTemp);
	cJSON_Delete(json);
	return ANDLINK_OK;
}

int nParseBootResponse(int nProtocol, const char *data, int data_len)
{
	if (data == NULL || data_len <= 0) {
		coap_printf("data input invalid");
		return ANDLINK_ERROR;
	}
	
	cJSON *json = cJSON_Parse(data);
	if (!json) {
		coap_printf("system error!");
		return ANDLINK_ERROR;
	}
	
	if(!cJSON_HasObjectItem(json, "respCode") || !cJSON_HasObjectItem(json, "ChallengeCode") 
		|| !cJSON_HasObjectItem(json, "ServerIP")) {
		cJSON_Delete(json);
		coap_printf("lack of parameter!");
		return ANDLINK_ERROR;
	}

	memset(g_ChallengeCode, 0, sizeof(g_ChallengeCode));
	char *challengeCode = cJSON_GetObjectItem(json, "ChallengeCode")->valuestring;
	strncpy(g_ChallengeCode, challengeCode, ChallengeCode_LENGTH);

	int respCode = cJSON_GetObjectItem(json, "respCode")->valueint;
	if (respCode == CLOUD_SUCCESS) return ANDLINK_OK;
	
	if (respCode == CLOUD_SERVER_CHANGED) {
		char *serverIP = cJSON_GetObjectItem(json, "ServerIP")->valuestring;
		nCloudServerChanged(nProtocol, serverIP);
	}

	return ANDLINK_ERROR;
}

int nParseRegisterResponse(int nProtocol, const char *data, int data_len)
{
	if (data == NULL || data_len <= 0) {
		coap_printf("data input invalid");
		return ANDLINK_ERROR;
	}

	cJSON *json = cJSON_Parse(data);
	if (!json) {
		coap_printf("system error!");
		return ANDLINK_ERROR;
	}
	
	if(!cJSON_HasObjectItem(json, "respCode")) {
		cJSON_Delete(json);
		coap_printf("lack of parameter!");
		return ANDLINK_ERROR;
	}

	int nRet = ANDLINK_OK;
	if(cJSON_HasObjectItem(json, "heartBeatTime")) {
		int nKeepAlive = cJSON_GetObjectItem(json, "heartBeatTime")->valueint;
		nHeartBeatChanged(nProtocol, nKeepAlive);
	}

	int respCode = cJSON_GetObjectItem(json, "respCode")->valueint;
	if (respCode == CLOUD_AUTH_SUCCESS) {
		if(cJSON_HasObjectItem(json, "MessageServer")) {
			char *serverIP = cJSON_GetObjectItem(json, "MessageServer")->valuestring;
			nvram_bufset(RT2860_NVRAM, "andlink_messageServer", serverIP);
			nvram_commit(RT2860_NVRAM);
		}
		
		nRet = ANDLINK_OK;
		
	} else if (respCode == CLOUD_SERVER_CHANGED) {
		char *serverIP = cJSON_GetObjectItem(json, "ServerIP")->valuestring;
		nCloudServerChanged(nProtocol, serverIP);
		nRet = ANDLINK_ERROR;	
	}
		
	return nRet;
	
}

int nParseTcpHeaderLength(const char *sMessage, int *pLen)
{
	if (sMessage == NULL || pLen == NULL) return ANDLINK_ERROR;

	*pLen = 0;
	memcpy(pLen, sMessage, TCP_HEADER_LENGTH);
	*pLen = ntohl(*pLen);

	coap_printf("tcp header length: %d", *pLen);
	if (*pLen <= 0) return ANDLINK_ERROR;
	
	return ANDLINK_OK;
}

void vSetTcpDataCallbackFunc(recProtoClient *pProtoClient, TcpDataCallbackFunc cb)
{
    pProtoClient->tcpCallbackFunc = cb;
}

void vSetUdpDataCallbackFunc(recProtoClient *pProtoClient, UdpDataCallbackFunc cb)
{
    pProtoClient->udpCallbackFunc = cb;
}

int nTcpPushData(int nSockId, const char *data, int data_len)
{
	int nRet;
	
	if (data == NULL || data_len <=0){
		coap_printf("push data format error");
		return ANDLINK_ERROR;
	}

	coap_printf("data :%s, LEN: %d", data, data_len);
	
	char *sMessageServer = nvram_get(RT2860_NVRAM, "andlink_messageServer");
	if (strlen(sMessageServer) < 10)  {
		nRet = nTcpTransResponse(nSockId, data, data_len);
		return nRet;
	}
	
	/*** message server exists ***/
	char *pos = strchr(sMessageServer, ':');
	if (!pos) {
		coap_printf("message server [%s] format error!", sMessageServer);
		return ANDLINK_ERROR;
	}

	//1. connect to message server
	char sHost[SERVER_ADDR_LENGTH + 1] = {0};
	strncpy(sHost, sMessageServer, pos - sMessageServer);
	int nPort = atoi(pos + 1);
	int nMessageSockId = nTcpConnect(sHost, nPort);
	if (nSockId < 0) {
		coap_printf("Andlink connect to message server[%s:%d] error!", sHost, nPort);
		return ANDLINK_ERROR;
	}

	//2. report data 
	nTcpTransResponse(nMessageSockId, data, data_len);
	
	//3. close socket
	nTcpClose(nMessageSockId);

	return nRet;	
}

int nTcpTransResponse(int nSockId, const char *data, int data_len)
{
	if (data == NULL || data_len <= 0 || nSockId <= 0) {
		coap_printf("input parameters error!");
		return ANDLINK_ERROR;
	}

	int nRet = 0;	
	char *pData = (char*)malloc(TCP_HEADER_LENGTH + data_len + 1);
	if (pData == NULL) {
		coap_printf("data malloc error!");
		return ANDLINK_ERROR;
	}
	memset(pData, 0, TCP_HEADER_LENGTH + data_len + 1);

	//1. Tcp Header
	int nLen = htonl(data_len);
	memcpy(pData, &nLen, TCP_HEADER_LENGTH);
	
	//2. Tcp Body
	memcpy(pData + TCP_HEADER_LENGTH, data, data_len);

	//3. Send Message
	nRet = nTcpWriten(nSockId, pData, data_len + TCP_HEADER_LENGTH);
    if (nRet < 0) {
        coap_printf ("Tcp send Response data error [%d]\n", nRet);
		free(pData);
        return TCP_NETWORK_ERROR;
    }

	free(pData);
	return ANDLINK_OK;
}

int nUdpRecv(int nSockId, struct sockaddr *sendaddr, const char *data, int data_len)
{
	socklen_t len;
	int nLength;

	if (data == NULL || data_len <= 0 || nSockId < 0 || sendaddr == NULL) {
		coap_printf("input parameters error!");
		return ANDLINK_ERROR;
	}

	len = sizeof(struct sockaddr_in);
	nLength = recvfrom(nSockId, data, data_len, 0, sendaddr, &len);
	return nLength;
}

int nUdpWrite(int nSockId, struct sockaddr *sendaddr, const char *data, int data_len)
{
	int nRet;
	
	if (data == NULL || data_len <= 0 || nSockId < 0 || sendaddr == NULL) {
		coap_printf("input parameters error!");
		return ANDLINK_ERROR;
	}

	nRet = sendto(nSockId, data, data_len, 0, sendaddr, sizeof(struct sockaddr_in));
	if (nRet < 0) {
		coap_printf("send udp message error!");
		return ANDLINK_ERROR;
	}
	
	return ANDLINK_OK;
}

void* vTcpKeepAliveThread( void* arg )
{
    int i, nRet, nLength;
    char sKeepAliveBuffer[KEEP_ALIVE_LENGTH + 1] = {0};
	
	recProtoClient *pTcpClient = (recProtoClient*)arg;
	if (pTcpClient == NULL) {
		coap_printf("system exception.");
		return NULL;
	}
	
	if (pTcpClient->nSockId <= 0) {
		coap_printf("keepalive thread exit, sockfd[0]");
	    return NULL;
	}

	//keep alive time double check
	int nKeepAlive = pTcpClient->nKeepAlive;
	if (nKeepAlive <= 10 || nKeepAlive >= 300) {
		nKeepAlive = 60;
	}

	//compose the keep alive message
	nLength = 0;
    nComposeKeepAliveMessage(sKeepAliveBuffer, &nLength);
    
    while(!g_tcpExit)
    {
        //send keepalive message
        coap_printf("data :%s, LEN : %d", sKeepAliveBuffer, nLength);
        nRet = nTcpTransResponse(pTcpClient->nSockId, sKeepAliveBuffer, nLength);
        if (nRet == ANDLINK_ERROR) {
            coap_printf("Send heart beat error, break...\n");
			break;			
        }

		//Sleep
		for (i = 0; i < nKeepAlive; i++) {
        	sleep(1);
		}
    }

	coap_printf("keepalive thread exit.");
	g_tcpExit = 1;
    pthread_exit(NULL);
    return NULL;
}


void* vTcpReceiveThread( void* arg )
{
	int nRet, nLength;
    char sMessage[MAX_DATA_LENGTH] = {0};
	char sHeader[TCP_HEADER_LENGTH + 1] = {0};

	recProtoClient* pTcpClient = (recProtoClient*)arg;
	if (pTcpClient == NULL) {
		coap_printf("input parameters error.");
		return NULL;
	}
	
	if (pTcpClient->tcpCallbackFunc == NULL) {
		coap_printf("transCallback function not defined.");
		return NULL;
	}

	if (pTcpClient->nSockId == 0) {
		coap_printf("keepalive thread exit, sockfd[0]");
	    return NULL;
	}
	
    while(!g_tcpExit) {
		
		//1. read message header
		memset(&sHeader, 0, sizeof(sHeader));
		nRet = nTcpReadn(pTcpClient->nSockId, sHeader, TCP_HEADER_LENGTH, -1);
		if (nRet != TCP_HEADER_LENGTH) {
		    coap_printf("Receive Head fail, ret=%d, errno[%d] exit.\n", nRet, errno);
			break;
		}

		nLength = 0;
		nRet = nParseTcpHeaderLength(sHeader, &nLength);
		if (nRet != ANDLINK_OK) {
		    coap_printf("error parse message header[%d], Len[%d]\n", nRet, nLength);
		    break;
		}

		if (nLength > MAX_DATA_LENGTH || nLength < 0) {
			coap_printf("Trans data is size invalid, length[%d]", nLength);
			break;
		}

		//2. read message body
		nRet = nTcpReadn(pTcpClient->nSockId, sMessage, nLength, MacRecvTimeOut);
	    if (nRet < 0) {
	        coap_printf("Busuiness Recv data errro, ret=%d.\n", nRet);
			break;
	    } else if (nRet < nLength) {
	        coap_printf("Busuiness Recv data errro, ret = %d.\n", nRet);
	        break;
	    }

		//callback function
		pTcpClient->tcpCallbackFunc(pTcpClient->nSockId, sMessage, nLength);

	    usleep(50000); //50ms
    }

	coap_printf("receive thread exit.");
	g_tcpExit = 1;
    pthread_exit(NULL);
    return NULL;
}


void* vUdpKeepAliveThread( void* arg )
{
    int i, nRet, nLength;
    char sKeepAliveBuffer[KEEP_ALIVE_LENGTH + 1] = {0};
	
	recProtoClient *pUdpClient = (recProtoClient*)arg;
	if (pUdpClient == NULL) {
		coap_printf("system exception.");
		return NULL;
	}
	
	if (pUdpClient->nSockId < 0 || pUdpClient->sendaddr == NULL) {
		coap_printf("keepalive thread exit, sockfd[0]");
	    return NULL;
	}

	//keep alive time double check
	int nKeepAlive = pUdpClient->nKeepAlive;
	if (nKeepAlive <= 10 || nKeepAlive >= 300) {
		nKeepAlive = 60;
	}

	//compose the keep alive message
	nLength = 0;
    nComposeKeepAliveMessage(sKeepAliveBuffer, &nLength);
    
    while(!g_udpExit)
    {
        //send keepalive message
        nRet = nUdpWrite(pUdpClient->nSockId, pUdpClient->sendaddr, sKeepAliveBuffer, nLength);
        if (nRet != ANDLINK_OK) {
            coap_printf("Send heart beat error, break...\n");
			break;
        }

		//Sleep
		for (i = 0; i < nKeepAlive; i++) {
        	sleep(1);
		}
    }

	coap_printf("keepalive thread exit.");
	g_udpExit = 1;
    pthread_exit(NULL);
    return NULL;
}


void* vUdpReceiveThread( void* arg )
{
	int nLength;
	char sMessage[MAX_DATA_LENGTH + 1];

	recProtoClient* pUdpClient = (recProtoClient*)arg;
	if (pUdpClient == NULL) {
		coap_printf("input parameters error.");
		return NULL;
	}
	
	if (pUdpClient->udpCallbackFunc == NULL || pUdpClient->sendaddr == NULL) {
		coap_printf("transCallback function not defined or sendaddr is null.");
		return NULL;
	}

	if (pUdpClient->nSockId == 0) {
		coap_printf("keepalive thread exit, sockfd[0]");
	    return NULL;
	}
	
	while(!g_udpExit) {

		memset(sMessage, 0, sizeof(sMessage));
		nLength = nUdpRecv(pUdpClient->nSockId, pUdpClient->sendaddr, sMessage, MAX_DATA_LENGTH);
		if (nLength < 0) {
			coap_printf("udp server recvfrom error");
			break;
		}

		//callback handler
		pUdpClient->udpCallbackFunc(pUdpClient->nSockId, pUdpClient->sendaddr, sMessage, nLength);

		usleep(50000); //50ms
	}
	
	coap_printf("receive thread exit.");
	g_udpExit = 1;
    pthread_exit(NULL);
    return NULL;
}

int nCreateTcpClient(recProtoClient *pTcpClient, TcpDataCallbackFunc cb)
{
	int nSockId, nRet;
	char sMessage[MAX_DATA_LENGTH + 1] = {0};
	char sHeader[TCP_HEADER_LENGTH + 1] = {0};
	pthread_t  tRcvThreadId, tActiveThreadId;

	nInitTcpClient(pTcpClient, cb);
	if (nRet != ANDLINK_OK) {
		coap_printf("init tcp client error");
		return nRet;
	}
		
	nSockId = nTcpConnect(pTcpClient->sServerAddr, pTcpClient->nServerPort);
	if (nSockId <= 0) {
		coap_printf("Andlink connect to server error!");
		return ANDLINK_ERROR;
	}

	pTcpClient->nSockId = nSockId;

	// 1.1 发送boot请求
	int nLength = 0;
	memset(sMessage, 0 ,sizeof(sMessage));
	nRet = nComposeBootMessage(sMessage, &nLength);
	if (nRet != ANDLINK_OK) {
		coap_printf("compose boot message error!");
		return nRet;
	}

	coap_printf("data: %s, Len: %d", sMessage, nLength);
	nRet = nTcpTransResponse(pTcpClient->nSockId, sMessage, nLength);
	if (nRet != ANDLINK_OK) {
		coap_printf("send boot message error!");
		return nRet;
	}

	// 1.2 接收boot应答头部
	nRet = nTcpReadn(pTcpClient->nSockId, sHeader, TCP_HEADER_LENGTH, MacRecvTimeOut);
	if (nRet != TCP_HEADER_LENGTH) {
	    coap_printf("Receive Head fail, ret=%d, errno[%d] exit.\n", nRet, errno);
		return nRet;
	}

	//1.3 解析boot消息头部长度
	nLength = 0;
	nRet = nParseTcpHeaderLength(sHeader, &nLength);
	if (nRet != ANDLINK_OK) {
	    coap_printf("error parse message header[%d], Len[%d]\n", nRet, nLength);
	    return ANDLINK_ERROR;
	}

	// 1.4 接收boot消息Json
	memset(sMessage, 0 ,sizeof(sMessage));
	nRet = nTcpReadn(pTcpClient->nSockId, sMessage, nLength, MacRecvTimeOut);
	if (nRet < nLength) {
		coap_printf("Recv boot message errro, ret=%d.\n", nRet);
		return ANDLINK_ERROR;
	}

	// 1.5 解析boot消息内容
    nRet = nParseBootResponse(PROTO_TCP, sMessage, nLength);
	if (nRet != ANDLINK_OK) {
		coap_printf("Parse boot response error!");
		return ANDLINK_ERROR;
	}
	
	//2.1 发送Register请求
	nLength = 0;
	memset(sMessage, 0 ,sizeof(sMessage));
	nComposeRegisterMessage(sMessage, &nLength);
	if (nRet != ANDLINK_OK) {
		coap_printf("compose Register message error");
		return nRet;
	}

	nRet = nTcpTransResponse(pTcpClient->nSockId, sMessage, nLength);
	if (nRet != ANDLINK_OK) {
		coap_printf("send Register message error!");
		return nRet;
	}

	//2.2 接收Register应答头部
	memset(sHeader, 0, sizeof(sHeader));
	nRet = nTcpReadn(pTcpClient->nSockId, sHeader, TCP_HEADER_LENGTH, MacRecvTimeOut);
	if (nRet != TCP_HEADER_LENGTH) {
	    coap_printf("Receive Head fail, ret=%d, errno[%d] exit.\n", nRet, errno);
		return nRet;
	}

	//2.3 解析Register消息头部长度
	nLength = 0;
	nRet = nParseTcpHeaderLength(sHeader, &nLength);
	if (nRet != ANDLINK_OK) {
	    coap_printf("error parse message header[%d], Len[%d]\n", nRet, nLength);
	    return ANDLINK_ERROR;
	}

	//2.4 接收Register消息Json
	memset(sMessage, 0 ,sizeof(sMessage));
	nRet = nTcpReadn(pTcpClient->nSockId, sMessage, nLength, MacRecvTimeOut);
	if (nRet < nLength) {
		coap_printf("Recv Register message errro, ret=%d.\n", nRet);
		return ANDLINK_ERROR;
	}

	//2.5 解析boot消息内容
    nRet = nParseRegisterResponse(PROTO_TCP, sMessage, nLength);
	if (nRet != ANDLINK_OK) {
		coap_printf("Parse boot response error!");
		return ANDLINK_ERROR;
	}

	coap_printf("udp client created!!!");
	g_tcpExit = 0;

	//Create receive thread
    pthread_create(&tRcvThreadId, NULL, vTcpReceiveThread, pTcpClient);

    //Create keepalive thread
    pthread_create(&tActiveThreadId, NULL, vTcpKeepAliveThread, pTcpClient);

	pthread_join(tRcvThreadId, NULL);
    pthread_join(tActiveThreadId, NULL);

	nTcpClose(pTcpClient->nSockId);
	
	return ANDLINK_OK;
}

int nCreateUdpClient(recProtoClient *pUdpClient, UdpDataCallbackFunc cb)
{
	int nSockId, nRet;
	struct  sockaddr_in rServaddr;
	char sMessage[MAX_DATA_LENGTH + 1] = {0};
	pthread_t  tRcvThreadId, tActiveThreadId;

	nRet = nInitUdpClient(pUdpClient, cb);
	if (nRet != ANDLINK_OK) {
		coap_printf("init udp client error");
		return nRet;
	}
	
	nSockId = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (nSockId < 0)  {
		coap_printf("Andlink connect to server error!");
		 return ANDLINK_ERROR;
	}

	pUdpClient->nSockId = nSockId;

	//3. create serverAddr
    memset(&rServaddr, 0, sizeof(rServaddr));
    rServaddr.sin_family = AF_INET;
    rServaddr.sin_port = htons(pUdpClient->nServerPort);
	rServaddr.sin_addr.s_addr = inet_addr(pUdpClient->sServerAddr);

	pUdpClient->sendaddr = (struct sockaddr *)&rServaddr;
	
	// 1.1 发送boot请求
	int nLength = 0;
	memset(sMessage, 0 ,sizeof(sMessage));
	nRet = nComposeBootMessage(sMessage, &nLength);
	if (nRet != ANDLINK_OK) {
		coap_printf("compose boot message error");
		return nRet;
	}

	coap_printf("send boot message[%s]", sMessage);
	nRet = nUdpWrite(pUdpClient->nSockId, pUdpClient->sendaddr, sMessage, nLength);
	if (nRet != ANDLINK_OK) {
		coap_printf("send boot message error");
		return nRet;
	}

	// 1.2 接收boot请求
	memset(sMessage, 0 ,sizeof(sMessage));
	nLength = nUdpRecv(pUdpClient->nSockId, pUdpClient->sendaddr, sMessage,  MAX_DATA_LENGTH);
	if (nLength <= 0) {
		coap_printf("recv boot response error!");
		return ANDLINK_ERROR;
	}

	//解析boot应答
	coap_printf("recv boot message[%s]", sMessage);
	nRet = nParseBootResponse(PROTO_UDP, sMessage, nLength);
	if (nRet != ANDLINK_OK) {
		coap_printf("Parse boot response error!");
		return ANDLINK_ERROR;
	}
	
	// 2.1 发送Register请求
	nLength = 0;
	memset(sMessage, 0 ,sizeof(sMessage));
	nComposeRegisterMessage(sMessage, &nLength);
	if (nRet != ANDLINK_OK) {
		coap_printf("compose register message error");
		return nRet;
	}

	coap_printf("send register message[%s]", sMessage);
	nRet = nUdpWrite(pUdpClient->nSockId, pUdpClient->sendaddr, sMessage, nLength);
	if (nRet != ANDLINK_OK) {
		coap_printf("send register message error");
		return nRet;
	}

	// 2.2 接收register应答
	memset(sMessage, 0 ,sizeof(sMessage));
	nLength = nUdpRecv(pUdpClient->nSockId, pUdpClient->sendaddr, sMessage, MAX_DATA_LENGTH);
	if (nLength <= 0) {
		coap_printf("recv register message error");
		return ANDLINK_ERROR;
	}

	//解析register应答
	coap_printf("recv register message[%s]", sMessage);
	nRet = nParseRegisterResponse(PROTO_UDP, sMessage, nLength);
	if (nRet != ANDLINK_OK) {
		coap_printf("Parse register response error!");
		return ANDLINK_ERROR;
	}

	coap_printf("udp client created!!!");
	g_udpExit = 0;

	//3. create receive thread
    pthread_create(&tRcvThreadId, NULL, vUdpReceiveThread, pUdpClient);

    //4. create keepalive thread
    pthread_create(&tActiveThreadId, NULL, vUdpKeepAliveThread, pUdpClient);

	pthread_join(tRcvThreadId, NULL);
    pthread_join(tActiveThreadId, NULL);

	nTcpClose(pUdpClient->nSockId);
	
	return ANDLINK_OK;
}




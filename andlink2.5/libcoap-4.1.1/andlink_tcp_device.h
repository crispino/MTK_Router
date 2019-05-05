#ifndef ANDLINK_DEVICE_H
#define ANDLINK_DEVICE_H

#ifdef	__cplusplus
extern "C" {
#endif


#include "andlink_tcp_config.h"
#include "cJSON.h"

typedef void(*TcpDataCallbackFunc)(int nSockId,  const char *data, int data_len);
typedef void(*UdpDataCallbackFunc)(int nSockId,  struct sockaddr *sendaddr, const char *data, int data_len);


typedef struct {
	char sServerAddr[SERVER_ADDR_LENGTH + 1];
	int nServerPort;
	int nKeepAlive;
	int nSockId;
	struct sockaddr *sendaddr;
	
	TcpDataCallbackFunc   tcpCallbackFunc;
	UdpDataCallbackFunc	  udpCallbackFunc;
	
} recProtoClient;


/***云平台心跳间隔时间发生变化处理
	input : nHeartBeat  
	desc  : 心跳间隔 单位: 秒
**/
int nHeartBeatChanged(int nProtocol, int nHeartBeat);

int nCloudServerChanged(int nProtocol, const char* serverIP);

void rebootTcpClient( );

void rebootUdpClient( );

int nInitTcpClient(recProtoClient *pTcpClient, TcpDataCallbackFunc cb); 

int nInitUdpClient(recProtoClient *pUdpClient, UdpDataCallbackFunc cb); 

void vSetTcpDataCallbackFunc(recProtoClient *pProtoClient, TcpDataCallbackFunc cb);

void vSetUdpDataCallbackFunc(recProtoClient *pProtoClient, UdpDataCallbackFunc cb);

int nTcpPushData(int nSockId, const char *data, int data_len);

int nTcpTransResponse(int nSockId, const char *data, int data_len);

int nUdpWrite(int nSockId, struct sockaddr *sendaddr, const char *data, int data_len);

int nCreateUdpServer(int nPort, UdpDataCallbackFunc cbFunction);

int nCreateTcpClient(recProtoClient *pTcpClient, TcpDataCallbackFunc cb);

int nCreateUdpClient(recProtoClient *pTcpClient, UdpDataCallbackFunc cb);

int nParseTcpHeaderLength(const char *sMesage, int *pLen);

void vGenerateDevSN(char *devSN);

int nTcpExit();

#ifdef __cplusplus
}
#endif


#endif


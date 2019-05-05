#ifndef ANDLINK_TCP_UTILS_H
#define ANDLINK_TCP_UTILS_H

#ifdef	__cplusplus
extern "C" {
#endif


#include "andlink_tcp_config.h"


#define MacRecvTimeOut     10 
#define TCP_NETWORK_ERROR -1001
#define TCP_NETWORK_TIMEOUT -1002
#define TCPLIB_FAIL	-1000

int isIPAddr(const char *sValue);

int nTcpConnect(const char* sHost, int nServ);

int nTcpListen(const char*  sHost, int nServ);

int nTcpAccept(int nListenfd, char* sIPClient);


int nTcpReadn(int      nSockfd,
              char*    pvRecvBuf,
              size_t   nBytesToRead,
              int      nTimeout );

int nTcpWriten(int          nSockfd,
               const char*  pvSendBuf,
               size_t       nBytesToWrite);

int nTcpClose(int nSockfd);

int nTcpSelect(int nSockfd, int nTimeout);

#ifdef __cplusplus
}
#endif


#endif



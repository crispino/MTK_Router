/******************************************************************************
 * file:zlink_device.cpp  
 * version: 1.0.0
 * description:- a client to simulate the box or mobile
 * modification history
 * --------------------
 * author: xiaoning 2018-06-09 15:06:48
 * --------------------
*******************************************************************************/

#include "andlink_tcp_utils.h"


/*ÅÐ¶ÏÊÇ·ñÎªIPµØÖ·**/
int isIPAddr(const char *sValue)
{
    if (NULL == sValue) return -1;

    while(*sValue != '\0') {
		if ( (*sValue < '0' || *sValue > '9') && (*sValue!='.') )
		        return -1;
		
		sValue++;
    }
    return 0;
}

int nTcpConnect( const char* sHost, int nServ )
{
    int     nSockfd;
    int     nRetConn;
    struct  sockaddr_in rServaddr;
    struct  linger      rLinger;
    int     nRetSetOpt;
    struct hostent* h;

    memset(&rServaddr, 0, sizeof(rServaddr));
    if (isIPAddr(sHost) != 0)
    {
	    if(!(h=gethostbyname(sHost))) 
	    {
	        return TCPLIB_FAIL;
	    }
		bzero( (char *)&rServaddr, sizeof(rServaddr));
		bcopy(h->h_addr,&rServaddr.sin_addr,sizeof(rServaddr.sin_addr));
    }
    else
    {
        rServaddr.sin_addr.s_addr   = inet_addr(sHost);
    }
   

    rServaddr.sin_family        = AF_INET;
    rServaddr.sin_port          = htons((unsigned short)nServ);
    
    nSockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if( nSockfd < 0)
    {
        return TCPLIB_FAIL;
    }

    rLinger.l_onoff  = 1;
    rLinger.l_linger = 0;

    if( (nRetSetOpt = setsockopt(nSockfd, SOL_SOCKET, SO_LINGER, (char *)&rLinger, sizeof(rLinger))) != 0 )
    {
        nTcpClose(nSockfd);
        return TCPLIB_FAIL;
    }

    if(0 > (nRetConn = connect(nSockfd, (struct sockaddr*)&rServaddr,
                sizeof(rServaddr))))
    {
        nTcpClose(nSockfd);
        return TCPLIB_FAIL;
    }

    return nSockfd;
} 


#define  MacListenQueue 1024

/**************************************************************************
 Function Name 
 Description  : the call of this function will construct a tcp server on desig
                nated IP and PORT
 Parameters    : const char* sHost(I) - the IP address or hostname of server
                 if NULL,the IP address decided by system
                  int nServ(I) - the port on which server will listen
 
 Return Value : int
                   TCPLIB_OK success
                   TCPLIB_FAIL error
 *************************************************************************/
int nTcpListen(const char* sHost, int nServ)
{
    int     nSockfd;
    int     nRetSetOpt;
    int     nRetBnd;
    int     nOne;

    struct  sockaddr_in rServaddr;
    struct  linger      rLinger;

    
    memset(&rServaddr, 0, sizeof(rServaddr));

    rServaddr.sin_family = AF_INET;
    rServaddr.sin_port   = htons((unsigned short)nServ);
    if(sHost == NULL)
    {
        rServaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        rServaddr.sin_addr.s_addr = inet_addr(sHost);
    }

    
    if( (nSockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) < 0 )
    {
        return TCPLIB_FAIL;
    }

    
    nOne = 1;
    if( (nRetSetOpt = setsockopt(nSockfd, SOL_SOCKET, SO_REUSEADDR,(char *)&nOne, sizeof(nOne))) != 0 )
    {
        nTcpClose(nSockfd);
        return TCPLIB_FAIL;
    }
    
    rLinger.l_onoff  = 1;
    rLinger.l_linger = 0;

    if( (nRetSetOpt = setsockopt(nSockfd, SOL_SOCKET, SO_LINGER, (char *)&rLinger, sizeof(rLinger))) != 0 )
    {
        nTcpClose(nSockfd);
        return TCPLIB_FAIL;
    }

    
    if( (nRetBnd = bind(nSockfd, (struct sockaddr*)&rServaddr, sizeof(rServaddr))) < 0 )
    {
        nTcpClose(nSockfd);
        return TCPLIB_FAIL;
    }    

    
    if( listen(nSockfd, MacListenQueue) < 0 )
    {
        nTcpClose(nSockfd);
        return TCPLIB_FAIL;
    }

    return nSockfd;
}


/**************************************************************************
 Function Name 
 Description : this function will block until a client construct a connetion 
                     to the server and the function will return a socket file 
                    descriptor representing the construted connnection
 
 Parameters : int nListenfd(I) - the socket descriptor returned by nTcpListen
                char* sIPClient(O) - the IP address of client who constructed 
                                        the connection
 
 Return Value : int 
                  > 0  success
                  TCPLIB_FAIL error
 Return Value  :
 *************************************************************************/
int nTcpAccept(int nListenfd, char* sIPClient)
{
    int     nSockfd;
    int     nLenCliAddr;
    
    struct sockaddr_in rCliAddr;

    nLenCliAddr = sizeof(rCliAddr);
    memset(&rCliAddr, 0, sizeof(rCliAddr));
    
    for(;;)
    {
        nSockfd = accept(nListenfd, (struct sockaddr*)&rCliAddr, (socklen_t*)&nLenCliAddr); 
        
        if( nSockfd < 0 )
        {
            if( EINTR == errno || ENOBUFS == errno ) 
                continue;
            else
                return TCPLIB_FAIL;
        }
        break;
    }

    if(sIPClient != NULL)
    {
        strcpy(sIPClient, (const char*)inet_ntoa(rCliAddr.sin_addr));
    }

    return nSockfd;
} 


/**************************************************************************
 Function Name 
 Description   : this function will read n bytes from designated socket 
                    descriptor or return less than n bytes when timeout
 
 Parameters    : int nSockfd(I) - socket fd from which data will be read
                                         the fd will be returned by nTcpConnect
                                            and nTcpAccept
                   void* pvRecvBuf(IO) - the buf to receive data
                   size_t nBytesToRead(I) - how many bytes the caller want to
                                                read 
                   int nTimeout(IO) - the timeout time ,unit is second,if NULL,
                   will block...,if *pnTimeout == -1,time out
 
 Return Value : int
                 > 0 - the bytes read
                 = 0 - socket fd is nTcpClosed
                 < 0 - error occured
 Return Value  :
 *************************************************************************/
int nTcpReadn(int    nSockfd,
              char*  pvRecvBuf,
              size_t nBytesToRead,
              int    nTimeout)
{
    int     nLeft;
    int     nRead;
    int     nRet;
    int     nRetSel;
    char*   pvTmp;

    pvTmp = pvRecvBuf;
    nLeft = nBytesToRead;
    nRet  = 0;
    while( nLeft > 0 )
    {   
        nRetSel = nTcpSelect(nSockfd, nTimeout);
        if(nRetSel == 0)
        {  
            nRet = 0; 
            break;
        }
        if(nRetSel <0)
        {
            nRet = -1;
            break;
        }
        
        if((nRead = read(nSockfd, pvTmp, nLeft)) < 0)
        {
            if( errno == EINTR)
            {
                nRead = 0;
                continue;
            }
            else
            {
                nRet = -2;
                break;
            }
        }
        if(nRead == 0)
        {
            /*select >0 , read=0, means network error*/
            nRet = -3;
            break;
        }

        nLeft-=nRead;
        pvTmp+=nRead;
    }

    if ( pvTmp > pvRecvBuf)  
    {
        return pvTmp-pvRecvBuf;
    }
    else 
    {
        return nRet;
    }
    
}


/**************************************************************************
 Function Name 
 Description   : this function will write n bytes data to designates socket fd
 Parameters    : int nSockfd(I) - socket fd to write data to
                 const void* pvSendBuf(I) - the buffer which contain the data
                                                 to write 
                 size_t nBytesToWrite(I) - how many bytes to write
 
 Return Value  : int
                    > 0 - bytes that have been written
                    -1 - error occured
 Parameters    :
 Return Value  :
 *************************************************************************/
int nTcpWriten(int          nSockfd,
               const char*  pvSendBuf,
               size_t       nBytesToWrite)
{
    int         nLeft;
    int         nWritten;
    const char* pcPtr;

    pcPtr = pvSendBuf;
    nLeft = nBytesToWrite;


    while( nLeft > 0 )
    {
        if( 0 >= (nWritten = write(nSockfd, pcPtr, nLeft)) )
        {
            if( EINTR == errno )
                nWritten = 0;
            else
				{
                	return -1;
                }
        }
        nLeft-=nWritten;
        pcPtr+=nWritten;
    }
    
    return nBytesToWrite;
}


int nTcpSelect(int  nSockfd, int  nTimeout)
{
    int     nRet;
    fd_set  fds;

    struct timeval timeout;
    struct timeval *prTimeout;

    if ( nSockfd < 0 )  return -1;
    
    prTimeout = &timeout;
    FD_ZERO(&fds);
    FD_SET((unsigned int)nSockfd, &fds);

    if ( nTimeout < 0 )
    {
        prTimeout = NULL; 
    }
    else
    {
        prTimeout->tv_sec = nTimeout;
        prTimeout->tv_usec = 0;
    }

    nRet =select(nSockfd+1, &fds, NULL, NULL, prTimeout);
    return nRet;
}


int nTcpClose( int nSockfd )
{
    return close(nSockfd);
}




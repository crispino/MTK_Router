#ifndef ANDLINK_TCP_CONFIG_H
#define ANDLINK_TCP_CONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif


#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <fcntl.h> 
#include <limits.h> 
#include <errno.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include<pthread.h>

#define UDP_SERVER_PORT 9007
#define TCP_SERVER_PORT	9005

#define PROTO_UDP	1
#define PROTO_TCP	2

#define SERVER_ADDR_LENGTH	64
#define MAX_DATA_LENGTH	1024
#define TCP_HEADER_LENGTH 4
#define KEEP_ALIVE_LENGTH 256
#define DevRND_LENGTH	32
#define ChallengeCode_LENGTH	16


#define ANDLINK_OK 			 0
#define ANDLINK_ERROR 			-1
#define ANDLINK_SYSTEM_ERR		-2
#define ANDLINK_JSON_ERR		-3


//cloud error code
#define INVALID_ID				 ""
#define DevRND_SUCCESS			 1
#define CLOUD_AUTH_SUCCESS		 2
#define CLOUD_AUTH_FAIL      	-2
#define CLOUD_SUCCESS			 0
#define CLOUD_FAIL		   		-1
#define CLOUD_NEED_REGISTER	    -3
#define CLOUD_SERVER_CHANGED	-5

#ifdef __cplusplus
}
#endif


#endif

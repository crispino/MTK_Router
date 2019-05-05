#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>  
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>


#include "ont/error.h"
#include "ont/platform.h"
#include "port_sock.h"


int ont_platform_tcp_create(ont_socket_t **sock)
{
    int flags = 1;
    int ret = 0;
    int s_buf_size = 655360; /*FOR MONVIF*/
	int on = 1;
    
    ont_socket_t *_sock;
    
    if ( !sock)
    {
        return ONT_ERR_BADPARAM;   
    }

    _sock =(ont_socket_t*)ont_platform_malloc(sizeof(ont_socket_t));

    _sock->fd = socket(AF_INET, SOCK_STREAM, 0);        
    if ( _sock->fd < 0 )        
    {   
        ont_platform_free(_sock);
        SL_DEBUG("errno: %d", errno);
        return ONT_ERR_SOCKET_OP_FAIL;        
    }  

	/*    
		flags = fcntl(_sock->fd, F_GETFL, 0);
		flags |= O_NONBLOCK;
		ret = fcntl( _sock->fd, F_SETFL, flags);        
	*/
	if ((ioctl(_sock->fd, FIONBIO, &on)) != 0)
	{
    	ont_platform_free(_sock);
		SL_DEBUG( "cannot set socket as non-blocking IO");
		ret = -1;
	}

    if ( 0 != ret )        
    {
    	ont_platform_free(_sock);
        SL_DEBUG("errno: %d", errno);
        return ONT_ERR_SOCKET_OP_FAIL;       
    }
    
    setsockopt(_sock->fd, SOL_SOCKET, SO_SNDBUF, (char *)&s_buf_size, sizeof(int));
        
    *sock = _sock;
    return ONT_ERR_OK;
}

int ont_platform_tcp_connect(ont_socket_t *sock, const char *ip, uint16_t port)
{
    int _sock=0;
    struct sockaddr_in saClient;
    int ret = 0;
    int errnumber = 0;

    if ( !sock )
    {
        return ONT_ERR_BADPARAM;   
    }
    
    _sock  = sock->fd;
    memset(&saClient, 0x00, sizeof(struct sockaddr));
    saClient.sin_family  = AF_INET;
    saClient.sin_port = htons(port);
    saClient.sin_addr.s_addr = inet_addr(ip);

	//SL_DEBUG();
    ret = connect(_sock, (const struct sockaddr *)(&saClient), sizeof(saClient));
    errnumber = errno;
	//SL_DEBUG();
    if (ret<0)
    {       
        if ( EINTR == errno || EINPROGRESS ==errno || EALREADY ==errno)
        {  
            printf("error num is %d \n", errno);
            return ONT_ERR_SOCKET_INPROGRESS;
        }
        else if (EISCONN == errno)
        {
            return ONT_ERR_SOCKET_ISCONN;
        }
        else
        {    
			if (0 == errnumber)
                return ONT_ERR_SOCKET_INPROGRESS;
			
             printf("error number is %d, ret = %d \n", errnumber, ret);
			 SL_DEBUG();
             return ONT_ERR_SOCKET_OP_FAIL;
        }  
    }
    
    return ONT_ERR_OK;      
}

int ont_platform_tcp_send(ont_socket_t *sock, const char *buf,
                          unsigned int size, unsigned int *bytes_sent)
{
    int ret;       
    int _sock=0;
    if ( !buf || !size || !bytes_sent || !sock ) 
    {
        return ONT_ERR_BADPARAM;
    }
    _sock  = sock->fd;
        
	ret = send(_sock , buf, size, MSG_NOSIGNAL);	
    if ( ret>=0 )   
    {
        *bytes_sent = (int)ret;        
        return ONT_ERR_OK;    
    }    
    else if ( EAGAIN == errno || EINTR==errno || 0==errno)  //benj: modify for test  
    { 
        *bytes_sent = 0;
        return ONT_ERR_OK;   
    }
    return ONT_ERR_SOCKET_OP_FAIL;      
}

int ont_platform_tcp_recv(ont_socket_t *sock, char *buf,
                          unsigned int size, unsigned int *bytes_read)
{
    int ret;     
    int _sock = 0;
    
    if ( !buf || !size || !bytes_read || !sock ) 
    {
        return ONT_ERR_BADPARAM; 
    }
    _sock  = sock->fd;
  
    ret = recv(_sock, buf, size, MSG_NOSIGNAL);  
    if ( ret>=0 )    
    {       
            
        *bytes_read = (int)ret;            
        return ONT_ERR_OK;        

    }    
    else if ( EAGAIN == errno || EINTR==errno || 0==errno) //benj: modify for test 
    {        
        *bytes_read = 0;        
        return ONT_ERR_OK;   
    }    
    return ONT_ERR_SOCKET_OP_FAIL;        
}


int ont_platform_tcp_close(ont_socket_t *sock)
{
    if ( !sock)
    {
        return ONT_ERR_BADPARAM;   
    }
    close(sock->fd);
    ont_platform_free(sock);
    return ONT_ERR_OK;
}



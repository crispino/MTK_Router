/* coap -- simple implementation of the Constrained Application Protocol (CoAP)
 *         as defined in draft-ietf-core-coap
 *
 * Copyright (C) 2010--2013 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use. 
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>

#include "config.h"
#include "resource.h"
#include "coap.h"
#include "Qlink_API.h"


#define COAP_RESOURCE_CHECK_TIME 2

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

/* temporary storage for dynamic resource representations */
//static int quit = 0;
extern volatile int g_coapExit;

static onReceiveResponse RecvDataHandle = NULL;

/* changeable clock base (see handle_put_time()) */
static time_t my_clock_base = 0;

struct coap_resource_t *time_resource = NULL;

#ifndef WITHOUT_ASYNC
/* This variable is used to mimic long-running tasks that require
 * asynchronous responses. */
static coap_async_state_t *async = NULL;
#endif /* WITHOUT_ASYNC */

/* SIGINT handler: set quit to 1 for graceful termination */
//void
//handle_sigint(int signum) {
//  quit = 1;
//}

#define COAP_SER_INDEX "This is a test server made with libcoap (see http://libcoap.sf.net)\n" \
   	      "Copyright (C) 2010--2013 Olaf Bergmann <bergmann@tzi.org>\n\n"

void Qlink_setReciveInternetChannelCallback(onReceiveResponse Fun)
{
    if(Fun) {
        RecvDataHandle = Fun;
    }
}

void 
hnd_get_index(coap_context_t  *ctx, struct coap_resource_t *resource, 
	      coap_address_t *peer, coap_pdu_t *request, str *token,
	      coap_pdu_t *response) {
  unsigned char buf[3];

  response->hdr->code = COAP_RESPONSE_CODE(205);

  coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
	  coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);

  coap_add_option(response, COAP_OPTION_MAXAGE,
	  coap_encode_var_bytes(buf, 0x2ffff), buf);
    
  coap_add_data(response, strlen(COAP_SER_INDEX), (unsigned char *)COAP_SER_INDEX);
}

void 
hnd_post(coap_context_t  *ctx, struct coap_resource_t *resource, 
	     coap_address_t *peer, coap_pdu_t *request, str *token,
	     coap_pdu_t *response) {
  coap_tick_t t;
  size_t size;
  unsigned char *data;

  /* FIXME: re-set my_clock_base to clock_offset if my_clock_base == 0
   * and request is empty. When not empty, set to value in request payload
   * (insist on query ?ticks). Return Created or Ok.
   */

  /* if my_clock_base was deleted, we pretend to have no such resource */
  response->hdr->code = 
    my_clock_base ? COAP_RESPONSE_CODE(204) : COAP_RESPONSE_CODE(201);

  resource->dirty = 1;

  coap_get_data(request, &size, &data);
  
  if (size == 0) {		/* re-init */
    coap_printf("client post none\n");
    return;
  } else {
      coap_printf(">>>>>>>%s,len:%d\n", data, size);
      if (RecvDataHandle) { 
          char *tmp = !RecvDataHandle(data, size) ? 
                      "{\"result\":1}":"{\"result\":0}";
          unsigned char buf[3];
          response->hdr->code = COAP_RESPONSE_CODE(205);
          
          coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
              coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
        
          coap_add_option(response, COAP_OPTION_MAXAGE,
              coap_encode_var_bytes(buf, 0x2ffff), buf);
            
          coap_add_data(response, strlen(tmp), (unsigned char *)tmp);
      }
  }
}

#ifndef WITHOUT_ASYNC
void 
hnd_get_async(coap_context_t  *ctx, struct coap_resource_t *resource, 
	      coap_address_t *peer, coap_pdu_t *request, str *token,
	      coap_pdu_t *response) {
  coap_opt_iterator_t opt_iter;
  coap_opt_t *option;
  unsigned long delay = 5;
  size_t size;

  if (async) {
    if (async->id != request->hdr->id) {
      coap_opt_filter_t f;
      coap_option_filter_clear(f);
      response->hdr->code = COAP_RESPONSE_CODE(503);
    }
    return;
  }

  option = coap_check_option(request, COAP_OPTION_URI_QUERY, &opt_iter);
  if (option) {
    unsigned char *p = COAP_OPT_VALUE(option);

    delay = 0;
    for (size = COAP_OPT_LENGTH(option); size; --size, ++p)
      delay = delay * 10 + (*p - '0');
  }

  async = coap_register_async(ctx, peer, request, 
			      COAP_ASYNC_SEPARATE | COAP_ASYNC_CONFIRM,
			      (void *)(COAP_TICKS_PER_SECOND * delay));
}

void 
check_async(coap_context_t  *ctx, coap_tick_t now) {
  coap_pdu_t *response;
  coap_async_state_t *tmp;

  size_t size = sizeof(coap_hdr_t) + 13;

  if (!async || now < async->created + (unsigned long)async->appdata) 
    return;

  response = coap_pdu_init(async->flags & COAP_ASYNC_CONFIRM 
			   ? COAP_MESSAGE_CON
			   : COAP_MESSAGE_NON,
			   COAP_RESPONSE_CODE(205), 0, size);
  if (!response) {
    debug("check_async: insufficient memory, we'll try later\n");
    async->appdata = 
      (void *)((unsigned long)async->appdata + 15 * COAP_TICKS_PER_SECOND);
    return;
  }
  
  response->hdr->id = coap_new_message_id(ctx);

  if (async->tokenlen)
    coap_add_token(response, async->tokenlen, async->token);

  coap_add_data(response, 4, (unsigned char *)"done");

  if (coap_send(ctx, &async->peer, response) == COAP_INVALID_TID) {
    debug("check_async: cannot send response for message %d\n", 
	  response->hdr->id);
  }
  coap_delete_pdu(response);
  coap_remove_async(ctx, async->id, &tmp);
  coap_free_async(async);
  async = NULL;
}
#endif /* WITHOUT_ASYNC */

void
init_resources(coap_context_t *ctx) {
  coap_resource_t *r;

  r = coap_resource_init(NULL, 0, 0);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_index);

  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
  coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"General Info\"", 14, 0);
  coap_add_resource(ctx, r);

#if 0
  /* store clock base to use in /time */
  my_clock_base = clock_offset;

  r = coap_resource_init((unsigned char *)"qlink/netinfo", 13, 0);
  coap_register_handler(r, COAP_REQUEST_POST, hnd_post);
  coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"QLINK-NETINFO Resource\"", 24, 0);

  coap_add_resource(ctx, r);
  time_resource = r;
#endif

  init_andlink_resources(&ctx);

#ifndef WITHOUT_ASYNC
#if 0
  r = coap_resource_init((unsigned char *)"async", 5, 0);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_async);

  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
  coap_add_resource(ctx, r);
#endif
#endif /* WITHOUT_ASYNC */
}

static coap_context_t *
get_context(const char *node, const char *port) {
  coap_context_t *ctx = NULL;  
  int s;
  struct addrinfo hints;
  struct addrinfo *result, *rp;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_DGRAM; /* Coap uses UDP */
  hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;
  
  s = getaddrinfo(node, port, &hints, &result);
  if ( s != 0 ) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return NULL;
  } 

  /* iterate through results until success */
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    coap_address_t addr;

    if (rp->ai_addrlen <= sizeof(addr.addr)) {
      coap_address_init(&addr);
      addr.size = rp->ai_addrlen;
      memcpy(&addr.addr, rp->ai_addr, rp->ai_addrlen);

      ctx = coap_new_context(&addr);
      if (ctx) {
	/* TODO: output address:port for successful binding */
	goto finish;
      }
    }
  }
  
  fprintf(stderr, "no context available for interface '%s'\n", node);

 finish:
  freeaddrinfo(result);
  return ctx;
}

//int
//main(int argc, char **argv) {
int Qlink_StartCoapServer() 
{
  coap_context_t  *ctx;
  fd_set readfds;
  struct timeval tv, *timeout;
  int result;
  coap_tick_t now;
  coap_queue_t *nextpdu;
  char addr_str[NI_MAXHOST] = "::";
  char port_str[NI_MAXSERV] = "5683";
  int opt;
  coap_log_t log_level = LOG_INFO;
  
  printf("start server\n");

  coap_set_log_level(log_level);
  coap_printf("start server coap test debug\n");

  ctx = get_context(addr_str, port_str);
  if (!ctx)
    return -1;

  init_resources(ctx);

//  signal(SIGINT, handle_sigint);

  while ( !g_coapExit ) {
    FD_ZERO(&readfds);
    FD_SET( ctx->sockfd, &readfds );

    nextpdu = coap_peek_next( ctx );

    coap_ticks(&now);
    while (nextpdu && nextpdu->t <= now - ctx->sendqueue_basetime) {
      coap_retransmit( ctx, coap_pop_next( ctx ) );
      nextpdu = coap_peek_next( ctx );
    }

    if ( nextpdu && nextpdu->t <= COAP_RESOURCE_CHECK_TIME ) {
      /* set timeout if there is a pdu to send before our automatic timeout occurs */
      tv.tv_usec = ((nextpdu->t) % COAP_TICKS_PER_SECOND) * 1000000 / COAP_TICKS_PER_SECOND;
      tv.tv_sec = (nextpdu->t) / COAP_TICKS_PER_SECOND;
      timeout = &tv;
    } else {
      tv.tv_usec = 0;
      tv.tv_sec = COAP_RESOURCE_CHECK_TIME;
      timeout = &tv;
    }
    result = select( FD_SETSIZE, &readfds, 0, 0, timeout );

    if ( result < 0 ) {		/* error */
      if (errno != EINTR)
	perror("select");
    } else if ( result > 0 ) {	/* read from socket */
      if ( FD_ISSET( ctx->sockfd, &readfds ) ) {
	coap_read( ctx );	/* read received data */
	coap_dispatch( ctx );	/* and dispatch PDUs from receivequeue */
      }
    } else {			/* timeout */
      if (time_resource) {
	time_resource->dirty = 1;
      }
    }

#ifndef WITHOUT_ASYNC
    /* check if we have to send asynchronous responses */
    check_async(ctx, now);
#endif /* WITHOUT_ASYNC */

#ifndef WITHOUT_OBSERVE
    /* check if we have to send observe notifications */
    coap_check_notify(ctx);
#endif /* WITHOUT_OBSERVE */
  }

  coap_printf("coap server exit!");
  coap_free_context( ctx );
  printf("server end.\n");
  return 0;
}

/* coap-client -- simple CoAP client
 *
 * Copyright (C) 2010--2013 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use. 
 */

#include "config.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "coap.h"
#include "Qlink_API.h"

#define HIDEWIFIESSID         "CMCC-QLINK"
#define HIDEWIFIPASSWD      "sD5fH%9k"

#define COAPSERIP       "coap://192.168.0.100"
#define COAPACK         "/qlink/ack"
#define COAPREQUEST     "/qlink/request"
#define COAPSUCESS      "/qlink/success"
#define PORTSTR         "5683"
#define OBS_SECONFS     30

static const char* version = "1.0.0";
static const char* gatewayIP = "192.168.1.1";
char **responsebuf = NULL;
int* reslen;

int flags = 0;

static unsigned char _token_data[8];
str the_token = { 0, _token_data };

#define FLAGS_BLOCK 0x01

static coap_list_t *optlist = NULL;
/* Request URI.
 * TODO: associate the resources with transaction id and make it expireable */
static coap_uri_t uri;

/* reading is done when this flag is set */
static int ready = 0;

static str payload = { 0, NULL }; /* optional payload to send */

unsigned char msgtype = COAP_MESSAGE_CON; /* usually, requests are sent confirmable */

typedef unsigned char method_t;
method_t method = 1;		/* the method we are using in our requests */

coap_block_t block = { .num = 0, .m = 0, .szx = 6 };

unsigned int wait_seconds = 90;	/* default timeout in seconds */
coap_tick_t max_wait;		/* global timeout (changed by set_timeout()) */

coap_tick_t obs_wait = 0;	/* timeout for current subscription */

#define min(a,b) ((a) < (b) ? (a) : (b))

const char* Qlink_GetVersion()
{
   return version;
}

const char* Qlink_FindGatewayIP()
{
   return gatewayIP;
}

void InitClientData()
{
    responsebuf = NULL;
    flags = 0;
    memset(_token_data, 0, 8);
    the_token.length = 0;
    
    coap_delete_list(optlist);
    optlist = NULL;
    /* reading is done when this flag is set */
    ready = 0;
    
    msgtype = COAP_MESSAGE_CON; /* usually, requests are sent confirmable */
    
    method = 1;		/* the method we are using in our requests */
    
    block.num = 0;
    block.m = 0;
    block.szx = 6;
    
    wait_seconds = 20;	/* default timeout in seconds */
    max_wait = 0;		/* global timeout (changed by set_timeout()) */
    
    obs_wait = 0;	/* timeout for current subscription */
}

static inline void
set_timeout(coap_tick_t *timer, const unsigned int seconds) {
  coap_ticks(timer);
  *timer += seconds * COAP_TICKS_PER_SECOND;
}

coap_pdu_t *
new_ack( coap_context_t  *ctx, coap_queue_t *node ) {
  coap_pdu_t *pdu = coap_new_pdu();

  if (pdu) {
    pdu->hdr->type = COAP_MESSAGE_ACK;
    pdu->hdr->code = 0;
    pdu->hdr->id = node->pdu->hdr->id;
  }

  return pdu;
}

coap_pdu_t *
new_response( coap_context_t  *ctx, coap_queue_t *node, unsigned int code ) {
  coap_pdu_t *pdu = new_ack(ctx, node);

  if (pdu)
    pdu->hdr->code = code;

  return pdu;
}

coap_pdu_t *
coap_new_request(coap_context_t *ctx, method_t m, coap_list_t *options ) {
  coap_pdu_t *pdu;
  coap_list_t *opt;

  if ( ! ( pdu = coap_new_pdu() ) )
    return NULL;

  pdu->hdr->type = msgtype;
  pdu->hdr->id = coap_new_message_id(ctx);
  pdu->hdr->code = m;

  pdu->hdr->token_length = the_token.length;
  if ( !coap_add_token(pdu, the_token.length, the_token.s)) {
    debug("cannot add token to request\n");
  }

  coap_show_pdu(pdu);

  for (opt = options; opt; opt = opt->next) {
    coap_add_option(pdu, COAP_OPTION_KEY(*(coap_option *)opt->data),
		    COAP_OPTION_LENGTH(*(coap_option *)opt->data),
		    COAP_OPTION_DATA(*(coap_option *)opt->data));
  }

  if (payload.length) {
    if ((flags & FLAGS_BLOCK) == 0)
      coap_add_data(pdu, payload.length, payload.s);
    else
      coap_add_block(pdu, payload.length, payload.s, block.num, block.szx);
  }

  return pdu;
}

coap_tid_t
clear_obs(coap_context_t *ctx, const coap_address_t *remote) {
  coap_pdu_t *pdu;
  coap_tid_t tid = COAP_INVALID_TID;

  /* create bare PDU w/o any option  */
  coap_log(LOG_INFO, "response code 7.31 is %d\n", COAP_RESPONSE_CODE(731));
  pdu = coap_pdu_init(msgtype, COAP_RESPONSE_CODE(731), 
		      coap_new_message_id(ctx),
		      sizeof(coap_hdr_t) + the_token.length);

  if (!pdu) {
    return tid;
  }

  if (!coap_add_token(pdu, the_token.length, the_token.s)) {
    coap_log(LOG_CRIT, "cannot add token");
    coap_delete_pdu(pdu);
    return tid;
  }
  coap_show_pdu(pdu);

  if (pdu->hdr->type == COAP_MESSAGE_CON)
    tid = coap_send_confirmed(ctx, remote, pdu);
  else 
    tid = coap_send(ctx, remote, pdu);
    
  if (tid == COAP_INVALID_TID) {
    debug("clear_obs: error sending new request");
    coap_delete_pdu(pdu);
  } else if (pdu->hdr->type != COAP_MESSAGE_CON)
    coap_delete_pdu(pdu);

  return tid;
}

int 
resolve_address(const str *server, struct sockaddr *dst) {
  
  struct addrinfo *res, *ainfo;
  struct addrinfo hints;
  static char addrstr[256];
  int error, len=-1;

  memset(addrstr, 0, sizeof(addrstr));
  if (server->length)
    memcpy(addrstr, server->s, server->length);
  else
    memcpy(addrstr, "localhost", 9);

  memset ((char *)&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_family = AF_UNSPEC;

  error = getaddrinfo(addrstr, "", &hints, &res);

  if (error != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
    return error;
  }

  for (ainfo = res; ainfo != NULL; ainfo = ainfo->ai_next) {
    switch (ainfo->ai_family) {
    case AF_INET6:
    case AF_INET:
      len = ainfo->ai_addrlen;
      memcpy(dst, ainfo->ai_addr, len);
      goto finish;
    default:
      ;
    }
  }

 finish:
  freeaddrinfo(res);
  return len;
}

static inline coap_opt_t *
get_block(coap_pdu_t *pdu, coap_opt_iterator_t *opt_iter) {
  coap_opt_filter_t f;
  
  assert(pdu);

  memset(f, 0, sizeof(coap_opt_filter_t));
  coap_option_setb(f, COAP_OPTION_BLOCK1);
  coap_option_setb(f, COAP_OPTION_BLOCK2);

  coap_option_iterator_init(pdu, opt_iter, f);
  return coap_option_next(opt_iter);
}

#define HANDLE_BLOCK1(Pdu)						\
  ((method == COAP_REQUEST_PUT || method == COAP_REQUEST_POST) &&	\
   ((flags & FLAGS_BLOCK) == 0) &&					\
   ((Pdu)->hdr->code == COAP_RESPONSE_CODE(201) ||			\
    (Pdu)->hdr->code == COAP_RESPONSE_CODE(204)))

inline int
check_token(coap_pdu_t *received) {
  return received->hdr->token_length == the_token.length &&
    memcmp(received->hdr->token, the_token.s, the_token.length) == 0;
}

void
message_handler(struct coap_context_t  *ctx, 
		const coap_address_t *remote, 
		coap_pdu_t *sent,
		coap_pdu_t *received,
		const coap_tid_t id) {

  coap_pdu_t *pdu = NULL;
  coap_opt_t *block_opt;
  coap_opt_iterator_t opt_iter;
  unsigned char buf[4];
  coap_list_t *option;
  size_t len;
  unsigned char *databuf;
  coap_tid_t tid;

#ifndef NDEBUG
  if (LOG_DEBUG <= coap_get_log_level()) {
    debug("** process incoming %d.%02d response:\n",
	  (received->hdr->code >> 5), received->hdr->code & 0x1F);
    coap_show_pdu(received);
  }
#endif

  /* check if this is a response to our original request */
  if (!check_token(received)) {
    /* drop if this was just some message, or send RST in case of notification */
    if (!sent && (received->hdr->type == COAP_MESSAGE_CON || 
		  received->hdr->type == COAP_MESSAGE_NON))
      coap_send_rst(ctx, remote, received);
    return;
  }

  if (received->hdr->type == COAP_MESSAGE_RST) {
    info("got RST\n");
    return;
  }

  /* output the received data, if any */
  if (received->hdr->code == COAP_RESPONSE_CODE(205)) {

    /* set obs timer if we have successfully subscribed a resource */
    if (sent && coap_check_option(received, COAP_OPTION_SUBSCRIPTION, &opt_iter)) {
      debug("observation relationship established, set timeout to %d\n", OBS_SECONFS);
      set_timeout(&obs_wait, OBS_SECONFS);
    }
    
    /* Got some data, check if block option is set. Behavior is undefined if
     * both, Block1 and Block2 are present. */
    block_opt = get_block(received, &opt_iter);
    if (!block_opt) {
      /* There is no block option set, just read the data and we are done. */
      if (coap_get_data(received, &len, &databuf)) {
        /* DONOT FORGET FREE */
        *responsebuf = (char*)coap_malloc(len+1);
        memset(*responsebuf, 0, len+1);
		if (*responsebuf) {
          strncpy(*responsebuf, databuf, len);
          *reslen = len;
        }
      }
    } else {
      unsigned short blktype = opt_iter.type;

      /* TODO: check if we are looking at the correct block number */
      if (coap_get_data(received, &len, &databuf))
          printf("%s:%d:>>>>>>>%s,len:%d\n", __FUNCTION__, __LINE__, databuf, len);

      if (COAP_OPT_BLOCK_MORE(block_opt)) {
	/* more bit is set */
	debug("found the M bit, block size is %u, block nr. %u\n",
	      COAP_OPT_BLOCK_SZX(block_opt), coap_opt_block_num(block_opt));

	/* create pdu with request for next block */
	pdu = coap_new_request(ctx, method, NULL); /* first, create bare PDU w/o any option  */
	if ( pdu ) {
	  /* add URI components from optlist */
	  for (option = optlist; option; option = option->next ) {
	    switch (COAP_OPTION_KEY(*(coap_option *)option->data)) {
	    case COAP_OPTION_URI_HOST :
	    case COAP_OPTION_URI_PORT :
	    case COAP_OPTION_URI_PATH :
	    case COAP_OPTION_URI_QUERY :
	      coap_add_option ( pdu, COAP_OPTION_KEY(*(coap_option *)option->data),
				COAP_OPTION_LENGTH(*(coap_option *)option->data),
				COAP_OPTION_DATA(*(coap_option *)option->data) );
	      break;
	    default:
	      ;			/* skip other options */
	    }
	  }

	  /* finally add updated block option from response, clear M bit */
	  /* blocknr = (blocknr & 0xfffffff7) + 0x10; */
	  debug("query block %d\n", (coap_opt_block_num(block_opt) + 1));
	  coap_add_option(pdu, blktype, coap_encode_var_bytes(buf, 
	      ((coap_opt_block_num(block_opt) + 1) << 4)| 
              COAP_OPT_BLOCK_SZX(block_opt)), buf);

	  if (received->hdr->type == COAP_MESSAGE_CON)
	    tid = coap_send_confirmed(ctx, remote, pdu);
	  else 
	    tid = coap_send(ctx, remote, pdu);

	  if (tid == COAP_INVALID_TID) {
	    debug("message_handler: error sending new request");
            coap_delete_pdu(pdu);
	  } else {
	    set_timeout(&max_wait, wait_seconds);
            if (received->hdr->type != COAP_MESSAGE_CON)
              coap_delete_pdu(pdu);
          }

	  return;
	}
      }
    }
  } else {			/* no 2.05 */

    /* check if an error was signaled and output payload if so */
    if (COAP_RESPONSE_CLASS(received->hdr->code) >= 4) {
      fprintf(stderr, "%d.%02d", 
	      (received->hdr->code >> 5), received->hdr->code & 0x1F);
      if (coap_get_data(received, &len, &databuf)) {
      fprintf(stderr, " ");
	while(len--)
	  fprintf(stderr, "%c", *databuf++);
      }
      fprintf(stderr, "\n");
    }
    
  }

  /* finally send new request, if needed */
  if (pdu && coap_send(ctx, remote, pdu) == COAP_INVALID_TID) {
    debug("message_handler: error sending response");
  }
  coap_delete_pdu(pdu);

  /* our job is done, we can exit at any time */
  ready = coap_check_option(received, COAP_OPTION_SUBSCRIPTION, &opt_iter) == NULL;
}

int
order_opts(void *a, void *b) {
  if (!a || !b)
    return a < b ? -1 : 1;

  if (COAP_OPTION_KEY(*(coap_option *)a) < COAP_OPTION_KEY(*(coap_option *)b))
    return -1;

  return COAP_OPTION_KEY(*(coap_option *)a) == COAP_OPTION_KEY(*(coap_option *)b);
}


coap_list_t *
new_option_node(unsigned short key, unsigned int length, unsigned char *data) {
  coap_option *option;
  coap_list_t *node;

  option = coap_malloc(sizeof(coap_option) + length);
  if ( !option )
    goto error;

  COAP_OPTION_KEY(*option) = key;
  COAP_OPTION_LENGTH(*option) = length;
  memcpy(COAP_OPTION_DATA(*option), data, length);

  /* we can pass NULL here as delete function since option is released automatically  */
  node = coap_new_listnode(option, NULL);

  if ( node )
    return node;

 error:
  perror("new_option_node: malloc");
  coap_free( option );
  return NULL;
}

void
cmdline_uri(char *arg) {
  unsigned char portbuf[2];
#define BUFSIZE 40
  unsigned char _buf[BUFSIZE];
  unsigned char *buf = _buf;
  size_t buflen;
  int res;
  
  /* split arg into Uri-* options */
  coap_split_uri((unsigned char *)arg, strlen(arg), &uri );
    
  if (uri.port != COAP_DEFAULT_PORT) {
    coap_insert( &optlist, 
         new_option_node(COAP_OPTION_URI_PORT,
                 coap_encode_var_bytes(portbuf, uri.port),
               portbuf),
         order_opts);    
  }
    
  if (uri.path.length) {
    buflen = BUFSIZE;
    res = coap_split_path(uri.path.s, uri.path.length, buf, &buflen);
    
    while (res--) {
      coap_insert(&optlist, new_option_node(COAP_OPTION_URI_PATH,
                          COAP_OPT_LENGTH(buf),
                          COAP_OPT_VALUE(buf)),
            order_opts);
    
      buf += COAP_OPT_SIZE(buf);      
    }
  }
    
  if (uri.query.length) {
    buflen = BUFSIZE;
    buf = _buf;
    res = coap_split_query(uri.query.s, uri.query.length, buf, &buflen);
    
    while (res--) {
      coap_insert(&optlist, new_option_node(COAP_OPTION_URI_QUERY,
                          COAP_OPT_LENGTH(buf),
                          COAP_OPT_VALUE(buf)),
          order_opts);
    
      buf += COAP_OPT_SIZE(buf);      
    }
  }
}

/* Called after processing the options from the commandline to set 
 * Block1 or Block2 depending on method. */
void 
set_blocksize() {
  unsigned char buf[4];	/* hack: temporarily take encoded bytes */
  unsigned short opt;

  if (method != COAP_REQUEST_DELETE) {
    opt = method == COAP_REQUEST_GET ? COAP_OPTION_BLOCK2 : COAP_OPTION_BLOCK1;

    coap_insert(&optlist, new_option_node(opt,
                coap_encode_var_bytes(buf, (block.num << 4 | block.szx)), buf),
		order_opts);
  }
}

method_t
cmdline_method(char *arg) {
  static char *methods[] =
    { 0, "get", "post", "put", "delete", 0};
  unsigned char i;

  for (i=1; methods[i] && strcasecmp(arg,methods[i]) != 0 ; ++i)
    ;

  return i;	     /* note that we do not prevent illegal methods */
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
  hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV | AI_ALL;
  
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

int Qlink_SendCoapRequest(const char* url, method_t method_opt, const char *data, char **response, int *len) {
  InitClientData();
  coap_context_t  *ctx = NULL;
  coap_address_t dst;
  char addr[INET6_ADDRSTRLEN];
  void *addrptr = NULL;
  fd_set readfds;
  struct timeval tv;
  int result;
  coap_tick_t now;
  coap_queue_t *nextpdu;
  coap_pdu_t  *pdu;
  str server;
  unsigned short port = COAP_DEFAULT_PORT;
  char port_str[NI_MAXSERV] = "0";
  int res;
  coap_log_t log_level = LOG_INFO;
  coap_tid_t tid = COAP_INVALID_TID;

  coap_set_log_level(log_level);
  
  cmdline_uri((char*)url);
  
  payload.s = (char*)data;
  payload.length = strlen(data);
  method = method_opt;

  server = uri.host;
  port = uri.port;

  responsebuf = response;
  reslen = len;
  /* resolve destination address where server should be sent */
  res = resolve_address(&server, &dst.addr.sa);

  if (res < 0) {
    fprintf(stderr, "failed to resolve address\n");
    exit(-1);
  }

  dst.size = res;
  dst.addr.sin.sin_port = htons(port);

  /* add Uri-Host if server address differs from uri.host */
  
  switch (dst.addr.sa.sa_family) {
  case AF_INET: 
    addrptr = &dst.addr.sin.sin_addr;

    /* create context for IPv4 */
    ctx = get_context("0.0.0.0", port_str);
    break;
  case AF_INET6:
    addrptr = &dst.addr.sin6.sin6_addr;

    /* create context for IPv6 */
    ctx = get_context("::", port_str);
    break;
  default:
    ;
  }

  if (!ctx) {
    coap_log(LOG_EMERG, "cannot create context\n");
    return -1;
  }

  coap_register_option(ctx, COAP_OPTION_BLOCK2);
  coap_register_response_handler(ctx, message_handler);

  /* construct CoAP message */

  if (addrptr
      && (inet_ntop(dst.addr.sa.sa_family, addrptr, addr, sizeof(addr)) != 0)
      && (strlen(addr) != uri.host.length 
	  || memcmp(addr, uri.host.s, uri.host.length) != 0)) {
      /* add Uri-Host */

    coap_insert(&optlist, new_option_node(COAP_OPTION_URI_HOST,
					  uri.host.length, uri.host.s),
		order_opts);
  }

  /* set block option if requested at commandline */
  if (flags & FLAGS_BLOCK)
    set_blocksize();

  if (! (pdu = coap_new_request(ctx, method, optlist)))
    return -1;

#ifndef NDEBUG
  if (LOG_DEBUG <= coap_get_log_level()) {
    debug("sending CoAP request:\n");
    coap_show_pdu(pdu);
  }
#endif

  if (pdu->hdr->type == COAP_MESSAGE_CON)
    tid = coap_send_confirmed(ctx, &dst, pdu);
  else 
    tid = coap_send(ctx, &dst, pdu);

  if (pdu->hdr->type != COAP_MESSAGE_CON || tid == COAP_INVALID_TID)
    coap_delete_pdu(pdu);

  set_timeout(&max_wait, wait_seconds);
  debug("timeout is set to %d seconds\n", wait_seconds);

  while ( !(ready && coap_can_exit(ctx)) ) {
    FD_ZERO(&readfds);
    FD_SET( ctx->sockfd, &readfds );

    nextpdu = coap_peek_next( ctx );

    coap_ticks(&now);
    while (nextpdu && nextpdu->t <= now - ctx->sendqueue_basetime) {
      coap_retransmit( ctx, coap_pop_next( ctx ));
      nextpdu = coap_peek_next( ctx );
    }

    if (nextpdu && nextpdu->t < min(obs_wait ? obs_wait : max_wait, max_wait) - now) { 
      /* set timeout if there is a pdu to send */
      tv.tv_usec = ((nextpdu->t) % COAP_TICKS_PER_SECOND) * 1000000 / COAP_TICKS_PER_SECOND;
      tv.tv_sec = (nextpdu->t) / COAP_TICKS_PER_SECOND;
    } else {
      /* check if obs_wait fires before max_wait */
      if (obs_wait && obs_wait < max_wait) {
	tv.tv_usec = ((obs_wait - now) % COAP_TICKS_PER_SECOND) * 1000000 / COAP_TICKS_PER_SECOND;
	tv.tv_sec = (obs_wait - now) / COAP_TICKS_PER_SECOND;	
      } else {
	tv.tv_usec = ((max_wait - now) % COAP_TICKS_PER_SECOND) * 1000000 / COAP_TICKS_PER_SECOND;
	tv.tv_sec = (max_wait - now) / COAP_TICKS_PER_SECOND;
      }
    }

    result = select(ctx->sockfd + 1, &readfds, 0, 0, &tv);

    if ( result < 0 ) {		/* error */
		if (errno != EINTR)
      perror("select");
    } else if ( result > 0 ) {	/* read from socket */
      if ( FD_ISSET( ctx->sockfd, &readfds ) ) {
	coap_read( ctx );	/* read received data */
    coap_dispatch( ctx );	/* and dispatch PDUs from receivequeue */
    break;
      }
    } else {
      coap_ticks(&now);
      if (max_wait <= now) {
          printf("timeout\n");
	break;
      } 
      if (obs_wait && obs_wait <= now) {
	debug("clear observation relationship\n");
	clear_obs(ctx, &dst); /* FIXME: handle error case COAP_TID_INVALID */

	/* make sure that the obs timer does not fire again */
	obs_wait = 0; 
      } 
    }
  }

  coap_free_context( ctx );

  return 0;
}
static inline int ParseResponse(char *url, char *data)
{
    char *reponse = NULL;
    int replen;
    if (Qlink_SendCoapRequest(url, COAP_REQUEST_POST, data, &reponse, &replen) == -1) {
        if (reponse) free(reponse);
    } else {
        if (reponse) {
            if (strncmp(reponse, "{\"result\":1}", replen) == 0) {
                free(reponse);
                return 1;
            } else {
                free(reponse);
                return 0;
            }
        }
    }
    return -1;
}

int Qlink_notifyGatewayJoinBoot(const char* deviceId, const char* deviceType, const char* ipAddress)
{
  char url[500] = { 0 };
  snprintf(url, 500, "%s:%s%s", COAPSERIP, PORTSTR, COAPREQUEST);

  char data[500] = {0};
  snprintf(data, 500, "{\"deviceId\":\"%s\",\"deviceType\":\"%s\",\"ipAddress\":\"%s\"}", deviceId, deviceType, ipAddress);
  return ParseResponse(url, data);
}

int Qlink_ackQlinkNetinfo(const char* deviceId)
{
  char url[500] = { 0 };
  snprintf(url, 500, "%s:%s%s", COAPSERIP, PORTSTR, COAPSUCESS);

  char data[500] = {0};
  snprintf(data, 500, "{\"deviceId\":\"%s\"}", deviceId);
  return ParseResponse(url, data);
}

const char* Qlink_GetHiddenSSID()
{
    return HIDEWIFIESSID;
}

const char* Qlink_GetHiddenSSIDPWD()
{
    return HIDEWIFIPASSWD;
}

/* header auto-generated by pidl */

#ifndef _PIDL_HEADER_schannel
#define _PIDL_HEADER_schannel

#include <stdint.h>

#include "libcli/util/ntstatus.h"

#include "librpc/gen_ndr/netlogon.h"
#include "librpc/gen_ndr/nbt.h"
#include "librpc/gen_ndr/misc.h"
#include "librpc/gen_ndr/security.h"
#ifndef _HEADER_schannel
#define _HEADER_schannel

#define NL_AUTH_SIGNATURE_SIZE	( 0x20 )
struct netlogon_creds_CredentialState {
	uint32_t negotiate_flags;
	uint8_t session_key[16];
	uint32_t sequence;
	struct netr_Credential seed;
	struct netr_Credential client;
	struct netr_Credential server;
	enum netr_SchannelType secure_channel_type;
	const char *computer_name;/* [charset(UTF8)] */
	const char *account_name;/* [charset(UTF8)] */
	struct dom_sid *sid;/* [unique] */
}/* [public,flag(LIBNDR_PRINT_ARRAY_HEX)] */;

enum NL_AUTH_MESSAGE_TYPE
#ifndef USE_UINT_ENUMS
 {
	NL_NEGOTIATE_REQUEST=(int)(0x00000000),
	NL_NEGOTIATE_RESPONSE=(int)(0x00000001)
}
#else
 { __donnot_use_enum_NL_AUTH_MESSAGE_TYPE=0x7FFFFFFF}
#define NL_NEGOTIATE_REQUEST ( 0x00000000 )
#define NL_NEGOTIATE_RESPONSE ( 0x00000001 )
#endif
;

/* bitmap NL_AUTH_MESSAGE_FLAGS */
#define NL_FLAG_OEM_NETBIOS_DOMAIN_NAME ( 0x00000001 )
#define NL_FLAG_OEM_NETBIOS_COMPUTER_NAME ( 0x00000002 )
#define NL_FLAG_UTF8_DNS_DOMAIN_NAME ( 0x00000004 )
#define NL_FLAG_UTF8_DNS_HOST_NAME ( 0x00000008 )
#define NL_FLAG_UTF8_NETBIOS_COMPUTER_NAME ( 0x00000010 )

union NL_AUTH_MESSAGE_BUFFER {
	const char * a;/* [flag(LIBNDR_FLAG_STR_ASCII|LIBNDR_FLAG_STR_NULLTERM),case(NL_FLAG_OEM_NETBIOS_DOMAIN_NAME)] */
	const char * u;/* [case(NL_FLAG_UTF8_DNS_DOMAIN_NAME)] */
}/* [public,nodiscriminant,noprint] */;

union NL_AUTH_MESSAGE_BUFFER_REPLY {
	uint32_t dummy;/* [case(NL_NEGOTIATE_RESPONSE)] */
}/* [public,nodiscriminant,noprint] */;

struct NL_AUTH_MESSAGE {
	enum NL_AUTH_MESSAGE_TYPE MessageType;
	uint32_t Flags;
	union NL_AUTH_MESSAGE_BUFFER oem_netbios_domain;/* [switch_is(Flags&NL_FLAG_OEM_NETBIOS_DOMAIN_NAME)] */
	union NL_AUTH_MESSAGE_BUFFER oem_netbios_computer;/* [switch_is(Flags&NL_FLAG_OEM_NETBIOS_COMPUTER_NAME)] */
	union NL_AUTH_MESSAGE_BUFFER utf8_dns_domain;/* [switch_is(Flags&NL_FLAG_UTF8_DNS_DOMAIN_NAME)] */
	union NL_AUTH_MESSAGE_BUFFER utf8_dns_host;/* [switch_is(Flags&NL_FLAG_UTF8_DNS_HOST_NAME)] */
	union NL_AUTH_MESSAGE_BUFFER utf8_netbios_computer;/* [switch_is(Flags&NL_FLAG_UTF8_NETBIOS_COMPUTER_NAME)] */
	union NL_AUTH_MESSAGE_BUFFER_REPLY Buffer;/* [switch_is(MessageType&NL_NEGOTIATE_RESPONSE)] */
}/* [flag(LIBNDR_PRINT_ARRAY_HEX),public] */;

enum NL_SIGNATURE_ALGORITHM
#ifndef USE_UINT_ENUMS
 {
	NL_SIGN_HMAC_SHA256=(int)(0x0013),
	NL_SIGN_HMAC_MD5=(int)(0x0077)
}
#else
 { __donnot_use_enum_NL_SIGNATURE_ALGORITHM=0x7FFFFFFF}
#define NL_SIGN_HMAC_SHA256 ( 0x0013 )
#define NL_SIGN_HMAC_MD5 ( 0x0077 )
#endif
;

enum NL_SEAL_ALGORITHM
#ifndef USE_UINT_ENUMS
 {
	NL_SEAL_AES128=(int)(0x001A),
	NL_SEAL_RC4=(int)(0x007A),
	NL_SEAL_NONE=(int)(0xFFFF)
}
#else
 { __donnot_use_enum_NL_SEAL_ALGORITHM=0x7FFFFFFF}
#define NL_SEAL_AES128 ( 0x001A )
#define NL_SEAL_RC4 ( 0x007A )
#define NL_SEAL_NONE ( 0xFFFF )
#endif
;

struct NL_AUTH_SIGNATURE {
	enum NL_SIGNATURE_ALGORITHM SignatureAlgorithm;/* [value(NL_SIGN_HMAC_MD5)] */
	enum NL_SEAL_ALGORITHM SealAlgorithm;
	uint16_t Pad;
	uint16_t Flags;
	uint8_t SequenceNumber[8];
	uint8_t Checksum[8];
	uint8_t Confounder[8];
}/* [flag(LIBNDR_PRINT_ARRAY_HEX),public] */;

struct NL_AUTH_SHA2_SIGNATURE {
	enum NL_SIGNATURE_ALGORITHM SignatureAlgorithm;/* [value(NL_SIGN_HMAC_SHA256)] */
	enum NL_SEAL_ALGORITHM SealAlgorithm;
	uint16_t Pad;
	uint16_t Flags;
	uint8_t SequenceNumber[8];
	uint8_t Checksum[32];
	uint8_t Confounder[8];
}/* [public,flag(LIBNDR_PRINT_ARRAY_HEX)] */;

#endif /* _HEADER_schannel */
#endif /* _PIDL_HEADER_schannel */

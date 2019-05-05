/* header auto-generated by pidl */

#ifndef _PIDL_HEADER_frsapi
#define _PIDL_HEADER_frsapi

#include <stdint.h>

#include "libcli/util/ntstatus.h"

#include "librpc/gen_ndr/misc.h"
#ifndef _HEADER_frsapi
#define _HEADER_frsapi

enum frsapi_InfoEnum
#ifndef USE_UINT_ENUMS
 {
	FRSAPI_INFO_VERSION=(int)(0),
	FRSAPI_INFO_SETS=(int)(1),
	FRSAPI_INFO_DS=(int)(2),
	FRSAPI_INFO_MEMORY=(int)(3),
	FRSAPI_INFO_IDTABLE=(int)(4),
	FRSAPI_INFO_OUTLOG=(int)(5),
	FRSAPI_INFO_INLOG=(int)(6),
	FRSAPI_INFO_THREADS=(int)(7),
	FRSAPI_INFO_STAGE=(int)(8),
	FRSAPI_INFO_CONFIGTABLE=(int)(9)
}
#else
 { __donnot_use_enum_frsapi_InfoEnum=0x7FFFFFFF}
#define FRSAPI_INFO_VERSION ( 0 )
#define FRSAPI_INFO_SETS ( 1 )
#define FRSAPI_INFO_DS ( 2 )
#define FRSAPI_INFO_MEMORY ( 3 )
#define FRSAPI_INFO_IDTABLE ( 4 )
#define FRSAPI_INFO_OUTLOG ( 5 )
#define FRSAPI_INFO_INLOG ( 6 )
#define FRSAPI_INFO_THREADS ( 7 )
#define FRSAPI_INFO_STAGE ( 8 )
#define FRSAPI_INFO_CONFIGTABLE ( 9 )
#endif
;

struct frsapi_Info {
	uint32_t length;
	struct GUID guid;
	uint32_t length2;
	uint32_t unknown1;
	enum frsapi_InfoEnum level;
	uint32_t query_counter;
	uint32_t unknown2;
	uint32_t offset;
	uint32_t blob_len;
	DATA_BLOB blob;/* [flag(LIBNDR_FLAG_REMAINING),subcontext_size(length-offset),subcontext(0)] */
};

enum frsapi_ReplicaSetType
#ifndef USE_UINT_ENUMS
 {
	FRSAPI_REPLICA_SET_TYPE_0=(int)(0x00000000),
	FRSAPI_REPLICA_SET_TYPE_DOMAIN=(int)(0x00000002),
	FRSAPI_REPLICA_SET_TYPE_DFS=(int)(0x00000003)
}
#else
 { __donnot_use_enum_frsapi_ReplicaSetType=0x7FFFFFFF}
#define FRSAPI_REPLICA_SET_TYPE_0 ( 0x00000000 )
#define FRSAPI_REPLICA_SET_TYPE_DOMAIN ( 0x00000002 )
#define FRSAPI_REPLICA_SET_TYPE_DFS ( 0x00000003 )
#endif
;

enum frsapi_WriterCommandsValues
#ifndef USE_UINT_ENUMS
 {
	FRSAPI_WRITER_COMMAND_FREEZE=(int)(0x00000001),
	FRSAPI_WRITER_COMMAND_THAW=(int)(0x00000002)
}
#else
 { __donnot_use_enum_frsapi_WriterCommandsValues=0x7FFFFFFF}
#define FRSAPI_WRITER_COMMAND_FREEZE ( 0x00000001 )
#define FRSAPI_WRITER_COMMAND_THAW ( 0x00000002 )
#endif
;


struct FRSAPI_VERIFY_PROMOTION {
	int _dummy_element;
};


struct FRSAPI_PROMOTION_STATUS {
	int _dummy_element;
};


struct FRSAPI_START_DEMOTION {
	int _dummy_element;
};


struct FRSAPI_COMMIT_DEMOTION {
	int _dummy_element;
};


struct frsapi_SetDsPollingIntervalW {
	struct {
		uint32_t CurrentInterval;
		uint32_t DsPollingLongInterval;
		uint32_t DsPollingShortInterval;
	} in;

	struct {
		WERROR result;
	} out;

};


struct frsapi_GetDsPollingIntervalW {
	struct {
		uint32_t *CurrentInterval;/* [ref] */
		uint32_t *DsPollingLongInterval;/* [ref] */
		uint32_t *DsPollingShortInterval;/* [ref] */
		WERROR result;
	} out;

};


struct FRSAPI_VERIFY_PROMOTION_W {
	int _dummy_element;
};


struct frsapi_InfoW {
	struct {
		uint32_t length;/* [range(0,0x10000)] */
		struct frsapi_Info *info;/* [unique] */
	} in;

	struct {
		struct frsapi_Info *info;/* [unique] */
		WERROR result;
	} out;

};


struct frsapi_IsPathReplicated {
	struct {
		const char *path;/* [unique,charset(UTF16)] */
		enum frsapi_ReplicaSetType replica_set_type;
	} in;

	struct {
		uint32_t *replicated;/* [ref] */
		uint32_t *primary;/* [ref] */
		uint32_t *root;/* [ref] */
		struct GUID *replica_set_guid;/* [ref] */
		WERROR result;
	} out;

};


struct frsapi_WriterCommand {
	struct {
		enum frsapi_WriterCommandsValues command;
	} in;

	struct {
		WERROR result;
	} out;

};


struct frsapi_ForceReplication {
	struct {
		struct GUID *replica_set_guid;/* [unique] */
		struct GUID *connection_guid;/* [unique] */
		const char *replica_set_name;/* [unique,charset(UTF16)] */
		const char *partner_dns_name;/* [charset(UTF16),unique] */
	} in;

	struct {
		WERROR result;
	} out;

};

#endif /* _HEADER_frsapi */
#endif /* _PIDL_HEADER_frsapi */

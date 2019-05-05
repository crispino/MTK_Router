/* server functions auto-generated by pidl */
#include "autoconf/librpc/gen_ndr/ndr_drsblobs.h"

NTSTATUS dcerpc_server_drsblobs_init(void);

/* drsblobs - dcerpc server boilerplate generated by pidl */


static NTSTATUS drsblobs__op_bind(struct dcesrv_call_state *dce_call, const struct dcesrv_interface *iface, uint32_t if_version)
{
#ifdef DCESRV_INTERFACE_DRSBLOBS_BIND
	return DCESRV_INTERFACE_DRSBLOBS_BIND(dce_call,iface);
#else
	return NT_STATUS_OK;
#endif
}

static void drsblobs__op_unbind(struct dcesrv_connection_context *context, const struct dcesrv_interface *iface)
{
#ifdef DCESRV_INTERFACE_DRSBLOBS_UNBIND
	DCESRV_INTERFACE_DRSBLOBS_UNBIND(context, iface);
#else
	return;
#endif
}

static NTSTATUS drsblobs__op_ndr_pull(struct dcesrv_call_state *dce_call, TALLOC_CTX *mem_ctx, struct ndr_pull *pull, void **r)
{
	enum ndr_err_code ndr_err;
	uint16_t opnum = dce_call->pkt.u.request.opnum;

	dce_call->fault_code = 0;

	if (opnum >= ndr_table_drsblobs.num_calls) {
		dce_call->fault_code = DCERPC_FAULT_OP_RNG_ERROR;
		return NT_STATUS_NET_WRITE_FAULT;
	}

	*r = talloc_named(mem_ctx,
			  ndr_table_drsblobs.calls[opnum].struct_size,
			  "struct %s",
			  ndr_table_drsblobs.calls[opnum].name);
	NT_STATUS_HAVE_NO_MEMORY(*r);

        /* unravel the NDR for the packet */
	ndr_err = ndr_table_drsblobs.calls[opnum].ndr_pull(pull, NDR_IN, *r);
	if (!NDR_ERR_CODE_IS_SUCCESS(ndr_err)) {
		dcerpc_log_packet(dce_call->conn->packet_log_dir, 
				  &ndr_table_drsblobs, opnum, NDR_IN,
				  &dce_call->pkt.u.request.stub_and_verifier);
		dce_call->fault_code = DCERPC_FAULT_NDR;
		return NT_STATUS_NET_WRITE_FAULT;
	}

	return NT_STATUS_OK;
}

static NTSTATUS drsblobs__op_dispatch(struct dcesrv_call_state *dce_call, TALLOC_CTX *mem_ctx, void *r)
{
	uint16_t opnum = dce_call->pkt.u.request.opnum;

	switch (opnum) {
	case 0: {
		struct decode_replPropertyMetaData *r2 = (struct decode_replPropertyMetaData *)r;
		if (DEBUGLEVEL >= 10) {
			NDR_PRINT_FUNCTION_DEBUG(decode_replPropertyMetaData, NDR_IN, r2);
		}
		dcesrv_decode_replPropertyMetaData(dce_call, mem_ctx, r2);
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_replPropertyMetaData will reply async\n"));
		}
		break;
	}
	case 1: {
		struct decode_replUpToDateVector *r2 = (struct decode_replUpToDateVector *)r;
		if (DEBUGLEVEL >= 10) {
			NDR_PRINT_FUNCTION_DEBUG(decode_replUpToDateVector, NDR_IN, r2);
		}
		dcesrv_decode_replUpToDateVector(dce_call, mem_ctx, r2);
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_replUpToDateVector will reply async\n"));
		}
		break;
	}
	case 2: {
		struct decode_repsFromTo *r2 = (struct decode_repsFromTo *)r;
		if (DEBUGLEVEL >= 10) {
			NDR_PRINT_FUNCTION_DEBUG(decode_repsFromTo, NDR_IN, r2);
		}
		dcesrv_decode_repsFromTo(dce_call, mem_ctx, r2);
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_repsFromTo will reply async\n"));
		}
		break;
	}
	case 3: {
		struct decode_partialAttributeSet *r2 = (struct decode_partialAttributeSet *)r;
		if (DEBUGLEVEL >= 10) {
			NDR_PRINT_FUNCTION_DEBUG(decode_partialAttributeSet, NDR_IN, r2);
		}
		dcesrv_decode_partialAttributeSet(dce_call, mem_ctx, r2);
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_partialAttributeSet will reply async\n"));
		}
		break;
	}
	case 4: {
		struct decode_prefixMap *r2 = (struct decode_prefixMap *)r;
		if (DEBUGLEVEL >= 10) {
			NDR_PRINT_FUNCTION_DEBUG(decode_prefixMap, NDR_IN, r2);
		}
		dcesrv_decode_prefixMap(dce_call, mem_ctx, r2);
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_prefixMap will reply async\n"));
		}
		break;
	}
	case 5: {
		struct decode_ldapControlDirSync *r2 = (struct decode_ldapControlDirSync *)r;
		if (DEBUGLEVEL >= 10) {
			NDR_PRINT_FUNCTION_DEBUG(decode_ldapControlDirSync, NDR_IN, r2);
		}
		dcesrv_decode_ldapControlDirSync(dce_call, mem_ctx, r2);
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_ldapControlDirSync will reply async\n"));
		}
		break;
	}
	case 6: {
		struct decode_supplementalCredentials *r2 = (struct decode_supplementalCredentials *)r;
		if (DEBUGLEVEL >= 10) {
			NDR_PRINT_FUNCTION_DEBUG(decode_supplementalCredentials, NDR_IN, r2);
		}
		dcesrv_decode_supplementalCredentials(dce_call, mem_ctx, r2);
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_supplementalCredentials will reply async\n"));
		}
		break;
	}
	case 7: {
		struct decode_Packages *r2 = (struct decode_Packages *)r;
		if (DEBUGLEVEL >= 10) {
			NDR_PRINT_FUNCTION_DEBUG(decode_Packages, NDR_IN, r2);
		}
		dcesrv_decode_Packages(dce_call, mem_ctx, r2);
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_Packages will reply async\n"));
		}
		break;
	}
	case 8: {
		struct decode_PrimaryKerberos *r2 = (struct decode_PrimaryKerberos *)r;
		if (DEBUGLEVEL >= 10) {
			NDR_PRINT_FUNCTION_DEBUG(decode_PrimaryKerberos, NDR_IN, r2);
		}
		dcesrv_decode_PrimaryKerberos(dce_call, mem_ctx, r2);
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_PrimaryKerberos will reply async\n"));
		}
		break;
	}
	case 9: {
		struct decode_PrimaryCLEARTEXT *r2 = (struct decode_PrimaryCLEARTEXT *)r;
		if (DEBUGLEVEL >= 10) {
			NDR_PRINT_FUNCTION_DEBUG(decode_PrimaryCLEARTEXT, NDR_IN, r2);
		}
		dcesrv_decode_PrimaryCLEARTEXT(dce_call, mem_ctx, r2);
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_PrimaryCLEARTEXT will reply async\n"));
		}
		break;
	}
	case 10: {
		struct decode_PrimaryWDigest *r2 = (struct decode_PrimaryWDigest *)r;
		if (DEBUGLEVEL >= 10) {
			NDR_PRINT_FUNCTION_DEBUG(decode_PrimaryWDigest, NDR_IN, r2);
		}
		dcesrv_decode_PrimaryWDigest(dce_call, mem_ctx, r2);
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_PrimaryWDigest will reply async\n"));
		}
		break;
	}
	case 11: {
		struct decode_trustAuthInOut *r2 = (struct decode_trustAuthInOut *)r;
		if (DEBUGLEVEL >= 10) {
			NDR_PRINT_FUNCTION_DEBUG(decode_trustAuthInOut, NDR_IN, r2);
		}
		dcesrv_decode_trustAuthInOut(dce_call, mem_ctx, r2);
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_trustAuthInOut will reply async\n"));
		}
		break;
	}
	case 12: {
		struct decode_trustDomainPasswords *r2 = (struct decode_trustDomainPasswords *)r;
		if (DEBUGLEVEL >= 10) {
			NDR_PRINT_FUNCTION_DEBUG(decode_trustDomainPasswords, NDR_IN, r2);
		}
		dcesrv_decode_trustDomainPasswords(dce_call, mem_ctx, r2);
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_trustDomainPasswords will reply async\n"));
		}
		break;
	}
	case 13: {
		struct decode_ExtendedErrorInfo *r2 = (struct decode_ExtendedErrorInfo *)r;
		if (DEBUGLEVEL >= 10) {
			NDR_PRINT_FUNCTION_DEBUG(decode_ExtendedErrorInfo, NDR_IN, r2);
		}
		dcesrv_decode_ExtendedErrorInfo(dce_call, mem_ctx, r2);
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_ExtendedErrorInfo will reply async\n"));
		}
		break;
	}
	case 14: {
		struct decode_ForestTrustInfo *r2 = (struct decode_ForestTrustInfo *)r;
		if (DEBUGLEVEL >= 10) {
			NDR_PRINT_FUNCTION_DEBUG(decode_ForestTrustInfo, NDR_IN, r2);
		}
		dcesrv_decode_ForestTrustInfo(dce_call, mem_ctx, r2);
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_ForestTrustInfo will reply async\n"));
		}
		break;
	}

	default:
		dce_call->fault_code = DCERPC_FAULT_OP_RNG_ERROR;
		break;
	}

	if (dce_call->fault_code != 0) {
		dcerpc_log_packet(dce_call->conn->packet_log_dir, 
		          &ndr_table_drsblobs, opnum, NDR_IN,
				  &dce_call->pkt.u.request.stub_and_verifier);
		return NT_STATUS_NET_WRITE_FAULT;
	}

	return NT_STATUS_OK;
}

static NTSTATUS drsblobs__op_reply(struct dcesrv_call_state *dce_call, TALLOC_CTX *mem_ctx, void *r)
{
	uint16_t opnum = dce_call->pkt.u.request.opnum;

	switch (opnum) {
	case 0: {
		struct decode_replPropertyMetaData *r2 = (struct decode_replPropertyMetaData *)r;
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_replPropertyMetaData replied async\n"));
		}
		if (DEBUGLEVEL >= 10 && dce_call->fault_code == 0) {
			NDR_PRINT_FUNCTION_DEBUG(decode_replPropertyMetaData, NDR_OUT | NDR_SET_VALUES, r2);
		}
		if (dce_call->fault_code != 0) {
			DEBUG(2,("dcerpc_fault %s in decode_replPropertyMetaData\n", dcerpc_errstr(mem_ctx, dce_call->fault_code)));
		}
		break;
	}
	case 1: {
		struct decode_replUpToDateVector *r2 = (struct decode_replUpToDateVector *)r;
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_replUpToDateVector replied async\n"));
		}
		if (DEBUGLEVEL >= 10 && dce_call->fault_code == 0) {
			NDR_PRINT_FUNCTION_DEBUG(decode_replUpToDateVector, NDR_OUT | NDR_SET_VALUES, r2);
		}
		if (dce_call->fault_code != 0) {
			DEBUG(2,("dcerpc_fault %s in decode_replUpToDateVector\n", dcerpc_errstr(mem_ctx, dce_call->fault_code)));
		}
		break;
	}
	case 2: {
		struct decode_repsFromTo *r2 = (struct decode_repsFromTo *)r;
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_repsFromTo replied async\n"));
		}
		if (DEBUGLEVEL >= 10 && dce_call->fault_code == 0) {
			NDR_PRINT_FUNCTION_DEBUG(decode_repsFromTo, NDR_OUT | NDR_SET_VALUES, r2);
		}
		if (dce_call->fault_code != 0) {
			DEBUG(2,("dcerpc_fault %s in decode_repsFromTo\n", dcerpc_errstr(mem_ctx, dce_call->fault_code)));
		}
		break;
	}
	case 3: {
		struct decode_partialAttributeSet *r2 = (struct decode_partialAttributeSet *)r;
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_partialAttributeSet replied async\n"));
		}
		if (DEBUGLEVEL >= 10 && dce_call->fault_code == 0) {
			NDR_PRINT_FUNCTION_DEBUG(decode_partialAttributeSet, NDR_OUT | NDR_SET_VALUES, r2);
		}
		if (dce_call->fault_code != 0) {
			DEBUG(2,("dcerpc_fault %s in decode_partialAttributeSet\n", dcerpc_errstr(mem_ctx, dce_call->fault_code)));
		}
		break;
	}
	case 4: {
		struct decode_prefixMap *r2 = (struct decode_prefixMap *)r;
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_prefixMap replied async\n"));
		}
		if (DEBUGLEVEL >= 10 && dce_call->fault_code == 0) {
			NDR_PRINT_FUNCTION_DEBUG(decode_prefixMap, NDR_OUT | NDR_SET_VALUES, r2);
		}
		if (dce_call->fault_code != 0) {
			DEBUG(2,("dcerpc_fault %s in decode_prefixMap\n", dcerpc_errstr(mem_ctx, dce_call->fault_code)));
		}
		break;
	}
	case 5: {
		struct decode_ldapControlDirSync *r2 = (struct decode_ldapControlDirSync *)r;
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_ldapControlDirSync replied async\n"));
		}
		if (DEBUGLEVEL >= 10 && dce_call->fault_code == 0) {
			NDR_PRINT_FUNCTION_DEBUG(decode_ldapControlDirSync, NDR_OUT | NDR_SET_VALUES, r2);
		}
		if (dce_call->fault_code != 0) {
			DEBUG(2,("dcerpc_fault %s in decode_ldapControlDirSync\n", dcerpc_errstr(mem_ctx, dce_call->fault_code)));
		}
		break;
	}
	case 6: {
		struct decode_supplementalCredentials *r2 = (struct decode_supplementalCredentials *)r;
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_supplementalCredentials replied async\n"));
		}
		if (DEBUGLEVEL >= 10 && dce_call->fault_code == 0) {
			NDR_PRINT_FUNCTION_DEBUG(decode_supplementalCredentials, NDR_OUT | NDR_SET_VALUES, r2);
		}
		if (dce_call->fault_code != 0) {
			DEBUG(2,("dcerpc_fault %s in decode_supplementalCredentials\n", dcerpc_errstr(mem_ctx, dce_call->fault_code)));
		}
		break;
	}
	case 7: {
		struct decode_Packages *r2 = (struct decode_Packages *)r;
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_Packages replied async\n"));
		}
		if (DEBUGLEVEL >= 10 && dce_call->fault_code == 0) {
			NDR_PRINT_FUNCTION_DEBUG(decode_Packages, NDR_OUT | NDR_SET_VALUES, r2);
		}
		if (dce_call->fault_code != 0) {
			DEBUG(2,("dcerpc_fault %s in decode_Packages\n", dcerpc_errstr(mem_ctx, dce_call->fault_code)));
		}
		break;
	}
	case 8: {
		struct decode_PrimaryKerberos *r2 = (struct decode_PrimaryKerberos *)r;
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_PrimaryKerberos replied async\n"));
		}
		if (DEBUGLEVEL >= 10 && dce_call->fault_code == 0) {
			NDR_PRINT_FUNCTION_DEBUG(decode_PrimaryKerberos, NDR_OUT | NDR_SET_VALUES, r2);
		}
		if (dce_call->fault_code != 0) {
			DEBUG(2,("dcerpc_fault %s in decode_PrimaryKerberos\n", dcerpc_errstr(mem_ctx, dce_call->fault_code)));
		}
		break;
	}
	case 9: {
		struct decode_PrimaryCLEARTEXT *r2 = (struct decode_PrimaryCLEARTEXT *)r;
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_PrimaryCLEARTEXT replied async\n"));
		}
		if (DEBUGLEVEL >= 10 && dce_call->fault_code == 0) {
			NDR_PRINT_FUNCTION_DEBUG(decode_PrimaryCLEARTEXT, NDR_OUT | NDR_SET_VALUES, r2);
		}
		if (dce_call->fault_code != 0) {
			DEBUG(2,("dcerpc_fault %s in decode_PrimaryCLEARTEXT\n", dcerpc_errstr(mem_ctx, dce_call->fault_code)));
		}
		break;
	}
	case 10: {
		struct decode_PrimaryWDigest *r2 = (struct decode_PrimaryWDigest *)r;
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_PrimaryWDigest replied async\n"));
		}
		if (DEBUGLEVEL >= 10 && dce_call->fault_code == 0) {
			NDR_PRINT_FUNCTION_DEBUG(decode_PrimaryWDigest, NDR_OUT | NDR_SET_VALUES, r2);
		}
		if (dce_call->fault_code != 0) {
			DEBUG(2,("dcerpc_fault %s in decode_PrimaryWDigest\n", dcerpc_errstr(mem_ctx, dce_call->fault_code)));
		}
		break;
	}
	case 11: {
		struct decode_trustAuthInOut *r2 = (struct decode_trustAuthInOut *)r;
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_trustAuthInOut replied async\n"));
		}
		if (DEBUGLEVEL >= 10 && dce_call->fault_code == 0) {
			NDR_PRINT_FUNCTION_DEBUG(decode_trustAuthInOut, NDR_OUT | NDR_SET_VALUES, r2);
		}
		if (dce_call->fault_code != 0) {
			DEBUG(2,("dcerpc_fault %s in decode_trustAuthInOut\n", dcerpc_errstr(mem_ctx, dce_call->fault_code)));
		}
		break;
	}
	case 12: {
		struct decode_trustDomainPasswords *r2 = (struct decode_trustDomainPasswords *)r;
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_trustDomainPasswords replied async\n"));
		}
		if (DEBUGLEVEL >= 10 && dce_call->fault_code == 0) {
			NDR_PRINT_FUNCTION_DEBUG(decode_trustDomainPasswords, NDR_OUT | NDR_SET_VALUES, r2);
		}
		if (dce_call->fault_code != 0) {
			DEBUG(2,("dcerpc_fault %s in decode_trustDomainPasswords\n", dcerpc_errstr(mem_ctx, dce_call->fault_code)));
		}
		break;
	}
	case 13: {
		struct decode_ExtendedErrorInfo *r2 = (struct decode_ExtendedErrorInfo *)r;
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_ExtendedErrorInfo replied async\n"));
		}
		if (DEBUGLEVEL >= 10 && dce_call->fault_code == 0) {
			NDR_PRINT_FUNCTION_DEBUG(decode_ExtendedErrorInfo, NDR_OUT | NDR_SET_VALUES, r2);
		}
		if (dce_call->fault_code != 0) {
			DEBUG(2,("dcerpc_fault %s in decode_ExtendedErrorInfo\n", dcerpc_errstr(mem_ctx, dce_call->fault_code)));
		}
		break;
	}
	case 14: {
		struct decode_ForestTrustInfo *r2 = (struct decode_ForestTrustInfo *)r;
		if (dce_call->state_flags & DCESRV_CALL_STATE_FLAG_ASYNC) {
			DEBUG(5,("function decode_ForestTrustInfo replied async\n"));
		}
		if (DEBUGLEVEL >= 10 && dce_call->fault_code == 0) {
			NDR_PRINT_FUNCTION_DEBUG(decode_ForestTrustInfo, NDR_OUT | NDR_SET_VALUES, r2);
		}
		if (dce_call->fault_code != 0) {
			DEBUG(2,("dcerpc_fault %s in decode_ForestTrustInfo\n", dcerpc_errstr(mem_ctx, dce_call->fault_code)));
		}
		break;
	}

	default:
		dce_call->fault_code = DCERPC_FAULT_OP_RNG_ERROR;
		break;
	}

	if (dce_call->fault_code != 0) {
		dcerpc_log_packet(dce_call->conn->packet_log_dir,
		          &ndr_table_drsblobs, opnum, NDR_IN,
				  &dce_call->pkt.u.request.stub_and_verifier);
		return NT_STATUS_NET_WRITE_FAULT;
	}

	return NT_STATUS_OK;
}

static NTSTATUS drsblobs__op_ndr_push(struct dcesrv_call_state *dce_call, TALLOC_CTX *mem_ctx, struct ndr_push *push, const void *r)
{
	enum ndr_err_code ndr_err;
	uint16_t opnum = dce_call->pkt.u.request.opnum;

	ndr_err = ndr_table_drsblobs.calls[opnum].ndr_push(push, NDR_OUT, r);
	if (!NDR_ERR_CODE_IS_SUCCESS(ndr_err)) {
		dce_call->fault_code = DCERPC_FAULT_NDR;
		return NT_STATUS_NET_WRITE_FAULT;
	}

	return NT_STATUS_OK;
}

const struct dcesrv_interface dcesrv_drsblobs_interface = {
	.name		= "drsblobs",
	.syntax_id  = {{0x12345778,0x1234,0xabcd,{0x00,0x01},{0x00,0x00,0x00,0x01}},0.0},
	.bind		= drsblobs__op_bind,
	.unbind		= drsblobs__op_unbind,
	.ndr_pull	= drsblobs__op_ndr_pull,
	.dispatch	= drsblobs__op_dispatch,
	.reply		= drsblobs__op_reply,
	.ndr_push	= drsblobs__op_ndr_push
};


static NTSTATUS drsblobs__op_init_server(struct dcesrv_context *dce_ctx, const struct dcesrv_endpoint_server *ep_server)
{
	int i;

	for (i=0;i<ndr_table_drsblobs.endpoints->count;i++) {
		NTSTATUS ret;
		const char *name = ndr_table_drsblobs.endpoints->names[i];

		ret = dcesrv_interface_register(dce_ctx, name, &dcesrv_drsblobs_interface, NULL);
		if (!NT_STATUS_IS_OK(ret)) {
			DEBUG(1,("drsblobs_op_init_server: failed to register endpoint '%s'\n",name));
			return ret;
		}
	}

	return NT_STATUS_OK;
}

static bool drsblobs__op_interface_by_uuid(struct dcesrv_interface *iface, const struct GUID *uuid, uint32_t if_version)
{
	if (dcesrv_drsblobs_interface.syntax_id.if_version == if_version &&
		GUID_equal(&dcesrv_drsblobs_interface.syntax_id.uuid, uuid)) {
		memcpy(iface,&dcesrv_drsblobs_interface, sizeof(*iface));
		return true;
	}

	return false;
}

static bool drsblobs__op_interface_by_name(struct dcesrv_interface *iface, const char *name)
{
	if (strcmp(dcesrv_drsblobs_interface.name, name)==0) {
		memcpy(iface, &dcesrv_drsblobs_interface, sizeof(*iface));
		return true;
	}

	return false;
}

NTSTATUS dcerpc_server_drsblobs_init(void)
{
	NTSTATUS ret;
	struct dcesrv_endpoint_server ep_server;

	/* fill in our name */
	ep_server.name = "drsblobs";

	/* fill in all the operations */
	ep_server.init_server = drsblobs__op_init_server;

	ep_server.interface_by_uuid = drsblobs__op_interface_by_uuid;
	ep_server.interface_by_name = drsblobs__op_interface_by_name;

	/* register ourselves with the DCERPC subsystem. */
	ret = dcerpc_register_ep_server(&ep_server);

	if (!NT_STATUS_IS_OK(ret)) {
		DEBUG(0,("Failed to register 'drsblobs' endpoint server!\n"));
		return ret;
	}

	return ret;
}

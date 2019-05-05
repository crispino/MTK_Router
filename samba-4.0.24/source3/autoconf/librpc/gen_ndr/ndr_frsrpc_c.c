/* client functions auto-generated by pidl */

#include "includes.h"
#include <tevent.h>
#include "lib/util/tevent_ntstatus.h"
#include "autoconf/librpc/gen_ndr/ndr_frsrpc.h"
#include "autoconf/librpc/gen_ndr/ndr_frsrpc_c.h"

/* frsrpc - client functions generated by pidl */

struct dcerpc_frsrpc_FrsSendCommPkt_r_state {
	TALLOC_CTX *out_mem_ctx;
};

static void dcerpc_frsrpc_FrsSendCommPkt_r_done(struct tevent_req *subreq);

struct tevent_req *dcerpc_frsrpc_FrsSendCommPkt_r_send(TALLOC_CTX *mem_ctx,
	struct tevent_context *ev,
	struct dcerpc_binding_handle *h,
	struct frsrpc_FrsSendCommPkt *r)
{
	struct tevent_req *req;
	struct dcerpc_frsrpc_FrsSendCommPkt_r_state *state;
	struct tevent_req *subreq;

	req = tevent_req_create(mem_ctx, &state,
				struct dcerpc_frsrpc_FrsSendCommPkt_r_state);
	if (req == NULL) {
		return NULL;
	}

	state->out_mem_ctx = NULL;

	subreq = dcerpc_binding_handle_call_send(state, ev, h,
			NULL, &ndr_table_frsrpc,
			NDR_FRSRPC_FRSSENDCOMMPKT, state, r);
	if (tevent_req_nomem(subreq, req)) {
		return tevent_req_post(req, ev);
	}
	tevent_req_set_callback(subreq, dcerpc_frsrpc_FrsSendCommPkt_r_done, req);

	return req;
}

static void dcerpc_frsrpc_FrsSendCommPkt_r_done(struct tevent_req *subreq)
{
	struct tevent_req *req =
		tevent_req_callback_data(subreq,
		struct tevent_req);
	NTSTATUS status;

	status = dcerpc_binding_handle_call_recv(subreq);
	if (!NT_STATUS_IS_OK(status)) {
		tevent_req_nterror(req, status);
		return;
	}

	tevent_req_done(req);
}

NTSTATUS dcerpc_frsrpc_FrsSendCommPkt_r_recv(struct tevent_req *req, TALLOC_CTX *mem_ctx)
{
	struct dcerpc_frsrpc_FrsSendCommPkt_r_state *state =
		tevent_req_data(req,
		struct dcerpc_frsrpc_FrsSendCommPkt_r_state);
	NTSTATUS status;

	if (tevent_req_is_nterror(req, &status)) {
		tevent_req_received(req);
		return status;
	}

	talloc_steal(mem_ctx, state->out_mem_ctx);

	tevent_req_received(req);
	return NT_STATUS_OK;
}

NTSTATUS dcerpc_frsrpc_FrsSendCommPkt_r(struct dcerpc_binding_handle *h, TALLOC_CTX *mem_ctx, struct frsrpc_FrsSendCommPkt *r)
{
	NTSTATUS status;

	status = dcerpc_binding_handle_call(h,
			NULL, &ndr_table_frsrpc,
			NDR_FRSRPC_FRSSENDCOMMPKT, mem_ctx, r);

	return status;
}

struct dcerpc_frsrpc_FrsSendCommPkt_state {
	struct frsrpc_FrsSendCommPkt orig;
	struct frsrpc_FrsSendCommPkt tmp;
	TALLOC_CTX *out_mem_ctx;
};

static void dcerpc_frsrpc_FrsSendCommPkt_done(struct tevent_req *subreq);

struct tevent_req *dcerpc_frsrpc_FrsSendCommPkt_send(TALLOC_CTX *mem_ctx,
						     struct tevent_context *ev,
						     struct dcerpc_binding_handle *h,
						     struct frsrpc_FrsSendCommPktReq _req /* [in]  */)
{
	struct tevent_req *req;
	struct dcerpc_frsrpc_FrsSendCommPkt_state *state;
	struct tevent_req *subreq;

	req = tevent_req_create(mem_ctx, &state,
				struct dcerpc_frsrpc_FrsSendCommPkt_state);
	if (req == NULL) {
		return NULL;
	}
	state->out_mem_ctx = NULL;

	/* In parameters */
	state->orig.in.req = _req;

	/* Out parameters */

	/* Result */
	ZERO_STRUCT(state->orig.out.result);

	/* make a temporary copy, that we pass to the dispatch function */
	state->tmp = state->orig;

	subreq = dcerpc_frsrpc_FrsSendCommPkt_r_send(state, ev, h, &state->tmp);
	if (tevent_req_nomem(subreq, req)) {
		return tevent_req_post(req, ev);
	}
	tevent_req_set_callback(subreq, dcerpc_frsrpc_FrsSendCommPkt_done, req);
	return req;
}

static void dcerpc_frsrpc_FrsSendCommPkt_done(struct tevent_req *subreq)
{
	struct tevent_req *req = tevent_req_callback_data(
		subreq, struct tevent_req);
	struct dcerpc_frsrpc_FrsSendCommPkt_state *state = tevent_req_data(
		req, struct dcerpc_frsrpc_FrsSendCommPkt_state);
	NTSTATUS status;
	TALLOC_CTX *mem_ctx;

	if (state->out_mem_ctx) {
		mem_ctx = state->out_mem_ctx;
	} else {
		mem_ctx = state;
	}

	status = dcerpc_frsrpc_FrsSendCommPkt_r_recv(subreq, mem_ctx);
	TALLOC_FREE(subreq);
	if (!NT_STATUS_IS_OK(status)) {
		tevent_req_nterror(req, status);
		return;
	}

	/* Copy out parameters */

	/* Copy result */
	state->orig.out.result = state->tmp.out.result;

	/* Reset temporary structure */
	ZERO_STRUCT(state->tmp);

	tevent_req_done(req);
}

NTSTATUS dcerpc_frsrpc_FrsSendCommPkt_recv(struct tevent_req *req,
					   TALLOC_CTX *mem_ctx,
					   WERROR *result)
{
	struct dcerpc_frsrpc_FrsSendCommPkt_state *state = tevent_req_data(
		req, struct dcerpc_frsrpc_FrsSendCommPkt_state);
	NTSTATUS status;

	if (tevent_req_is_nterror(req, &status)) {
		tevent_req_received(req);
		return status;
	}

	/* Steal possible out parameters to the callers context */
	talloc_steal(mem_ctx, state->out_mem_ctx);

	/* Return result */
	*result = state->orig.out.result;

	tevent_req_received(req);
	return NT_STATUS_OK;
}

NTSTATUS dcerpc_frsrpc_FrsSendCommPkt(struct dcerpc_binding_handle *h,
				      TALLOC_CTX *mem_ctx,
				      struct frsrpc_FrsSendCommPktReq _req /* [in]  */,
				      WERROR *result)
{
	struct frsrpc_FrsSendCommPkt r;
	NTSTATUS status;

	/* In parameters */
	r.in.req = _req;

	/* Out parameters */

	/* Result */
	ZERO_STRUCT(r.out.result);

	status = dcerpc_frsrpc_FrsSendCommPkt_r(h, mem_ctx, &r);
	if (!NT_STATUS_IS_OK(status)) {
		return status;
	}

	/* Return variables */

	/* Return result */
	*result = r.out.result;

	return NT_STATUS_OK;
}

struct dcerpc_frsrpc_FrsVerifyPromotionParent_r_state {
	TALLOC_CTX *out_mem_ctx;
};

static void dcerpc_frsrpc_FrsVerifyPromotionParent_r_done(struct tevent_req *subreq);

struct tevent_req *dcerpc_frsrpc_FrsVerifyPromotionParent_r_send(TALLOC_CTX *mem_ctx,
	struct tevent_context *ev,
	struct dcerpc_binding_handle *h,
	struct frsrpc_FrsVerifyPromotionParent *r)
{
	struct tevent_req *req;
	struct dcerpc_frsrpc_FrsVerifyPromotionParent_r_state *state;
	struct tevent_req *subreq;

	req = tevent_req_create(mem_ctx, &state,
				struct dcerpc_frsrpc_FrsVerifyPromotionParent_r_state);
	if (req == NULL) {
		return NULL;
	}

	state->out_mem_ctx = NULL;

	subreq = dcerpc_binding_handle_call_send(state, ev, h,
			NULL, &ndr_table_frsrpc,
			NDR_FRSRPC_FRSVERIFYPROMOTIONPARENT, state, r);
	if (tevent_req_nomem(subreq, req)) {
		return tevent_req_post(req, ev);
	}
	tevent_req_set_callback(subreq, dcerpc_frsrpc_FrsVerifyPromotionParent_r_done, req);

	return req;
}

static void dcerpc_frsrpc_FrsVerifyPromotionParent_r_done(struct tevent_req *subreq)
{
	struct tevent_req *req =
		tevent_req_callback_data(subreq,
		struct tevent_req);
	NTSTATUS status;

	status = dcerpc_binding_handle_call_recv(subreq);
	if (!NT_STATUS_IS_OK(status)) {
		tevent_req_nterror(req, status);
		return;
	}

	tevent_req_done(req);
}

NTSTATUS dcerpc_frsrpc_FrsVerifyPromotionParent_r_recv(struct tevent_req *req, TALLOC_CTX *mem_ctx)
{
	struct dcerpc_frsrpc_FrsVerifyPromotionParent_r_state *state =
		tevent_req_data(req,
		struct dcerpc_frsrpc_FrsVerifyPromotionParent_r_state);
	NTSTATUS status;

	if (tevent_req_is_nterror(req, &status)) {
		tevent_req_received(req);
		return status;
	}

	talloc_steal(mem_ctx, state->out_mem_ctx);

	tevent_req_received(req);
	return NT_STATUS_OK;
}

NTSTATUS dcerpc_frsrpc_FrsVerifyPromotionParent_r(struct dcerpc_binding_handle *h, TALLOC_CTX *mem_ctx, struct frsrpc_FrsVerifyPromotionParent *r)
{
	NTSTATUS status;

	status = dcerpc_binding_handle_call(h,
			NULL, &ndr_table_frsrpc,
			NDR_FRSRPC_FRSVERIFYPROMOTIONPARENT, mem_ctx, r);

	return status;
}

struct dcerpc_frsrpc_FrsVerifyPromotionParent_state {
	struct frsrpc_FrsVerifyPromotionParent orig;
	struct frsrpc_FrsVerifyPromotionParent tmp;
	TALLOC_CTX *out_mem_ctx;
};

static void dcerpc_frsrpc_FrsVerifyPromotionParent_done(struct tevent_req *subreq);

struct tevent_req *dcerpc_frsrpc_FrsVerifyPromotionParent_send(TALLOC_CTX *mem_ctx,
							       struct tevent_context *ev,
							       struct dcerpc_binding_handle *h,
							       const char *_parent_account /* [in] [charset(UTF16),unique] */,
							       const char *_parent_password /* [in] [charset(UTF16),unique] */,
							       const char *_replica_set_name /* [in] [unique,charset(UTF16)] */,
							       const char *_replica_set_type /* [in] [charset(UTF16),unique] */,
							       enum frsrpc_PartnerAuthLevel _partner_auth_level /* [in]  */,
							       uint32_t ___ndr_guid_size /* [in]  */)
{
	struct tevent_req *req;
	struct dcerpc_frsrpc_FrsVerifyPromotionParent_state *state;
	struct tevent_req *subreq;

	req = tevent_req_create(mem_ctx, &state,
				struct dcerpc_frsrpc_FrsVerifyPromotionParent_state);
	if (req == NULL) {
		return NULL;
	}
	state->out_mem_ctx = NULL;

	/* In parameters */
	state->orig.in.parent_account = _parent_account;
	state->orig.in.parent_password = _parent_password;
	state->orig.in.replica_set_name = _replica_set_name;
	state->orig.in.replica_set_type = _replica_set_type;
	state->orig.in.partner_auth_level = _partner_auth_level;
	state->orig.in.__ndr_guid_size = ___ndr_guid_size;

	/* Out parameters */

	/* Result */
	ZERO_STRUCT(state->orig.out.result);

	/* make a temporary copy, that we pass to the dispatch function */
	state->tmp = state->orig;

	subreq = dcerpc_frsrpc_FrsVerifyPromotionParent_r_send(state, ev, h, &state->tmp);
	if (tevent_req_nomem(subreq, req)) {
		return tevent_req_post(req, ev);
	}
	tevent_req_set_callback(subreq, dcerpc_frsrpc_FrsVerifyPromotionParent_done, req);
	return req;
}

static void dcerpc_frsrpc_FrsVerifyPromotionParent_done(struct tevent_req *subreq)
{
	struct tevent_req *req = tevent_req_callback_data(
		subreq, struct tevent_req);
	struct dcerpc_frsrpc_FrsVerifyPromotionParent_state *state = tevent_req_data(
		req, struct dcerpc_frsrpc_FrsVerifyPromotionParent_state);
	NTSTATUS status;
	TALLOC_CTX *mem_ctx;

	if (state->out_mem_ctx) {
		mem_ctx = state->out_mem_ctx;
	} else {
		mem_ctx = state;
	}

	status = dcerpc_frsrpc_FrsVerifyPromotionParent_r_recv(subreq, mem_ctx);
	TALLOC_FREE(subreq);
	if (!NT_STATUS_IS_OK(status)) {
		tevent_req_nterror(req, status);
		return;
	}

	/* Copy out parameters */

	/* Copy result */
	state->orig.out.result = state->tmp.out.result;

	/* Reset temporary structure */
	ZERO_STRUCT(state->tmp);

	tevent_req_done(req);
}

NTSTATUS dcerpc_frsrpc_FrsVerifyPromotionParent_recv(struct tevent_req *req,
						     TALLOC_CTX *mem_ctx,
						     WERROR *result)
{
	struct dcerpc_frsrpc_FrsVerifyPromotionParent_state *state = tevent_req_data(
		req, struct dcerpc_frsrpc_FrsVerifyPromotionParent_state);
	NTSTATUS status;

	if (tevent_req_is_nterror(req, &status)) {
		tevent_req_received(req);
		return status;
	}

	/* Steal possible out parameters to the callers context */
	talloc_steal(mem_ctx, state->out_mem_ctx);

	/* Return result */
	*result = state->orig.out.result;

	tevent_req_received(req);
	return NT_STATUS_OK;
}

NTSTATUS dcerpc_frsrpc_FrsVerifyPromotionParent(struct dcerpc_binding_handle *h,
						TALLOC_CTX *mem_ctx,
						const char *_parent_account /* [in] [charset(UTF16),unique] */,
						const char *_parent_password /* [in] [charset(UTF16),unique] */,
						const char *_replica_set_name /* [in] [unique,charset(UTF16)] */,
						const char *_replica_set_type /* [in] [charset(UTF16),unique] */,
						enum frsrpc_PartnerAuthLevel _partner_auth_level /* [in]  */,
						uint32_t ___ndr_guid_size /* [in]  */,
						WERROR *result)
{
	struct frsrpc_FrsVerifyPromotionParent r;
	NTSTATUS status;

	/* In parameters */
	r.in.parent_account = _parent_account;
	r.in.parent_password = _parent_password;
	r.in.replica_set_name = _replica_set_name;
	r.in.replica_set_type = _replica_set_type;
	r.in.partner_auth_level = _partner_auth_level;
	r.in.__ndr_guid_size = ___ndr_guid_size;

	/* Out parameters */

	/* Result */
	ZERO_STRUCT(r.out.result);

	status = dcerpc_frsrpc_FrsVerifyPromotionParent_r(h, mem_ctx, &r);
	if (!NT_STATUS_IS_OK(status)) {
		return status;
	}

	/* Return variables */

	/* Return result */
	*result = r.out.result;

	return NT_STATUS_OK;
}

struct dcerpc_frsrpc_FrsStartPromotionParent_r_state {
	TALLOC_CTX *out_mem_ctx;
};

static void dcerpc_frsrpc_FrsStartPromotionParent_r_done(struct tevent_req *subreq);

struct tevent_req *dcerpc_frsrpc_FrsStartPromotionParent_r_send(TALLOC_CTX *mem_ctx,
	struct tevent_context *ev,
	struct dcerpc_binding_handle *h,
	struct frsrpc_FrsStartPromotionParent *r)
{
	struct tevent_req *req;
	struct dcerpc_frsrpc_FrsStartPromotionParent_r_state *state;
	struct tevent_req *subreq;

	req = tevent_req_create(mem_ctx, &state,
				struct dcerpc_frsrpc_FrsStartPromotionParent_r_state);
	if (req == NULL) {
		return NULL;
	}

	state->out_mem_ctx = talloc_new(state);
	if (tevent_req_nomem(state->out_mem_ctx, req)) {
		return tevent_req_post(req, ev);
	}

	subreq = dcerpc_binding_handle_call_send(state, ev, h,
			NULL, &ndr_table_frsrpc,
			NDR_FRSRPC_FRSSTARTPROMOTIONPARENT, state->out_mem_ctx, r);
	if (tevent_req_nomem(subreq, req)) {
		return tevent_req_post(req, ev);
	}
	tevent_req_set_callback(subreq, dcerpc_frsrpc_FrsStartPromotionParent_r_done, req);

	return req;
}

static void dcerpc_frsrpc_FrsStartPromotionParent_r_done(struct tevent_req *subreq)
{
	struct tevent_req *req =
		tevent_req_callback_data(subreq,
		struct tevent_req);
	NTSTATUS status;

	status = dcerpc_binding_handle_call_recv(subreq);
	if (!NT_STATUS_IS_OK(status)) {
		tevent_req_nterror(req, status);
		return;
	}

	tevent_req_done(req);
}

NTSTATUS dcerpc_frsrpc_FrsStartPromotionParent_r_recv(struct tevent_req *req, TALLOC_CTX *mem_ctx)
{
	struct dcerpc_frsrpc_FrsStartPromotionParent_r_state *state =
		tevent_req_data(req,
		struct dcerpc_frsrpc_FrsStartPromotionParent_r_state);
	NTSTATUS status;

	if (tevent_req_is_nterror(req, &status)) {
		tevent_req_received(req);
		return status;
	}

	talloc_steal(mem_ctx, state->out_mem_ctx);

	tevent_req_received(req);
	return NT_STATUS_OK;
}

NTSTATUS dcerpc_frsrpc_FrsStartPromotionParent_r(struct dcerpc_binding_handle *h, TALLOC_CTX *mem_ctx, struct frsrpc_FrsStartPromotionParent *r)
{
	NTSTATUS status;

	status = dcerpc_binding_handle_call(h,
			NULL, &ndr_table_frsrpc,
			NDR_FRSRPC_FRSSTARTPROMOTIONPARENT, mem_ctx, r);

	return status;
}

struct dcerpc_frsrpc_FrsStartPromotionParent_state {
	struct frsrpc_FrsStartPromotionParent orig;
	struct frsrpc_FrsStartPromotionParent tmp;
	TALLOC_CTX *out_mem_ctx;
};

static void dcerpc_frsrpc_FrsStartPromotionParent_done(struct tevent_req *subreq);

struct tevent_req *dcerpc_frsrpc_FrsStartPromotionParent_send(TALLOC_CTX *mem_ctx,
							      struct tevent_context *ev,
							      struct dcerpc_binding_handle *h,
							      const char *_parent_account /* [in] [unique,charset(UTF16)] */,
							      const char *_parent_password /* [in] [charset(UTF16),unique] */,
							      const char *_replica_set_name /* [in] [charset(UTF16),unique] */,
							      const char *_replica_set_type /* [in] [charset(UTF16),unique] */,
							      const char *_connection_name /* [in] [charset(UTF16),unique] */,
							      const char *_partner_name /* [in] [charset(UTF16),unique] */,
							      const char *_partner_princ_name /* [in] [unique,charset(UTF16)] */,
							      enum frsrpc_PartnerAuthLevel _partner_auth_level /* [in]  */,
							      uint32_t ___ndr_guid_size /* [in] [value(16),range(16,16)] */,
							      struct GUID *_connection_guid /* [in] [subcontext_size(16),subcontext(4),unique] */,
							      struct GUID *_partner_guid /* [in] [unique,subcontext_size(16),subcontext(4)] */,
							      struct GUID *_parent_guid /* [in,out] [unique,subcontext_size(16),subcontext(4)] */)
{
	struct tevent_req *req;
	struct dcerpc_frsrpc_FrsStartPromotionParent_state *state;
	struct tevent_req *subreq;

	req = tevent_req_create(mem_ctx, &state,
				struct dcerpc_frsrpc_FrsStartPromotionParent_state);
	if (req == NULL) {
		return NULL;
	}
	state->out_mem_ctx = NULL;

	/* In parameters */
	state->orig.in.parent_account = _parent_account;
	state->orig.in.parent_password = _parent_password;
	state->orig.in.replica_set_name = _replica_set_name;
	state->orig.in.replica_set_type = _replica_set_type;
	state->orig.in.connection_name = _connection_name;
	state->orig.in.partner_name = _partner_name;
	state->orig.in.partner_princ_name = _partner_princ_name;
	state->orig.in.partner_auth_level = _partner_auth_level;
	state->orig.in.__ndr_guid_size = ___ndr_guid_size;
	state->orig.in.connection_guid = _connection_guid;
	state->orig.in.partner_guid = _partner_guid;
	state->orig.in.parent_guid = _parent_guid;

	/* Out parameters */
	state->orig.out.parent_guid = _parent_guid;

	/* Result */
	ZERO_STRUCT(state->orig.out.result);

	state->out_mem_ctx = talloc_named_const(state, 0,
			     "dcerpc_frsrpc_FrsStartPromotionParent_out_memory");
	if (tevent_req_nomem(state->out_mem_ctx, req)) {
		return tevent_req_post(req, ev);
	}

	/* make a temporary copy, that we pass to the dispatch function */
	state->tmp = state->orig;

	subreq = dcerpc_frsrpc_FrsStartPromotionParent_r_send(state, ev, h, &state->tmp);
	if (tevent_req_nomem(subreq, req)) {
		return tevent_req_post(req, ev);
	}
	tevent_req_set_callback(subreq, dcerpc_frsrpc_FrsStartPromotionParent_done, req);
	return req;
}

static void dcerpc_frsrpc_FrsStartPromotionParent_done(struct tevent_req *subreq)
{
	struct tevent_req *req = tevent_req_callback_data(
		subreq, struct tevent_req);
	struct dcerpc_frsrpc_FrsStartPromotionParent_state *state = tevent_req_data(
		req, struct dcerpc_frsrpc_FrsStartPromotionParent_state);
	NTSTATUS status;
	TALLOC_CTX *mem_ctx;

	if (state->out_mem_ctx) {
		mem_ctx = state->out_mem_ctx;
	} else {
		mem_ctx = state;
	}

	status = dcerpc_frsrpc_FrsStartPromotionParent_r_recv(subreq, mem_ctx);
	TALLOC_FREE(subreq);
	if (!NT_STATUS_IS_OK(status)) {
		tevent_req_nterror(req, status);
		return;
	}

	/* Copy out parameters */
	if (state->orig.out.parent_guid && state->tmp.out.parent_guid) {
		*state->orig.out.parent_guid = *state->tmp.out.parent_guid;
	}

	/* Copy result */
	state->orig.out.result = state->tmp.out.result;

	/* Reset temporary structure */
	ZERO_STRUCT(state->tmp);

	tevent_req_done(req);
}

NTSTATUS dcerpc_frsrpc_FrsStartPromotionParent_recv(struct tevent_req *req,
						    TALLOC_CTX *mem_ctx,
						    WERROR *result)
{
	struct dcerpc_frsrpc_FrsStartPromotionParent_state *state = tevent_req_data(
		req, struct dcerpc_frsrpc_FrsStartPromotionParent_state);
	NTSTATUS status;

	if (tevent_req_is_nterror(req, &status)) {
		tevent_req_received(req);
		return status;
	}

	/* Steal possible out parameters to the callers context */
	talloc_steal(mem_ctx, state->out_mem_ctx);

	/* Return result */
	*result = state->orig.out.result;

	tevent_req_received(req);
	return NT_STATUS_OK;
}

NTSTATUS dcerpc_frsrpc_FrsStartPromotionParent(struct dcerpc_binding_handle *h,
					       TALLOC_CTX *mem_ctx,
					       const char *_parent_account /* [in] [unique,charset(UTF16)] */,
					       const char *_parent_password /* [in] [charset(UTF16),unique] */,
					       const char *_replica_set_name /* [in] [charset(UTF16),unique] */,
					       const char *_replica_set_type /* [in] [charset(UTF16),unique] */,
					       const char *_connection_name /* [in] [charset(UTF16),unique] */,
					       const char *_partner_name /* [in] [charset(UTF16),unique] */,
					       const char *_partner_princ_name /* [in] [unique,charset(UTF16)] */,
					       enum frsrpc_PartnerAuthLevel _partner_auth_level /* [in]  */,
					       uint32_t ___ndr_guid_size /* [in] [value(16),range(16,16)] */,
					       struct GUID *_connection_guid /* [in] [subcontext_size(16),subcontext(4),unique] */,
					       struct GUID *_partner_guid /* [in] [unique,subcontext_size(16),subcontext(4)] */,
					       struct GUID *_parent_guid /* [in,out] [unique,subcontext_size(16),subcontext(4)] */,
					       WERROR *result)
{
	struct frsrpc_FrsStartPromotionParent r;
	NTSTATUS status;

	/* In parameters */
	r.in.parent_account = _parent_account;
	r.in.parent_password = _parent_password;
	r.in.replica_set_name = _replica_set_name;
	r.in.replica_set_type = _replica_set_type;
	r.in.connection_name = _connection_name;
	r.in.partner_name = _partner_name;
	r.in.partner_princ_name = _partner_princ_name;
	r.in.partner_auth_level = _partner_auth_level;
	r.in.__ndr_guid_size = ___ndr_guid_size;
	r.in.connection_guid = _connection_guid;
	r.in.partner_guid = _partner_guid;
	r.in.parent_guid = _parent_guid;

	/* Out parameters */
	r.out.parent_guid = _parent_guid;

	/* Result */
	ZERO_STRUCT(r.out.result);

	status = dcerpc_frsrpc_FrsStartPromotionParent_r(h, mem_ctx, &r);
	if (!NT_STATUS_IS_OK(status)) {
		return status;
	}

	/* Return variables */
	if (_parent_guid && r.out.parent_guid) {
		*_parent_guid = *r.out.parent_guid;
	}

	/* Return result */
	*result = r.out.result;

	return NT_STATUS_OK;
}

struct dcerpc_frsrpc_FrsNOP_r_state {
	TALLOC_CTX *out_mem_ctx;
};

static void dcerpc_frsrpc_FrsNOP_r_done(struct tevent_req *subreq);

struct tevent_req *dcerpc_frsrpc_FrsNOP_r_send(TALLOC_CTX *mem_ctx,
	struct tevent_context *ev,
	struct dcerpc_binding_handle *h,
	struct frsrpc_FrsNOP *r)
{
	struct tevent_req *req;
	struct dcerpc_frsrpc_FrsNOP_r_state *state;
	struct tevent_req *subreq;

	req = tevent_req_create(mem_ctx, &state,
				struct dcerpc_frsrpc_FrsNOP_r_state);
	if (req == NULL) {
		return NULL;
	}

	state->out_mem_ctx = NULL;

	subreq = dcerpc_binding_handle_call_send(state, ev, h,
			NULL, &ndr_table_frsrpc,
			NDR_FRSRPC_FRSNOP, state, r);
	if (tevent_req_nomem(subreq, req)) {
		return tevent_req_post(req, ev);
	}
	tevent_req_set_callback(subreq, dcerpc_frsrpc_FrsNOP_r_done, req);

	return req;
}

static void dcerpc_frsrpc_FrsNOP_r_done(struct tevent_req *subreq)
{
	struct tevent_req *req =
		tevent_req_callback_data(subreq,
		struct tevent_req);
	NTSTATUS status;

	status = dcerpc_binding_handle_call_recv(subreq);
	if (!NT_STATUS_IS_OK(status)) {
		tevent_req_nterror(req, status);
		return;
	}

	tevent_req_done(req);
}

NTSTATUS dcerpc_frsrpc_FrsNOP_r_recv(struct tevent_req *req, TALLOC_CTX *mem_ctx)
{
	struct dcerpc_frsrpc_FrsNOP_r_state *state =
		tevent_req_data(req,
		struct dcerpc_frsrpc_FrsNOP_r_state);
	NTSTATUS status;

	if (tevent_req_is_nterror(req, &status)) {
		tevent_req_received(req);
		return status;
	}

	talloc_steal(mem_ctx, state->out_mem_ctx);

	tevent_req_received(req);
	return NT_STATUS_OK;
}

NTSTATUS dcerpc_frsrpc_FrsNOP_r(struct dcerpc_binding_handle *h, TALLOC_CTX *mem_ctx, struct frsrpc_FrsNOP *r)
{
	NTSTATUS status;

	status = dcerpc_binding_handle_call(h,
			NULL, &ndr_table_frsrpc,
			NDR_FRSRPC_FRSNOP, mem_ctx, r);

	return status;
}

struct dcerpc_frsrpc_FrsNOP_state {
	struct frsrpc_FrsNOP orig;
	struct frsrpc_FrsNOP tmp;
	TALLOC_CTX *out_mem_ctx;
};

static void dcerpc_frsrpc_FrsNOP_done(struct tevent_req *subreq);

struct tevent_req *dcerpc_frsrpc_FrsNOP_send(TALLOC_CTX *mem_ctx,
					     struct tevent_context *ev,
					     struct dcerpc_binding_handle *h)
{
	struct tevent_req *req;
	struct dcerpc_frsrpc_FrsNOP_state *state;
	struct tevent_req *subreq;

	req = tevent_req_create(mem_ctx, &state,
				struct dcerpc_frsrpc_FrsNOP_state);
	if (req == NULL) {
		return NULL;
	}
	state->out_mem_ctx = NULL;

	/* In parameters */

	/* Out parameters */

	/* Result */
	ZERO_STRUCT(state->orig.out.result);

	/* make a temporary copy, that we pass to the dispatch function */
	state->tmp = state->orig;

	subreq = dcerpc_frsrpc_FrsNOP_r_send(state, ev, h, &state->tmp);
	if (tevent_req_nomem(subreq, req)) {
		return tevent_req_post(req, ev);
	}
	tevent_req_set_callback(subreq, dcerpc_frsrpc_FrsNOP_done, req);
	return req;
}

static void dcerpc_frsrpc_FrsNOP_done(struct tevent_req *subreq)
{
	struct tevent_req *req = tevent_req_callback_data(
		subreq, struct tevent_req);
	struct dcerpc_frsrpc_FrsNOP_state *state = tevent_req_data(
		req, struct dcerpc_frsrpc_FrsNOP_state);
	NTSTATUS status;
	TALLOC_CTX *mem_ctx;

	if (state->out_mem_ctx) {
		mem_ctx = state->out_mem_ctx;
	} else {
		mem_ctx = state;
	}

	status = dcerpc_frsrpc_FrsNOP_r_recv(subreq, mem_ctx);
	TALLOC_FREE(subreq);
	if (!NT_STATUS_IS_OK(status)) {
		tevent_req_nterror(req, status);
		return;
	}

	/* Copy out parameters */

	/* Copy result */
	state->orig.out.result = state->tmp.out.result;

	/* Reset temporary structure */
	ZERO_STRUCT(state->tmp);

	tevent_req_done(req);
}

NTSTATUS dcerpc_frsrpc_FrsNOP_recv(struct tevent_req *req,
				   TALLOC_CTX *mem_ctx,
				   WERROR *result)
{
	struct dcerpc_frsrpc_FrsNOP_state *state = tevent_req_data(
		req, struct dcerpc_frsrpc_FrsNOP_state);
	NTSTATUS status;

	if (tevent_req_is_nterror(req, &status)) {
		tevent_req_received(req);
		return status;
	}

	/* Steal possible out parameters to the callers context */
	talloc_steal(mem_ctx, state->out_mem_ctx);

	/* Return result */
	*result = state->orig.out.result;

	tevent_req_received(req);
	return NT_STATUS_OK;
}

NTSTATUS dcerpc_frsrpc_FrsNOP(struct dcerpc_binding_handle *h,
			      TALLOC_CTX *mem_ctx,
			      WERROR *result)
{
	struct frsrpc_FrsNOP r;
	NTSTATUS status;

	/* In parameters */

	/* Out parameters */

	/* Result */
	ZERO_STRUCT(r.out.result);

	status = dcerpc_frsrpc_FrsNOP_r(h, mem_ctx, &r);
	if (!NT_STATUS_IS_OK(status)) {
		return status;
	}

	/* Return variables */

	/* Return result */
	*result = r.out.result;

	return NT_STATUS_OK;
}


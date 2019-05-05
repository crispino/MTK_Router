/* parser auto-generated by pidl */

#include "includes.h"
#include "autoconf/librpc/gen_ndr/ndr_printcap.h"

static enum ndr_err_code ndr_push_pcap_printer(struct ndr_push *ndr, int ndr_flags, const struct pcap_printer *r)
{
	NDR_PUSH_CHECK_FLAGS(ndr, ndr_flags);
	if (ndr_flags & NDR_SCALARS) {
		NDR_CHECK(ndr_push_align(ndr, 5));
		NDR_CHECK(ndr_push_unique_ptr(ndr, r->name));
		NDR_CHECK(ndr_push_unique_ptr(ndr, r->info));
		NDR_CHECK(ndr_push_unique_ptr(ndr, r->location));
		NDR_CHECK(ndr_push_trailer_align(ndr, 5));
	}
	if (ndr_flags & NDR_BUFFERS) {
		if (r->name) {
			NDR_CHECK(ndr_push_uint3264(ndr, NDR_SCALARS, ndr_charset_length(r->name, CH_UTF8)));
			NDR_CHECK(ndr_push_uint3264(ndr, NDR_SCALARS, 0));
			NDR_CHECK(ndr_push_uint3264(ndr, NDR_SCALARS, ndr_charset_length(r->name, CH_UTF8)));
			NDR_CHECK(ndr_push_charset(ndr, NDR_SCALARS, r->name, ndr_charset_length(r->name, CH_UTF8), sizeof(uint8_t), CH_UTF8));
		}
		if (r->info) {
			NDR_CHECK(ndr_push_uint3264(ndr, NDR_SCALARS, ndr_charset_length(r->info, CH_UTF8)));
			NDR_CHECK(ndr_push_uint3264(ndr, NDR_SCALARS, 0));
			NDR_CHECK(ndr_push_uint3264(ndr, NDR_SCALARS, ndr_charset_length(r->info, CH_UTF8)));
			NDR_CHECK(ndr_push_charset(ndr, NDR_SCALARS, r->info, ndr_charset_length(r->info, CH_UTF8), sizeof(uint8_t), CH_UTF8));
		}
		if (r->location) {
			NDR_CHECK(ndr_push_uint3264(ndr, NDR_SCALARS, ndr_charset_length(r->location, CH_UTF8)));
			NDR_CHECK(ndr_push_uint3264(ndr, NDR_SCALARS, 0));
			NDR_CHECK(ndr_push_uint3264(ndr, NDR_SCALARS, ndr_charset_length(r->location, CH_UTF8)));
			NDR_CHECK(ndr_push_charset(ndr, NDR_SCALARS, r->location, ndr_charset_length(r->location, CH_UTF8), sizeof(uint8_t), CH_UTF8));
		}
	}
	return NDR_ERR_SUCCESS;
}

static enum ndr_err_code ndr_pull_pcap_printer(struct ndr_pull *ndr, int ndr_flags, struct pcap_printer *r)
{
	uint32_t _ptr_name;
	uint32_t size_name_1 = 0;
	uint32_t length_name_1 = 0;
	TALLOC_CTX *_mem_save_name_0;
	uint32_t _ptr_info;
	uint32_t size_info_1 = 0;
	uint32_t length_info_1 = 0;
	TALLOC_CTX *_mem_save_info_0;
	uint32_t _ptr_location;
	uint32_t size_location_1 = 0;
	uint32_t length_location_1 = 0;
	TALLOC_CTX *_mem_save_location_0;
	NDR_PULL_CHECK_FLAGS(ndr, ndr_flags);
	if (ndr_flags & NDR_SCALARS) {
		NDR_CHECK(ndr_pull_align(ndr, 5));
		NDR_CHECK(ndr_pull_generic_ptr(ndr, &_ptr_name));
		if (_ptr_name) {
			NDR_PULL_ALLOC(ndr, r->name);
		} else {
			r->name = NULL;
		}
		NDR_CHECK(ndr_pull_generic_ptr(ndr, &_ptr_info));
		if (_ptr_info) {
			NDR_PULL_ALLOC(ndr, r->info);
		} else {
			r->info = NULL;
		}
		NDR_CHECK(ndr_pull_generic_ptr(ndr, &_ptr_location));
		if (_ptr_location) {
			NDR_PULL_ALLOC(ndr, r->location);
		} else {
			r->location = NULL;
		}
		NDR_CHECK(ndr_pull_trailer_align(ndr, 5));
	}
	if (ndr_flags & NDR_BUFFERS) {
		if (r->name) {
			_mem_save_name_0 = NDR_PULL_GET_MEM_CTX(ndr);
			NDR_PULL_SET_MEM_CTX(ndr, r->name, 0);
			NDR_CHECK(ndr_pull_array_size(ndr, &r->name));
			NDR_CHECK(ndr_pull_array_length(ndr, &r->name));
			size_name_1 = ndr_get_array_size(ndr, &r->name);
			length_name_1 = ndr_get_array_length(ndr, &r->name);
			if (length_name_1 > size_name_1) {
				return ndr_pull_error(ndr, NDR_ERR_ARRAY_SIZE, "Bad array size %u should exceed array length %u", size_name_1, length_name_1);
			}
			NDR_CHECK(ndr_check_string_terminator(ndr, length_name_1, sizeof(uint8_t)));
			NDR_CHECK(ndr_pull_charset(ndr, NDR_SCALARS, &r->name, length_name_1, sizeof(uint8_t), CH_UTF8));
			NDR_PULL_SET_MEM_CTX(ndr, _mem_save_name_0, 0);
		}
		if (r->info) {
			_mem_save_info_0 = NDR_PULL_GET_MEM_CTX(ndr);
			NDR_PULL_SET_MEM_CTX(ndr, r->info, 0);
			NDR_CHECK(ndr_pull_array_size(ndr, &r->info));
			NDR_CHECK(ndr_pull_array_length(ndr, &r->info));
			size_info_1 = ndr_get_array_size(ndr, &r->info);
			length_info_1 = ndr_get_array_length(ndr, &r->info);
			if (length_info_1 > size_info_1) {
				return ndr_pull_error(ndr, NDR_ERR_ARRAY_SIZE, "Bad array size %u should exceed array length %u", size_info_1, length_info_1);
			}
			NDR_CHECK(ndr_check_string_terminator(ndr, length_info_1, sizeof(uint8_t)));
			NDR_CHECK(ndr_pull_charset(ndr, NDR_SCALARS, &r->info, length_info_1, sizeof(uint8_t), CH_UTF8));
			NDR_PULL_SET_MEM_CTX(ndr, _mem_save_info_0, 0);
		}
		if (r->location) {
			_mem_save_location_0 = NDR_PULL_GET_MEM_CTX(ndr);
			NDR_PULL_SET_MEM_CTX(ndr, r->location, 0);
			NDR_CHECK(ndr_pull_array_size(ndr, &r->location));
			NDR_CHECK(ndr_pull_array_length(ndr, &r->location));
			size_location_1 = ndr_get_array_size(ndr, &r->location);
			length_location_1 = ndr_get_array_length(ndr, &r->location);
			if (length_location_1 > size_location_1) {
				return ndr_pull_error(ndr, NDR_ERR_ARRAY_SIZE, "Bad array size %u should exceed array length %u", size_location_1, length_location_1);
			}
			NDR_CHECK(ndr_check_string_terminator(ndr, length_location_1, sizeof(uint8_t)));
			NDR_CHECK(ndr_pull_charset(ndr, NDR_SCALARS, &r->location, length_location_1, sizeof(uint8_t), CH_UTF8));
			NDR_PULL_SET_MEM_CTX(ndr, _mem_save_location_0, 0);
		}
	}
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ void ndr_print_pcap_printer(struct ndr_print *ndr, const char *name, const struct pcap_printer *r)
{
	ndr_print_struct(ndr, name, "pcap_printer");
	if (r == NULL) { ndr_print_null(ndr); return; }
	ndr->depth++;
	ndr_print_ptr(ndr, "name", r->name);
	ndr->depth++;
	if (r->name) {
		ndr_print_string(ndr, "name", r->name);
	}
	ndr->depth--;
	ndr_print_ptr(ndr, "info", r->info);
	ndr->depth++;
	if (r->info) {
		ndr_print_string(ndr, "info", r->info);
	}
	ndr->depth--;
	ndr_print_ptr(ndr, "location", r->location);
	ndr->depth++;
	if (r->location) {
		ndr_print_string(ndr, "location", r->location);
	}
	ndr->depth--;
	ndr->depth--;
}

_PUBLIC_ enum ndr_err_code ndr_push_pcap_data(struct ndr_push *ndr, int ndr_flags, const struct pcap_data *r)
{
	uint32_t cntr_printers_0;
	NDR_PUSH_CHECK_FLAGS(ndr, ndr_flags);
	if (ndr_flags & NDR_SCALARS) {
		NDR_CHECK(ndr_push_uint3264(ndr, NDR_SCALARS, r->count));
		NDR_CHECK(ndr_push_align(ndr, 5));
		NDR_CHECK(ndr_push_NTSTATUS(ndr, NDR_SCALARS, r->status));
		NDR_CHECK(ndr_push_uint32(ndr, NDR_SCALARS, r->count));
		for (cntr_printers_0 = 0; cntr_printers_0 < r->count; cntr_printers_0++) {
			NDR_CHECK(ndr_push_pcap_printer(ndr, NDR_SCALARS, &r->printers[cntr_printers_0]));
		}
		NDR_CHECK(ndr_push_trailer_align(ndr, 5));
	}
	if (ndr_flags & NDR_BUFFERS) {
		for (cntr_printers_0 = 0; cntr_printers_0 < r->count; cntr_printers_0++) {
			NDR_CHECK(ndr_push_pcap_printer(ndr, NDR_BUFFERS, &r->printers[cntr_printers_0]));
		}
	}
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ enum ndr_err_code ndr_pull_pcap_data(struct ndr_pull *ndr, int ndr_flags, struct pcap_data *r)
{
	uint32_t size_printers_0 = 0;
	uint32_t cntr_printers_0;
	TALLOC_CTX *_mem_save_printers_0;
	NDR_PULL_CHECK_FLAGS(ndr, ndr_flags);
	if (ndr_flags & NDR_SCALARS) {
		NDR_CHECK(ndr_pull_array_size(ndr, &r->printers));
		NDR_CHECK(ndr_pull_align(ndr, 5));
		NDR_CHECK(ndr_pull_NTSTATUS(ndr, NDR_SCALARS, &r->status));
		NDR_CHECK(ndr_pull_uint32(ndr, NDR_SCALARS, &r->count));
		size_printers_0 = ndr_get_array_size(ndr, &r->printers);
		NDR_PULL_ALLOC_N(ndr, r->printers, size_printers_0);
		_mem_save_printers_0 = NDR_PULL_GET_MEM_CTX(ndr);
		NDR_PULL_SET_MEM_CTX(ndr, r->printers, 0);
		for (cntr_printers_0 = 0; cntr_printers_0 < size_printers_0; cntr_printers_0++) {
			NDR_CHECK(ndr_pull_pcap_printer(ndr, NDR_SCALARS, &r->printers[cntr_printers_0]));
		}
		NDR_PULL_SET_MEM_CTX(ndr, _mem_save_printers_0, 0);
		if (r->printers) {
			NDR_CHECK(ndr_check_array_size(ndr, (void*)&r->printers, r->count));
		}
		NDR_CHECK(ndr_pull_trailer_align(ndr, 5));
	}
	if (ndr_flags & NDR_BUFFERS) {
		size_printers_0 = ndr_get_array_size(ndr, &r->printers);
		_mem_save_printers_0 = NDR_PULL_GET_MEM_CTX(ndr);
		NDR_PULL_SET_MEM_CTX(ndr, r->printers, 0);
		for (cntr_printers_0 = 0; cntr_printers_0 < size_printers_0; cntr_printers_0++) {
			NDR_CHECK(ndr_pull_pcap_printer(ndr, NDR_BUFFERS, &r->printers[cntr_printers_0]));
		}
		NDR_PULL_SET_MEM_CTX(ndr, _mem_save_printers_0, 0);
	}
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ void ndr_print_pcap_data(struct ndr_print *ndr, const char *name, const struct pcap_data *r)
{
	uint32_t cntr_printers_0;
	ndr_print_struct(ndr, name, "pcap_data");
	if (r == NULL) { ndr_print_null(ndr); return; }
	ndr->depth++;
	ndr_print_NTSTATUS(ndr, "status", r->status);
	ndr_print_uint32(ndr, "count", r->count);
	ndr->print(ndr, "%s: ARRAY(%d)", "printers", (int)r->count);
	ndr->depth++;
	for (cntr_printers_0=0;cntr_printers_0<r->count;cntr_printers_0++) {
		ndr_print_pcap_printer(ndr, "printers", &r->printers[cntr_printers_0]);
	}
	ndr->depth--;
	ndr->depth--;
}


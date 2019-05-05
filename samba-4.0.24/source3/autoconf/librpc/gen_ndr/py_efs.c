
/* Python wrapper functions auto-generated by pidl */
#include <Python.h>
#include "includes.h"
#include <pytalloc.h>
#include "librpc/rpc/pyrpc.h"
#include "librpc/rpc/pyrpc_util.h"
#include "autoconf/librpc/gen_ndr/ndr_efs.h"
#include "autoconf/librpc/gen_ndr/ndr_efs_c.h"

#include "librpc/gen_ndr/security.h"
staticforward PyTypeObject EFS_HASH_BLOB_Type;
staticforward PyTypeObject ENCRYPTION_CERTIFICATE_HASH_Type;
staticforward PyTypeObject ENCRYPTION_CERTIFICATE_HASH_LIST_Type;
staticforward PyTypeObject EFS_CERTIFICATE_BLOB_Type;
staticforward PyTypeObject ENCRYPTION_CERTIFICATE_Type;
staticforward PyTypeObject efs_InterfaceType;

void initefs(void);static PyTypeObject *policy_handle_Type;
static PyTypeObject *ClientConnection_Type;
static PyTypeObject *dom_sid_Type;
static PyTypeObject *Object_Type;

static PyObject *py_EFS_HASH_BLOB_get_cbData(PyObject *obj, void *closure)
{
	struct EFS_HASH_BLOB *object = (struct EFS_HASH_BLOB *)pytalloc_get_ptr(obj);
	PyObject *py_cbData;
	py_cbData = PyInt_FromLong(object->cbData);
	return py_cbData;
}

static int py_EFS_HASH_BLOB_set_cbData(PyObject *py_obj, PyObject *value, void *closure)
{
	struct EFS_HASH_BLOB *object = (struct EFS_HASH_BLOB *)pytalloc_get_ptr(py_obj);
	PY_CHECK_TYPE(&PyInt_Type, value, return -1;);
	object->cbData = PyInt_AsLong(value);
	return 0;
}

static PyObject *py_EFS_HASH_BLOB_get_pbData(PyObject *obj, void *closure)
{
	struct EFS_HASH_BLOB *object = (struct EFS_HASH_BLOB *)pytalloc_get_ptr(obj);
	PyObject *py_pbData;
	if (object->pbData == NULL) {
		py_pbData = Py_None;
		Py_INCREF(py_pbData);
	} else {
		py_pbData = PyList_New(object->cbData);
		if (py_pbData == NULL) {
			return NULL;
		}
		{
			int pbData_cntr_1;
			for (pbData_cntr_1 = 0; pbData_cntr_1 < object->cbData; pbData_cntr_1++) {
				PyObject *py_pbData_1;
				py_pbData_1 = PyInt_FromLong(object->pbData[pbData_cntr_1]);
				PyList_SetItem(py_pbData, pbData_cntr_1, py_pbData_1);
			}
		}
	}
	return py_pbData;
}

static int py_EFS_HASH_BLOB_set_pbData(PyObject *py_obj, PyObject *value, void *closure)
{
	struct EFS_HASH_BLOB *object = (struct EFS_HASH_BLOB *)pytalloc_get_ptr(py_obj);
	talloc_unlink(pytalloc_get_mem_ctx(py_obj), object->pbData);
	if (value == Py_None) {
		object->pbData = NULL;
	} else {
		object->pbData = NULL;
		PY_CHECK_TYPE(&PyList_Type, value, return -1;);
		{
			int pbData_cntr_1;
			object->pbData = talloc_array_ptrtype(pytalloc_get_mem_ctx(py_obj), object->pbData, PyList_GET_SIZE(value));
			if (!object->pbData) { return -1;; }
			talloc_set_name_const(object->pbData, "ARRAY: object->pbData");
			for (pbData_cntr_1 = 0; pbData_cntr_1 < PyList_GET_SIZE(value); pbData_cntr_1++) {
				PY_CHECK_TYPE(&PyInt_Type, PyList_GET_ITEM(value, pbData_cntr_1), return -1;);
				object->pbData[pbData_cntr_1] = PyInt_AsLong(PyList_GET_ITEM(value, pbData_cntr_1));
			}
		}
	}
	return 0;
}

static PyGetSetDef py_EFS_HASH_BLOB_getsetters[] = {
	{ discard_const_p(char, "cbData"), py_EFS_HASH_BLOB_get_cbData, py_EFS_HASH_BLOB_set_cbData },
	{ discard_const_p(char, "pbData"), py_EFS_HASH_BLOB_get_pbData, py_EFS_HASH_BLOB_set_pbData },
	{ NULL }
};

static PyObject *py_EFS_HASH_BLOB_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
	return pytalloc_new(struct EFS_HASH_BLOB, type);
}


static PyTypeObject EFS_HASH_BLOB_Type = {
	PyObject_HEAD_INIT(NULL) 0,
	.tp_name = "efs.EFS_HASH_BLOB",
	.tp_getset = py_EFS_HASH_BLOB_getsetters,
	.tp_methods = NULL,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_basicsize = sizeof(pytalloc_Object),
	.tp_new = py_EFS_HASH_BLOB_new,
};


static PyObject *py_ENCRYPTION_CERTIFICATE_HASH_get_cbTotalLength(PyObject *obj, void *closure)
{
	struct ENCRYPTION_CERTIFICATE_HASH *object = (struct ENCRYPTION_CERTIFICATE_HASH *)pytalloc_get_ptr(obj);
	PyObject *py_cbTotalLength;
	py_cbTotalLength = PyInt_FromLong(object->cbTotalLength);
	return py_cbTotalLength;
}

static int py_ENCRYPTION_CERTIFICATE_HASH_set_cbTotalLength(PyObject *py_obj, PyObject *value, void *closure)
{
	struct ENCRYPTION_CERTIFICATE_HASH *object = (struct ENCRYPTION_CERTIFICATE_HASH *)pytalloc_get_ptr(py_obj);
	PY_CHECK_TYPE(&PyInt_Type, value, return -1;);
	object->cbTotalLength = PyInt_AsLong(value);
	return 0;
}

static PyObject *py_ENCRYPTION_CERTIFICATE_HASH_get_pUserSid(PyObject *obj, void *closure)
{
	struct ENCRYPTION_CERTIFICATE_HASH *object = (struct ENCRYPTION_CERTIFICATE_HASH *)pytalloc_get_ptr(obj);
	PyObject *py_pUserSid;
	if (object->pUserSid == NULL) {
		py_pUserSid = Py_None;
		Py_INCREF(py_pUserSid);
	} else {
		py_pUserSid = pytalloc_reference_ex(dom_sid_Type, object->pUserSid, object->pUserSid);
	}
	return py_pUserSid;
}

static int py_ENCRYPTION_CERTIFICATE_HASH_set_pUserSid(PyObject *py_obj, PyObject *value, void *closure)
{
	struct ENCRYPTION_CERTIFICATE_HASH *object = (struct ENCRYPTION_CERTIFICATE_HASH *)pytalloc_get_ptr(py_obj);
	talloc_unlink(pytalloc_get_mem_ctx(py_obj), object->pUserSid);
	if (value == Py_None) {
		object->pUserSid = NULL;
	} else {
		object->pUserSid = NULL;
		PY_CHECK_TYPE(dom_sid_Type, value, return -1;);
		if (talloc_reference(pytalloc_get_mem_ctx(py_obj), pytalloc_get_mem_ctx(value)) == NULL) {
			PyErr_NoMemory();
			return -1;
		}
		object->pUserSid = (struct dom_sid *)pytalloc_get_ptr(value);
	}
	return 0;
}

static PyObject *py_ENCRYPTION_CERTIFICATE_HASH_get_pHash(PyObject *obj, void *closure)
{
	struct ENCRYPTION_CERTIFICATE_HASH *object = (struct ENCRYPTION_CERTIFICATE_HASH *)pytalloc_get_ptr(obj);
	PyObject *py_pHash;
	if (object->pHash == NULL) {
		py_pHash = Py_None;
		Py_INCREF(py_pHash);
	} else {
		py_pHash = pytalloc_reference_ex(&EFS_HASH_BLOB_Type, object->pHash, object->pHash);
	}
	return py_pHash;
}

static int py_ENCRYPTION_CERTIFICATE_HASH_set_pHash(PyObject *py_obj, PyObject *value, void *closure)
{
	struct ENCRYPTION_CERTIFICATE_HASH *object = (struct ENCRYPTION_CERTIFICATE_HASH *)pytalloc_get_ptr(py_obj);
	talloc_unlink(pytalloc_get_mem_ctx(py_obj), object->pHash);
	if (value == Py_None) {
		object->pHash = NULL;
	} else {
		object->pHash = NULL;
		PY_CHECK_TYPE(&EFS_HASH_BLOB_Type, value, return -1;);
		if (talloc_reference(pytalloc_get_mem_ctx(py_obj), pytalloc_get_mem_ctx(value)) == NULL) {
			PyErr_NoMemory();
			return -1;
		}
		object->pHash = (struct EFS_HASH_BLOB *)pytalloc_get_ptr(value);
	}
	return 0;
}

static PyObject *py_ENCRYPTION_CERTIFICATE_HASH_get_lpDisplayInformation(PyObject *obj, void *closure)
{
	struct ENCRYPTION_CERTIFICATE_HASH *object = (struct ENCRYPTION_CERTIFICATE_HASH *)pytalloc_get_ptr(obj);
	PyObject *py_lpDisplayInformation;
	if (object->lpDisplayInformation == NULL) {
		py_lpDisplayInformation = Py_None;
		Py_INCREF(py_lpDisplayInformation);
	} else {
		if (object->lpDisplayInformation == NULL) {
			py_lpDisplayInformation = Py_None;
			Py_INCREF(py_lpDisplayInformation);
		} else {
			py_lpDisplayInformation = PyUnicode_Decode(object->lpDisplayInformation, strlen(object->lpDisplayInformation), "utf-8", "ignore");
		}
	}
	return py_lpDisplayInformation;
}

static int py_ENCRYPTION_CERTIFICATE_HASH_set_lpDisplayInformation(PyObject *py_obj, PyObject *value, void *closure)
{
	struct ENCRYPTION_CERTIFICATE_HASH *object = (struct ENCRYPTION_CERTIFICATE_HASH *)pytalloc_get_ptr(py_obj);
	if (value == Py_None) {
		object->lpDisplayInformation = NULL;
	} else {
		object->lpDisplayInformation = NULL;
		if (PyUnicode_Check(value)) {
			object->lpDisplayInformation = PyString_AS_STRING(PyUnicode_AsEncodedString(value, "utf-8", "ignore"));
		} else if (PyString_Check(value)) {
			object->lpDisplayInformation = PyString_AS_STRING(value);
		} else {
			PyErr_Format(PyExc_TypeError, "Expected string or unicode object, got %s", Py_TYPE(value)->tp_name);
			return -1;
		}
	}
	return 0;
}

static PyGetSetDef py_ENCRYPTION_CERTIFICATE_HASH_getsetters[] = {
	{ discard_const_p(char, "cbTotalLength"), py_ENCRYPTION_CERTIFICATE_HASH_get_cbTotalLength, py_ENCRYPTION_CERTIFICATE_HASH_set_cbTotalLength },
	{ discard_const_p(char, "pUserSid"), py_ENCRYPTION_CERTIFICATE_HASH_get_pUserSid, py_ENCRYPTION_CERTIFICATE_HASH_set_pUserSid },
	{ discard_const_p(char, "pHash"), py_ENCRYPTION_CERTIFICATE_HASH_get_pHash, py_ENCRYPTION_CERTIFICATE_HASH_set_pHash },
	{ discard_const_p(char, "lpDisplayInformation"), py_ENCRYPTION_CERTIFICATE_HASH_get_lpDisplayInformation, py_ENCRYPTION_CERTIFICATE_HASH_set_lpDisplayInformation },
	{ NULL }
};

static PyObject *py_ENCRYPTION_CERTIFICATE_HASH_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
	return pytalloc_new(struct ENCRYPTION_CERTIFICATE_HASH, type);
}


static PyTypeObject ENCRYPTION_CERTIFICATE_HASH_Type = {
	PyObject_HEAD_INIT(NULL) 0,
	.tp_name = "efs.ENCRYPTION_CERTIFICATE_HASH",
	.tp_getset = py_ENCRYPTION_CERTIFICATE_HASH_getsetters,
	.tp_methods = NULL,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_basicsize = sizeof(pytalloc_Object),
	.tp_new = py_ENCRYPTION_CERTIFICATE_HASH_new,
};


static PyObject *py_ENCRYPTION_CERTIFICATE_HASH_LIST_get_nCert_Hash(PyObject *obj, void *closure)
{
	struct ENCRYPTION_CERTIFICATE_HASH_LIST *object = (struct ENCRYPTION_CERTIFICATE_HASH_LIST *)pytalloc_get_ptr(obj);
	PyObject *py_nCert_Hash;
	py_nCert_Hash = PyInt_FromLong(object->nCert_Hash);
	return py_nCert_Hash;
}

static int py_ENCRYPTION_CERTIFICATE_HASH_LIST_set_nCert_Hash(PyObject *py_obj, PyObject *value, void *closure)
{
	struct ENCRYPTION_CERTIFICATE_HASH_LIST *object = (struct ENCRYPTION_CERTIFICATE_HASH_LIST *)pytalloc_get_ptr(py_obj);
	PY_CHECK_TYPE(&PyInt_Type, value, return -1;);
	object->nCert_Hash = PyInt_AsLong(value);
	return 0;
}

static PyObject *py_ENCRYPTION_CERTIFICATE_HASH_LIST_get_pUsers(PyObject *obj, void *closure)
{
	struct ENCRYPTION_CERTIFICATE_HASH_LIST *object = (struct ENCRYPTION_CERTIFICATE_HASH_LIST *)pytalloc_get_ptr(obj);
	PyObject *py_pUsers;
	py_pUsers = PyList_New(object->nCert_Hash);
	if (py_pUsers == NULL) {
		return NULL;
	}
	{
		int pUsers_cntr_0;
		for (pUsers_cntr_0 = 0; pUsers_cntr_0 < object->nCert_Hash; pUsers_cntr_0++) {
			PyObject *py_pUsers_0;
			if (object->pUsers[pUsers_cntr_0] == NULL) {
				py_pUsers_0 = Py_None;
				Py_INCREF(py_pUsers_0);
			} else {
				py_pUsers_0 = pytalloc_reference_ex(&ENCRYPTION_CERTIFICATE_HASH_Type, object->pUsers[pUsers_cntr_0], object->pUsers[pUsers_cntr_0]);
			}
			PyList_SetItem(py_pUsers, pUsers_cntr_0, py_pUsers_0);
		}
	}
	return py_pUsers;
}

static int py_ENCRYPTION_CERTIFICATE_HASH_LIST_set_pUsers(PyObject *py_obj, PyObject *value, void *closure)
{
	struct ENCRYPTION_CERTIFICATE_HASH_LIST *object = (struct ENCRYPTION_CERTIFICATE_HASH_LIST *)pytalloc_get_ptr(py_obj);
	PY_CHECK_TYPE(&PyList_Type, value, return -1;);
	{
		int pUsers_cntr_0;
		object->pUsers = talloc_array_ptrtype(pytalloc_get_mem_ctx(py_obj), object->pUsers, PyList_GET_SIZE(value));
		if (!object->pUsers) { return -1;; }
		talloc_set_name_const(object->pUsers, "ARRAY: object->pUsers");
		for (pUsers_cntr_0 = 0; pUsers_cntr_0 < PyList_GET_SIZE(value); pUsers_cntr_0++) {
			if (PyList_GET_ITEM(value, pUsers_cntr_0) == Py_None) {
				object->pUsers[pUsers_cntr_0] = NULL;
			} else {
				object->pUsers[pUsers_cntr_0] = NULL;
				PY_CHECK_TYPE(&ENCRYPTION_CERTIFICATE_HASH_Type, PyList_GET_ITEM(value, pUsers_cntr_0), return -1;);
				if (talloc_reference(object->pUsers, pytalloc_get_mem_ctx(PyList_GET_ITEM(value, pUsers_cntr_0))) == NULL) {
					PyErr_NoMemory();
					return -1;
				}
				object->pUsers[pUsers_cntr_0] = (struct ENCRYPTION_CERTIFICATE_HASH *)pytalloc_get_ptr(PyList_GET_ITEM(value, pUsers_cntr_0));
			}
		}
	}
	return 0;
}

static PyGetSetDef py_ENCRYPTION_CERTIFICATE_HASH_LIST_getsetters[] = {
	{ discard_const_p(char, "nCert_Hash"), py_ENCRYPTION_CERTIFICATE_HASH_LIST_get_nCert_Hash, py_ENCRYPTION_CERTIFICATE_HASH_LIST_set_nCert_Hash },
	{ discard_const_p(char, "pUsers"), py_ENCRYPTION_CERTIFICATE_HASH_LIST_get_pUsers, py_ENCRYPTION_CERTIFICATE_HASH_LIST_set_pUsers },
	{ NULL }
};

static PyObject *py_ENCRYPTION_CERTIFICATE_HASH_LIST_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
	return pytalloc_new(struct ENCRYPTION_CERTIFICATE_HASH_LIST, type);
}


static PyTypeObject ENCRYPTION_CERTIFICATE_HASH_LIST_Type = {
	PyObject_HEAD_INIT(NULL) 0,
	.tp_name = "efs.ENCRYPTION_CERTIFICATE_HASH_LIST",
	.tp_getset = py_ENCRYPTION_CERTIFICATE_HASH_LIST_getsetters,
	.tp_methods = NULL,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_basicsize = sizeof(pytalloc_Object),
	.tp_new = py_ENCRYPTION_CERTIFICATE_HASH_LIST_new,
};


static PyObject *py_EFS_CERTIFICATE_BLOB_get_dwCertEncodingType(PyObject *obj, void *closure)
{
	struct EFS_CERTIFICATE_BLOB *object = (struct EFS_CERTIFICATE_BLOB *)pytalloc_get_ptr(obj);
	PyObject *py_dwCertEncodingType;
	py_dwCertEncodingType = PyInt_FromLong(object->dwCertEncodingType);
	return py_dwCertEncodingType;
}

static int py_EFS_CERTIFICATE_BLOB_set_dwCertEncodingType(PyObject *py_obj, PyObject *value, void *closure)
{
	struct EFS_CERTIFICATE_BLOB *object = (struct EFS_CERTIFICATE_BLOB *)pytalloc_get_ptr(py_obj);
	PY_CHECK_TYPE(&PyInt_Type, value, return -1;);
	object->dwCertEncodingType = PyInt_AsLong(value);
	return 0;
}

static PyObject *py_EFS_CERTIFICATE_BLOB_get_cbData(PyObject *obj, void *closure)
{
	struct EFS_CERTIFICATE_BLOB *object = (struct EFS_CERTIFICATE_BLOB *)pytalloc_get_ptr(obj);
	PyObject *py_cbData;
	py_cbData = PyInt_FromLong(object->cbData);
	return py_cbData;
}

static int py_EFS_CERTIFICATE_BLOB_set_cbData(PyObject *py_obj, PyObject *value, void *closure)
{
	struct EFS_CERTIFICATE_BLOB *object = (struct EFS_CERTIFICATE_BLOB *)pytalloc_get_ptr(py_obj);
	PY_CHECK_TYPE(&PyInt_Type, value, return -1;);
	object->cbData = PyInt_AsLong(value);
	return 0;
}

static PyObject *py_EFS_CERTIFICATE_BLOB_get_pbData(PyObject *obj, void *closure)
{
	struct EFS_CERTIFICATE_BLOB *object = (struct EFS_CERTIFICATE_BLOB *)pytalloc_get_ptr(obj);
	PyObject *py_pbData;
	if (object->pbData == NULL) {
		py_pbData = Py_None;
		Py_INCREF(py_pbData);
	} else {
		py_pbData = PyList_New(object->cbData);
		if (py_pbData == NULL) {
			return NULL;
		}
		{
			int pbData_cntr_1;
			for (pbData_cntr_1 = 0; pbData_cntr_1 < object->cbData; pbData_cntr_1++) {
				PyObject *py_pbData_1;
				py_pbData_1 = PyInt_FromLong(object->pbData[pbData_cntr_1]);
				PyList_SetItem(py_pbData, pbData_cntr_1, py_pbData_1);
			}
		}
	}
	return py_pbData;
}

static int py_EFS_CERTIFICATE_BLOB_set_pbData(PyObject *py_obj, PyObject *value, void *closure)
{
	struct EFS_CERTIFICATE_BLOB *object = (struct EFS_CERTIFICATE_BLOB *)pytalloc_get_ptr(py_obj);
	talloc_unlink(pytalloc_get_mem_ctx(py_obj), object->pbData);
	if (value == Py_None) {
		object->pbData = NULL;
	} else {
		object->pbData = NULL;
		PY_CHECK_TYPE(&PyList_Type, value, return -1;);
		{
			int pbData_cntr_1;
			object->pbData = talloc_array_ptrtype(pytalloc_get_mem_ctx(py_obj), object->pbData, PyList_GET_SIZE(value));
			if (!object->pbData) { return -1;; }
			talloc_set_name_const(object->pbData, "ARRAY: object->pbData");
			for (pbData_cntr_1 = 0; pbData_cntr_1 < PyList_GET_SIZE(value); pbData_cntr_1++) {
				PY_CHECK_TYPE(&PyInt_Type, PyList_GET_ITEM(value, pbData_cntr_1), return -1;);
				object->pbData[pbData_cntr_1] = PyInt_AsLong(PyList_GET_ITEM(value, pbData_cntr_1));
			}
		}
	}
	return 0;
}

static PyGetSetDef py_EFS_CERTIFICATE_BLOB_getsetters[] = {
	{ discard_const_p(char, "dwCertEncodingType"), py_EFS_CERTIFICATE_BLOB_get_dwCertEncodingType, py_EFS_CERTIFICATE_BLOB_set_dwCertEncodingType },
	{ discard_const_p(char, "cbData"), py_EFS_CERTIFICATE_BLOB_get_cbData, py_EFS_CERTIFICATE_BLOB_set_cbData },
	{ discard_const_p(char, "pbData"), py_EFS_CERTIFICATE_BLOB_get_pbData, py_EFS_CERTIFICATE_BLOB_set_pbData },
	{ NULL }
};

static PyObject *py_EFS_CERTIFICATE_BLOB_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
	return pytalloc_new(struct EFS_CERTIFICATE_BLOB, type);
}


static PyTypeObject EFS_CERTIFICATE_BLOB_Type = {
	PyObject_HEAD_INIT(NULL) 0,
	.tp_name = "efs.EFS_CERTIFICATE_BLOB",
	.tp_getset = py_EFS_CERTIFICATE_BLOB_getsetters,
	.tp_methods = NULL,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_basicsize = sizeof(pytalloc_Object),
	.tp_new = py_EFS_CERTIFICATE_BLOB_new,
};


static PyObject *py_ENCRYPTION_CERTIFICATE_get_TotalLength(PyObject *obj, void *closure)
{
	struct ENCRYPTION_CERTIFICATE *object = (struct ENCRYPTION_CERTIFICATE *)pytalloc_get_ptr(obj);
	PyObject *py_TotalLength;
	py_TotalLength = PyInt_FromLong(object->TotalLength);
	return py_TotalLength;
}

static int py_ENCRYPTION_CERTIFICATE_set_TotalLength(PyObject *py_obj, PyObject *value, void *closure)
{
	struct ENCRYPTION_CERTIFICATE *object = (struct ENCRYPTION_CERTIFICATE *)pytalloc_get_ptr(py_obj);
	PY_CHECK_TYPE(&PyInt_Type, value, return -1;);
	object->TotalLength = PyInt_AsLong(value);
	return 0;
}

static PyObject *py_ENCRYPTION_CERTIFICATE_get_pUserSid(PyObject *obj, void *closure)
{
	struct ENCRYPTION_CERTIFICATE *object = (struct ENCRYPTION_CERTIFICATE *)pytalloc_get_ptr(obj);
	PyObject *py_pUserSid;
	if (object->pUserSid == NULL) {
		py_pUserSid = Py_None;
		Py_INCREF(py_pUserSid);
	} else {
		py_pUserSid = pytalloc_reference_ex(dom_sid_Type, object->pUserSid, object->pUserSid);
	}
	return py_pUserSid;
}

static int py_ENCRYPTION_CERTIFICATE_set_pUserSid(PyObject *py_obj, PyObject *value, void *closure)
{
	struct ENCRYPTION_CERTIFICATE *object = (struct ENCRYPTION_CERTIFICATE *)pytalloc_get_ptr(py_obj);
	talloc_unlink(pytalloc_get_mem_ctx(py_obj), object->pUserSid);
	if (value == Py_None) {
		object->pUserSid = NULL;
	} else {
		object->pUserSid = NULL;
		PY_CHECK_TYPE(dom_sid_Type, value, return -1;);
		if (talloc_reference(pytalloc_get_mem_ctx(py_obj), pytalloc_get_mem_ctx(value)) == NULL) {
			PyErr_NoMemory();
			return -1;
		}
		object->pUserSid = (struct dom_sid *)pytalloc_get_ptr(value);
	}
	return 0;
}

static PyObject *py_ENCRYPTION_CERTIFICATE_get_pCertBlob(PyObject *obj, void *closure)
{
	struct ENCRYPTION_CERTIFICATE *object = (struct ENCRYPTION_CERTIFICATE *)pytalloc_get_ptr(obj);
	PyObject *py_pCertBlob;
	if (object->pCertBlob == NULL) {
		py_pCertBlob = Py_None;
		Py_INCREF(py_pCertBlob);
	} else {
		py_pCertBlob = pytalloc_reference_ex(&EFS_CERTIFICATE_BLOB_Type, object->pCertBlob, object->pCertBlob);
	}
	return py_pCertBlob;
}

static int py_ENCRYPTION_CERTIFICATE_set_pCertBlob(PyObject *py_obj, PyObject *value, void *closure)
{
	struct ENCRYPTION_CERTIFICATE *object = (struct ENCRYPTION_CERTIFICATE *)pytalloc_get_ptr(py_obj);
	talloc_unlink(pytalloc_get_mem_ctx(py_obj), object->pCertBlob);
	if (value == Py_None) {
		object->pCertBlob = NULL;
	} else {
		object->pCertBlob = NULL;
		PY_CHECK_TYPE(&EFS_CERTIFICATE_BLOB_Type, value, return -1;);
		if (talloc_reference(pytalloc_get_mem_ctx(py_obj), pytalloc_get_mem_ctx(value)) == NULL) {
			PyErr_NoMemory();
			return -1;
		}
		object->pCertBlob = (struct EFS_CERTIFICATE_BLOB *)pytalloc_get_ptr(value);
	}
	return 0;
}

static PyGetSetDef py_ENCRYPTION_CERTIFICATE_getsetters[] = {
	{ discard_const_p(char, "TotalLength"), py_ENCRYPTION_CERTIFICATE_get_TotalLength, py_ENCRYPTION_CERTIFICATE_set_TotalLength },
	{ discard_const_p(char, "pUserSid"), py_ENCRYPTION_CERTIFICATE_get_pUserSid, py_ENCRYPTION_CERTIFICATE_set_pUserSid },
	{ discard_const_p(char, "pCertBlob"), py_ENCRYPTION_CERTIFICATE_get_pCertBlob, py_ENCRYPTION_CERTIFICATE_set_pCertBlob },
	{ NULL }
};

static PyObject *py_ENCRYPTION_CERTIFICATE_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
	return pytalloc_new(struct ENCRYPTION_CERTIFICATE, type);
}


static PyTypeObject ENCRYPTION_CERTIFICATE_Type = {
	PyObject_HEAD_INIT(NULL) 0,
	.tp_name = "efs.ENCRYPTION_CERTIFICATE",
	.tp_getset = py_ENCRYPTION_CERTIFICATE_getsetters,
	.tp_methods = NULL,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_basicsize = sizeof(pytalloc_Object),
	.tp_new = py_ENCRYPTION_CERTIFICATE_new,
};


static bool pack_py_EfsRpcOpenFileRaw_args_in(PyObject *args, PyObject *kwargs, struct EfsRpcOpenFileRaw *r)
{
	PyObject *py_FileName;
	PyObject *py_Flags;
	const char *kwnames[] = {
		"FileName", "Flags", NULL
	};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO:EfsRpcOpenFileRaw", discard_const_p(char *, kwnames), &py_FileName, &py_Flags)) {
		return false;
	}

	if (PyUnicode_Check(py_FileName)) {
		r->in.FileName = PyString_AS_STRING(PyUnicode_AsEncodedString(py_FileName, "utf-8", "ignore"));
	} else if (PyString_Check(py_FileName)) {
		r->in.FileName = PyString_AS_STRING(py_FileName);
	} else {
		PyErr_Format(PyExc_TypeError, "Expected string or unicode object, got %s", Py_TYPE(py_FileName)->tp_name);
		return false;
	}
	PY_CHECK_TYPE(&PyInt_Type, py_Flags, return false;);
	r->in.Flags = PyInt_AsLong(py_Flags);
	return true;
}

static PyObject *unpack_py_EfsRpcOpenFileRaw_args_out(struct EfsRpcOpenFileRaw *r)
{
	PyObject *result;
	PyObject *py_pvContext;
	py_pvContext = pytalloc_reference_ex(policy_handle_Type, r->out.pvContext, r->out.pvContext);
	result = py_pvContext;
	if (!W_ERROR_IS_OK(r->out.result)) {
		PyErr_SetWERROR(r->out.result);
		return NULL;
	}

	return result;
}

static bool pack_py_EfsRpcCloseRaw_args_in(PyObject *args, PyObject *kwargs, struct EfsRpcCloseRaw *r)
{
	PyObject *py_pvContext;
	const char *kwnames[] = {
		"pvContext", NULL
	};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:EfsRpcCloseRaw", discard_const_p(char *, kwnames), &py_pvContext)) {
		return false;
	}

	r->in.pvContext = talloc_ptrtype(r, r->in.pvContext);
	PY_CHECK_TYPE(policy_handle_Type, py_pvContext, return false;);
	if (talloc_reference(r, pytalloc_get_mem_ctx(py_pvContext)) == NULL) {
		PyErr_NoMemory();
		return false;
	}
	r->in.pvContext = (struct policy_handle *)pytalloc_get_ptr(py_pvContext);
	return true;
}

static PyObject *unpack_py_EfsRpcCloseRaw_args_out(struct EfsRpcCloseRaw *r)
{
	PyObject *result;
	PyObject *py_pvContext;
	py_pvContext = pytalloc_reference_ex(policy_handle_Type, r->out.pvContext, r->out.pvContext);
	result = py_pvContext;
	return result;
}

static bool pack_py_EfsRpcEncryptFileSrv_args_in(PyObject *args, PyObject *kwargs, struct EfsRpcEncryptFileSrv *r)
{
	PyObject *py_Filename;
	const char *kwnames[] = {
		"Filename", NULL
	};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:EfsRpcEncryptFileSrv", discard_const_p(char *, kwnames), &py_Filename)) {
		return false;
	}

	if (PyUnicode_Check(py_Filename)) {
		r->in.Filename = PyString_AS_STRING(PyUnicode_AsEncodedString(py_Filename, "utf-8", "ignore"));
	} else if (PyString_Check(py_Filename)) {
		r->in.Filename = PyString_AS_STRING(py_Filename);
	} else {
		PyErr_Format(PyExc_TypeError, "Expected string or unicode object, got %s", Py_TYPE(py_Filename)->tp_name);
		return false;
	}
	return true;
}

static PyObject *unpack_py_EfsRpcEncryptFileSrv_args_out(struct EfsRpcEncryptFileSrv *r)
{
	PyObject *result;
	result = Py_None;
	Py_INCREF(result);
	if (!W_ERROR_IS_OK(r->out.result)) {
		PyErr_SetWERROR(r->out.result);
		return NULL;
	}

	return result;
}

static bool pack_py_EfsRpcDecryptFileSrv_args_in(PyObject *args, PyObject *kwargs, struct EfsRpcDecryptFileSrv *r)
{
	PyObject *py_FileName;
	PyObject *py_Reserved;
	const char *kwnames[] = {
		"FileName", "Reserved", NULL
	};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO:EfsRpcDecryptFileSrv", discard_const_p(char *, kwnames), &py_FileName, &py_Reserved)) {
		return false;
	}

	if (PyUnicode_Check(py_FileName)) {
		r->in.FileName = PyString_AS_STRING(PyUnicode_AsEncodedString(py_FileName, "utf-8", "ignore"));
	} else if (PyString_Check(py_FileName)) {
		r->in.FileName = PyString_AS_STRING(py_FileName);
	} else {
		PyErr_Format(PyExc_TypeError, "Expected string or unicode object, got %s", Py_TYPE(py_FileName)->tp_name);
		return false;
	}
	PY_CHECK_TYPE(&PyInt_Type, py_Reserved, return false;);
	r->in.Reserved = PyInt_AsLong(py_Reserved);
	return true;
}

static PyObject *unpack_py_EfsRpcDecryptFileSrv_args_out(struct EfsRpcDecryptFileSrv *r)
{
	PyObject *result;
	result = Py_None;
	Py_INCREF(result);
	if (!W_ERROR_IS_OK(r->out.result)) {
		PyErr_SetWERROR(r->out.result);
		return NULL;
	}

	return result;
}

static bool pack_py_EfsRpcQueryUsersOnFile_args_in(PyObject *args, PyObject *kwargs, struct EfsRpcQueryUsersOnFile *r)
{
	PyObject *py_FileName;
	const char *kwnames[] = {
		"FileName", NULL
	};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:EfsRpcQueryUsersOnFile", discard_const_p(char *, kwnames), &py_FileName)) {
		return false;
	}

	if (PyUnicode_Check(py_FileName)) {
		r->in.FileName = PyString_AS_STRING(PyUnicode_AsEncodedString(py_FileName, "utf-8", "ignore"));
	} else if (PyString_Check(py_FileName)) {
		r->in.FileName = PyString_AS_STRING(py_FileName);
	} else {
		PyErr_Format(PyExc_TypeError, "Expected string or unicode object, got %s", Py_TYPE(py_FileName)->tp_name);
		return false;
	}
	return true;
}

static PyObject *unpack_py_EfsRpcQueryUsersOnFile_args_out(struct EfsRpcQueryUsersOnFile *r)
{
	PyObject *result;
	PyObject *py_pUsers;
	if (*r->out.pUsers == NULL) {
		py_pUsers = Py_None;
		Py_INCREF(py_pUsers);
	} else {
		py_pUsers = pytalloc_reference_ex(&ENCRYPTION_CERTIFICATE_HASH_LIST_Type, *r->out.pUsers, *r->out.pUsers);
	}
	result = py_pUsers;
	if (!W_ERROR_IS_OK(r->out.result)) {
		PyErr_SetWERROR(r->out.result);
		return NULL;
	}

	return result;
}

static bool pack_py_EfsRpcQueryRecoveryAgents_args_in(PyObject *args, PyObject *kwargs, struct EfsRpcQueryRecoveryAgents *r)
{
	PyObject *py_FileName;
	const char *kwnames[] = {
		"FileName", NULL
	};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:EfsRpcQueryRecoveryAgents", discard_const_p(char *, kwnames), &py_FileName)) {
		return false;
	}

	if (PyUnicode_Check(py_FileName)) {
		r->in.FileName = PyString_AS_STRING(PyUnicode_AsEncodedString(py_FileName, "utf-8", "ignore"));
	} else if (PyString_Check(py_FileName)) {
		r->in.FileName = PyString_AS_STRING(py_FileName);
	} else {
		PyErr_Format(PyExc_TypeError, "Expected string or unicode object, got %s", Py_TYPE(py_FileName)->tp_name);
		return false;
	}
	return true;
}

static PyObject *unpack_py_EfsRpcQueryRecoveryAgents_args_out(struct EfsRpcQueryRecoveryAgents *r)
{
	PyObject *result;
	PyObject *py_pRecoveryAgents;
	if (*r->out.pRecoveryAgents == NULL) {
		py_pRecoveryAgents = Py_None;
		Py_INCREF(py_pRecoveryAgents);
	} else {
		py_pRecoveryAgents = pytalloc_reference_ex(&ENCRYPTION_CERTIFICATE_HASH_LIST_Type, *r->out.pRecoveryAgents, *r->out.pRecoveryAgents);
	}
	result = py_pRecoveryAgents;
	if (!W_ERROR_IS_OK(r->out.result)) {
		PyErr_SetWERROR(r->out.result);
		return NULL;
	}

	return result;
}

static bool pack_py_EfsRpcSetFileEncryptionKey_args_in(PyObject *args, PyObject *kwargs, struct EfsRpcSetFileEncryptionKey *r)
{
	PyObject *py_pEncryptionCertificate;
	const char *kwnames[] = {
		"pEncryptionCertificate", NULL
	};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:EfsRpcSetFileEncryptionKey", discard_const_p(char *, kwnames), &py_pEncryptionCertificate)) {
		return false;
	}

	if (py_pEncryptionCertificate == Py_None) {
		r->in.pEncryptionCertificate = NULL;
	} else {
		r->in.pEncryptionCertificate = NULL;
		PY_CHECK_TYPE(&ENCRYPTION_CERTIFICATE_Type, py_pEncryptionCertificate, return false;);
		if (talloc_reference(r, pytalloc_get_mem_ctx(py_pEncryptionCertificate)) == NULL) {
			PyErr_NoMemory();
			return false;
		}
		r->in.pEncryptionCertificate = (struct ENCRYPTION_CERTIFICATE *)pytalloc_get_ptr(py_pEncryptionCertificate);
	}
	return true;
}

static PyObject *unpack_py_EfsRpcSetFileEncryptionKey_args_out(struct EfsRpcSetFileEncryptionKey *r)
{
	PyObject *result;
	result = Py_None;
	Py_INCREF(result);
	if (!W_ERROR_IS_OK(r->out.result)) {
		PyErr_SetWERROR(r->out.result);
		return NULL;
	}

	return result;
}

const struct PyNdrRpcMethodDef py_ndr_efs_methods[] = {
	{ "EfsRpcOpenFileRaw", "S.EfsRpcOpenFileRaw(FileName, Flags) -> pvContext", (py_dcerpc_call_fn)dcerpc_EfsRpcOpenFileRaw_r, (py_data_pack_fn)pack_py_EfsRpcOpenFileRaw_args_in, (py_data_unpack_fn)unpack_py_EfsRpcOpenFileRaw_args_out, 0, &ndr_table_efs },
	{ "EfsRpcCloseRaw", "S.EfsRpcCloseRaw(pvContext) -> pvContext", (py_dcerpc_call_fn)dcerpc_EfsRpcCloseRaw_r, (py_data_pack_fn)pack_py_EfsRpcCloseRaw_args_in, (py_data_unpack_fn)unpack_py_EfsRpcCloseRaw_args_out, 3, &ndr_table_efs },
	{ "EfsRpcEncryptFileSrv", "S.EfsRpcEncryptFileSrv(Filename) -> None", (py_dcerpc_call_fn)dcerpc_EfsRpcEncryptFileSrv_r, (py_data_pack_fn)pack_py_EfsRpcEncryptFileSrv_args_in, (py_data_unpack_fn)unpack_py_EfsRpcEncryptFileSrv_args_out, 4, &ndr_table_efs },
	{ "EfsRpcDecryptFileSrv", "S.EfsRpcDecryptFileSrv(FileName, Reserved) -> None", (py_dcerpc_call_fn)dcerpc_EfsRpcDecryptFileSrv_r, (py_data_pack_fn)pack_py_EfsRpcDecryptFileSrv_args_in, (py_data_unpack_fn)unpack_py_EfsRpcDecryptFileSrv_args_out, 5, &ndr_table_efs },
	{ "EfsRpcQueryUsersOnFile", "S.EfsRpcQueryUsersOnFile(FileName) -> pUsers", (py_dcerpc_call_fn)dcerpc_EfsRpcQueryUsersOnFile_r, (py_data_pack_fn)pack_py_EfsRpcQueryUsersOnFile_args_in, (py_data_unpack_fn)unpack_py_EfsRpcQueryUsersOnFile_args_out, 6, &ndr_table_efs },
	{ "EfsRpcQueryRecoveryAgents", "S.EfsRpcQueryRecoveryAgents(FileName) -> pRecoveryAgents", (py_dcerpc_call_fn)dcerpc_EfsRpcQueryRecoveryAgents_r, (py_data_pack_fn)pack_py_EfsRpcQueryRecoveryAgents_args_in, (py_data_unpack_fn)unpack_py_EfsRpcQueryRecoveryAgents_args_out, 7, &ndr_table_efs },
	{ "EfsRpcSetFileEncryptionKey", "S.EfsRpcSetFileEncryptionKey(pEncryptionCertificate) -> None", (py_dcerpc_call_fn)dcerpc_EfsRpcSetFileEncryptionKey_r, (py_data_pack_fn)pack_py_EfsRpcSetFileEncryptionKey_args_in, (py_data_unpack_fn)unpack_py_EfsRpcSetFileEncryptionKey_args_out, 10, &ndr_table_efs },
	{ NULL }
};

static PyObject *interface_efs_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
	return py_dcerpc_interface_init_helper(type, args, kwargs, &ndr_table_efs);
}

static PyTypeObject efs_InterfaceType = {
	PyObject_HEAD_INIT(NULL) 0,
	.tp_name = "efs.efs",
	.tp_basicsize = sizeof(dcerpc_InterfaceObject),
	.tp_doc = "efs(binding, lp_ctx=None, credentials=None) -> connection\n"
"\n"
"binding should be a DCE/RPC binding string (for example: ncacn_ip_tcp:127.0.0.1)\n"
"lp_ctx should be a path to a smb.conf file or a param.LoadParm object\n"
"credentials should be a credentials.Credentials object.\n\n",
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_new = interface_efs_new,
};

static PyMethodDef efs_methods[] = {
	{ NULL, NULL, 0, NULL }
};

void initefs(void)
{
	PyObject *m;
	PyObject *dep_samba_dcerpc_base;
	PyObject *dep_talloc;
	PyObject *dep_samba_dcerpc_misc;
	PyObject *dep_samba_dcerpc_security;

	dep_samba_dcerpc_base = PyImport_ImportModule("samba.dcerpc.base");
	if (dep_samba_dcerpc_base == NULL)
		return;

	dep_talloc = PyImport_ImportModule("talloc");
	if (dep_talloc == NULL)
		return;

	dep_samba_dcerpc_misc = PyImport_ImportModule("samba.dcerpc.misc");
	if (dep_samba_dcerpc_misc == NULL)
		return;

	dep_samba_dcerpc_security = PyImport_ImportModule("samba.dcerpc.security");
	if (dep_samba_dcerpc_security == NULL)
		return;

	policy_handle_Type = (PyTypeObject *)PyObject_GetAttrString(dep_samba_dcerpc_misc, "policy_handle");
	if (policy_handle_Type == NULL)
		return;

	ClientConnection_Type = (PyTypeObject *)PyObject_GetAttrString(dep_samba_dcerpc_base, "ClientConnection");
	if (ClientConnection_Type == NULL)
		return;

	dom_sid_Type = (PyTypeObject *)PyObject_GetAttrString(dep_samba_dcerpc_security, "dom_sid");
	if (dom_sid_Type == NULL)
		return;

	Object_Type = (PyTypeObject *)PyObject_GetAttrString(dep_talloc, "Object");
	if (Object_Type == NULL)
		return;

	EFS_HASH_BLOB_Type.tp_base = Object_Type;

	ENCRYPTION_CERTIFICATE_HASH_Type.tp_base = Object_Type;

	ENCRYPTION_CERTIFICATE_HASH_LIST_Type.tp_base = Object_Type;

	EFS_CERTIFICATE_BLOB_Type.tp_base = Object_Type;

	ENCRYPTION_CERTIFICATE_Type.tp_base = Object_Type;

	efs_InterfaceType.tp_base = ClientConnection_Type;

	if (PyType_Ready(&EFS_HASH_BLOB_Type) < 0)
		return;
	if (PyType_Ready(&ENCRYPTION_CERTIFICATE_HASH_Type) < 0)
		return;
	if (PyType_Ready(&ENCRYPTION_CERTIFICATE_HASH_LIST_Type) < 0)
		return;
	if (PyType_Ready(&EFS_CERTIFICATE_BLOB_Type) < 0)
		return;
	if (PyType_Ready(&ENCRYPTION_CERTIFICATE_Type) < 0)
		return;
	if (PyType_Ready(&efs_InterfaceType) < 0)
		return;
	if (!PyInterface_AddNdrRpcMethods(&efs_InterfaceType, py_ndr_efs_methods))
		return;

#ifdef PY_EFS_HASH_BLOB_PATCH
	PY_EFS_HASH_BLOB_PATCH(&EFS_HASH_BLOB_Type);
#endif
#ifdef PY_ENCRYPTION_CERTIFICATE_HASH_PATCH
	PY_ENCRYPTION_CERTIFICATE_HASH_PATCH(&ENCRYPTION_CERTIFICATE_HASH_Type);
#endif
#ifdef PY_ENCRYPTION_CERTIFICATE_HASH_LIST_PATCH
	PY_ENCRYPTION_CERTIFICATE_HASH_LIST_PATCH(&ENCRYPTION_CERTIFICATE_HASH_LIST_Type);
#endif
#ifdef PY_EFS_CERTIFICATE_BLOB_PATCH
	PY_EFS_CERTIFICATE_BLOB_PATCH(&EFS_CERTIFICATE_BLOB_Type);
#endif
#ifdef PY_ENCRYPTION_CERTIFICATE_PATCH
	PY_ENCRYPTION_CERTIFICATE_PATCH(&ENCRYPTION_CERTIFICATE_Type);
#endif
#ifdef PY_EFS_PATCH
	PY_EFS_PATCH(&efs_InterfaceType);
#endif

	m = Py_InitModule3("efs", efs_methods, "efs DCE/RPC");
	if (m == NULL)
		return;

	Py_INCREF((PyObject *)(void *)&EFS_HASH_BLOB_Type);
	PyModule_AddObject(m, "EFS_HASH_BLOB", (PyObject *)(void *)&EFS_HASH_BLOB_Type);
	Py_INCREF((PyObject *)(void *)&ENCRYPTION_CERTIFICATE_HASH_Type);
	PyModule_AddObject(m, "ENCRYPTION_CERTIFICATE_HASH", (PyObject *)(void *)&ENCRYPTION_CERTIFICATE_HASH_Type);
	Py_INCREF((PyObject *)(void *)&ENCRYPTION_CERTIFICATE_HASH_LIST_Type);
	PyModule_AddObject(m, "ENCRYPTION_CERTIFICATE_HASH_LIST", (PyObject *)(void *)&ENCRYPTION_CERTIFICATE_HASH_LIST_Type);
	Py_INCREF((PyObject *)(void *)&EFS_CERTIFICATE_BLOB_Type);
	PyModule_AddObject(m, "EFS_CERTIFICATE_BLOB", (PyObject *)(void *)&EFS_CERTIFICATE_BLOB_Type);
	Py_INCREF((PyObject *)(void *)&ENCRYPTION_CERTIFICATE_Type);
	PyModule_AddObject(m, "ENCRYPTION_CERTIFICATE", (PyObject *)(void *)&ENCRYPTION_CERTIFICATE_Type);
	Py_INCREF((PyObject *)(void *)&efs_InterfaceType);
	PyModule_AddObject(m, "efs", (PyObject *)(void *)&efs_InterfaceType);
#ifdef PY_MOD_EFS_PATCH
	PY_MOD_EFS_PATCH(m);
#endif

}

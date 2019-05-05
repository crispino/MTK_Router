
/* Python wrapper functions auto-generated by pidl */
#include <Python.h>
#include "includes.h"
#include <pytalloc.h>
#include "librpc/rpc/pyrpc.h"
#include "librpc/rpc/pyrpc_util.h"
#include "autoconf/librpc/gen_ndr/ndr_rot.h"
#include "autoconf/librpc/gen_ndr/ndr_rot_c.h"

#include "librpc/gen_ndr/orpc.h"
staticforward PyTypeObject rot_InterfaceType;

void initrot(void);static PyTypeObject *ClientConnection_Type;
static PyTypeObject *MInterfacePointer_Type;

static bool pack_py_rot_add_args_in(PyObject *args, PyObject *kwargs, struct rot_add *r)
{
	PyObject *py_flags;
	PyObject *py_unk;
	PyObject *py_moniker;
	const char *kwnames[] = {
		"flags", "unk", "moniker", NULL
	};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OOO:rot_add", discard_const_p(char *, kwnames), &py_flags, &py_unk, &py_moniker)) {
		return false;
	}

	PY_CHECK_TYPE(&PyInt_Type, py_flags, return false;);
	r->in.flags = PyInt_AsLong(py_flags);
	r->in.unk = talloc_ptrtype(r, r->in.unk);
	PY_CHECK_TYPE(MInterfacePointer_Type, py_unk, return false;);
	if (talloc_reference(r, pytalloc_get_mem_ctx(py_unk)) == NULL) {
		PyErr_NoMemory();
		return false;
	}
	r->in.unk = (struct MInterfacePointer *)pytalloc_get_ptr(py_unk);
	r->in.moniker = talloc_ptrtype(r, r->in.moniker);
	PY_CHECK_TYPE(MInterfacePointer_Type, py_moniker, return false;);
	if (talloc_reference(r, pytalloc_get_mem_ctx(py_moniker)) == NULL) {
		PyErr_NoMemory();
		return false;
	}
	r->in.moniker = (struct MInterfacePointer *)pytalloc_get_ptr(py_moniker);
	return true;
}

static PyObject *unpack_py_rot_add_args_out(struct rot_add *r)
{
	PyObject *result;
	PyObject *py_rotid;
	py_rotid = PyInt_FromLong(*r->out.rotid);
	result = py_rotid;
	if (!W_ERROR_IS_OK(r->out.result)) {
		PyErr_SetWERROR(r->out.result);
		return NULL;
	}

	return result;
}

static bool pack_py_rot_remove_args_in(PyObject *args, PyObject *kwargs, struct rot_remove *r)
{
	PyObject *py_rotid;
	const char *kwnames[] = {
		"rotid", NULL
	};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:rot_remove", discard_const_p(char *, kwnames), &py_rotid)) {
		return false;
	}

	PY_CHECK_TYPE(&PyInt_Type, py_rotid, return false;);
	r->in.rotid = PyInt_AsLong(py_rotid);
	return true;
}

static PyObject *unpack_py_rot_remove_args_out(struct rot_remove *r)
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

static bool pack_py_rot_is_listed_args_in(PyObject *args, PyObject *kwargs, struct rot_is_listed *r)
{
	PyObject *py_moniker;
	const char *kwnames[] = {
		"moniker", NULL
	};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:rot_is_listed", discard_const_p(char *, kwnames), &py_moniker)) {
		return false;
	}

	r->in.moniker = talloc_ptrtype(r, r->in.moniker);
	PY_CHECK_TYPE(MInterfacePointer_Type, py_moniker, return false;);
	if (talloc_reference(r, pytalloc_get_mem_ctx(py_moniker)) == NULL) {
		PyErr_NoMemory();
		return false;
	}
	r->in.moniker = (struct MInterfacePointer *)pytalloc_get_ptr(py_moniker);
	return true;
}

static PyObject *unpack_py_rot_is_listed_args_out(struct rot_is_listed *r)
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

static bool pack_py_rot_get_interface_pointer_args_in(PyObject *args, PyObject *kwargs, struct rot_get_interface_pointer *r)
{
	PyObject *py_moniker;
	const char *kwnames[] = {
		"moniker", NULL
	};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:rot_get_interface_pointer", discard_const_p(char *, kwnames), &py_moniker)) {
		return false;
	}

	r->in.moniker = talloc_ptrtype(r, r->in.moniker);
	PY_CHECK_TYPE(MInterfacePointer_Type, py_moniker, return false;);
	if (talloc_reference(r, pytalloc_get_mem_ctx(py_moniker)) == NULL) {
		PyErr_NoMemory();
		return false;
	}
	r->in.moniker = (struct MInterfacePointer *)pytalloc_get_ptr(py_moniker);
	return true;
}

static PyObject *unpack_py_rot_get_interface_pointer_args_out(struct rot_get_interface_pointer *r)
{
	PyObject *result;
	PyObject *py_ip;
	py_ip = pytalloc_reference_ex(MInterfacePointer_Type, r->out.ip, r->out.ip);
	result = py_ip;
	if (!W_ERROR_IS_OK(r->out.result)) {
		PyErr_SetWERROR(r->out.result);
		return NULL;
	}

	return result;
}

static bool pack_py_rot_set_modification_time_args_in(PyObject *args, PyObject *kwargs, struct rot_set_modification_time *r)
{
	PyObject *py_rotid;
	PyObject *py_t;
	const char *kwnames[] = {
		"rotid", "t", NULL
	};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO:rot_set_modification_time", discard_const_p(char *, kwnames), &py_rotid, &py_t)) {
		return false;
	}

	PY_CHECK_TYPE(&PyInt_Type, py_rotid, return false;);
	r->in.rotid = PyInt_AsLong(py_rotid);
	r->in.t = talloc_ptrtype(r, r->in.t);
	if (PyLong_Check(py_t)) {
		*r->in.t = PyLong_AsLongLong(py_t);
	} else if (PyInt_Check(py_t)) {
		*r->in.t = PyInt_AsLong(py_t);
	} else {
		PyErr_Format(PyExc_TypeError, "Expected type %s or %s",\
		  PyInt_Type.tp_name, PyLong_Type.tp_name);
		return false;
	}
	return true;
}

static PyObject *unpack_py_rot_set_modification_time_args_out(struct rot_set_modification_time *r)
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

static bool pack_py_rot_get_modification_time_args_in(PyObject *args, PyObject *kwargs, struct rot_get_modification_time *r)
{
	PyObject *py_moniker;
	const char *kwnames[] = {
		"moniker", NULL
	};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:rot_get_modification_time", discard_const_p(char *, kwnames), &py_moniker)) {
		return false;
	}

	r->in.moniker = talloc_ptrtype(r, r->in.moniker);
	PY_CHECK_TYPE(MInterfacePointer_Type, py_moniker, return false;);
	if (talloc_reference(r, pytalloc_get_mem_ctx(py_moniker)) == NULL) {
		PyErr_NoMemory();
		return false;
	}
	r->in.moniker = (struct MInterfacePointer *)pytalloc_get_ptr(py_moniker);
	return true;
}

static PyObject *unpack_py_rot_get_modification_time_args_out(struct rot_get_modification_time *r)
{
	PyObject *result;
	PyObject *py_t;
	py_t = PyLong_FromLongLong(*r->out.t);
	result = py_t;
	if (!W_ERROR_IS_OK(r->out.result)) {
		PyErr_SetWERROR(r->out.result);
		return NULL;
	}

	return result;
}

static bool pack_py_rot_enum_args_in(PyObject *args, PyObject *kwargs, struct rot_enum *r)
{
	const char *kwnames[] = {
		NULL
	};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, ":rot_enum", discard_const_p(char *, kwnames))) {
		return false;
	}

	return true;
}

static PyObject *unpack_py_rot_enum_args_out(struct rot_enum *r)
{
	PyObject *result;
	PyObject *py_EnumMoniker;
	py_EnumMoniker = pytalloc_reference_ex(MInterfacePointer_Type, r->out.EnumMoniker, r->out.EnumMoniker);
	result = py_EnumMoniker;
	if (!W_ERROR_IS_OK(r->out.result)) {
		PyErr_SetWERROR(r->out.result);
		return NULL;
	}

	return result;
}

const struct PyNdrRpcMethodDef py_ndr_rot_methods[] = {
	{ "add", "S.add(flags, unk, moniker) -> rotid", (py_dcerpc_call_fn)dcerpc_rot_add_r, (py_data_pack_fn)pack_py_rot_add_args_in, (py_data_unpack_fn)unpack_py_rot_add_args_out, 0, &ndr_table_rot },
	{ "remove", "S.remove(rotid) -> None", (py_dcerpc_call_fn)dcerpc_rot_remove_r, (py_data_pack_fn)pack_py_rot_remove_args_in, (py_data_unpack_fn)unpack_py_rot_remove_args_out, 1, &ndr_table_rot },
	{ "is_listed", "S.is_listed(moniker) -> None", (py_dcerpc_call_fn)dcerpc_rot_is_listed_r, (py_data_pack_fn)pack_py_rot_is_listed_args_in, (py_data_unpack_fn)unpack_py_rot_is_listed_args_out, 2, &ndr_table_rot },
	{ "get_interface_pointer", "S.get_interface_pointer(moniker) -> ip", (py_dcerpc_call_fn)dcerpc_rot_get_interface_pointer_r, (py_data_pack_fn)pack_py_rot_get_interface_pointer_args_in, (py_data_unpack_fn)unpack_py_rot_get_interface_pointer_args_out, 3, &ndr_table_rot },
	{ "set_modification_time", "S.set_modification_time(rotid, t) -> None", (py_dcerpc_call_fn)dcerpc_rot_set_modification_time_r, (py_data_pack_fn)pack_py_rot_set_modification_time_args_in, (py_data_unpack_fn)unpack_py_rot_set_modification_time_args_out, 4, &ndr_table_rot },
	{ "get_modification_time", "S.get_modification_time(moniker) -> t", (py_dcerpc_call_fn)dcerpc_rot_get_modification_time_r, (py_data_pack_fn)pack_py_rot_get_modification_time_args_in, (py_data_unpack_fn)unpack_py_rot_get_modification_time_args_out, 5, &ndr_table_rot },
	{ "enum", "S.enum() -> EnumMoniker", (py_dcerpc_call_fn)dcerpc_rot_enum_r, (py_data_pack_fn)pack_py_rot_enum_args_in, (py_data_unpack_fn)unpack_py_rot_enum_args_out, 6, &ndr_table_rot },
	{ NULL }
};

static PyObject *interface_rot_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
	return py_dcerpc_interface_init_helper(type, args, kwargs, &ndr_table_rot);
}

static PyTypeObject rot_InterfaceType = {
	PyObject_HEAD_INIT(NULL) 0,
	.tp_name = "rot.rot",
	.tp_basicsize = sizeof(dcerpc_InterfaceObject),
	.tp_doc = "rot(binding, lp_ctx=None, credentials=None) -> connection\n"
"\n"
"binding should be a DCE/RPC binding string (for example: ncacn_ip_tcp:127.0.0.1)\n"
"lp_ctx should be a path to a smb.conf file or a param.LoadParm object\n"
"credentials should be a credentials.Credentials object.\n\n",
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_new = interface_rot_new,
};

static PyMethodDef rot_methods[] = {
	{ NULL, NULL, 0, NULL }
};

void initrot(void)
{
	PyObject *m;
	PyObject *dep_samba_dcerpc_orpc;
	PyObject *dep_samba_dcerpc_base;

	dep_samba_dcerpc_orpc = PyImport_ImportModule("samba.dcerpc.orpc");
	if (dep_samba_dcerpc_orpc == NULL)
		return;

	dep_samba_dcerpc_base = PyImport_ImportModule("samba.dcerpc.base");
	if (dep_samba_dcerpc_base == NULL)
		return;

	ClientConnection_Type = (PyTypeObject *)PyObject_GetAttrString(dep_samba_dcerpc_base, "ClientConnection");
	if (ClientConnection_Type == NULL)
		return;

	MInterfacePointer_Type = (PyTypeObject *)PyObject_GetAttrString(dep_samba_dcerpc_orpc, "MInterfacePointer");
	if (MInterfacePointer_Type == NULL)
		return;

	rot_InterfaceType.tp_base = ClientConnection_Type;

	if (PyType_Ready(&rot_InterfaceType) < 0)
		return;
	if (!PyInterface_AddNdrRpcMethods(&rot_InterfaceType, py_ndr_rot_methods))
		return;

#ifdef PY_ROT_PATCH
	PY_ROT_PATCH(&rot_InterfaceType);
#endif

	m = Py_InitModule3("rot", rot_methods, "rot DCE/RPC");
	if (m == NULL)
		return;

	Py_INCREF((PyObject *)(void *)&rot_InterfaceType);
	PyModule_AddObject(m, "rot", (PyObject *)(void *)&rot_InterfaceType);
#ifdef PY_MOD_ROT_PATCH
	PY_MOD_ROT_PATCH(m);
#endif

}

#include <Python.h>
#include "diskstats.h"
#include "proc_fd.h"
#include "proc_pid.h"
#include "../common/includes.h"

#define MODULE_DOC "iXsystems procfs module"
static PyMethodDef ixprocfs_methods[] = { { .ml_name = NULL } };
static struct PyModuleDef moduledef = {
	PyModuleDef_HEAD_INIT,
	.m_name = "ixprocfs",
	.m_doc = MODULE_DOC,
	.m_size = -1,
	.m_methods = ixprocfs_methods,
};

PyObject* module_init(void)
{
	PyObject *m = NULL;
	m = PyModule_Create(&moduledef);
	if (m == NULL) {
		fprintf(stderr, "failed to initalize module\n");
		return NULL;
	}

	if (PyType_Ready(&PyDiskStats) < 0) {
		Py_DECREF(m);
		return NULL;
	}

	if (PyType_Ready(&PyDiskStatsEntry) < 0) {
		Py_DECREF(m);
		return NULL;
	}

	if (PyType_Ready(&PyProcFd) < 0) {
		Py_DECREF(m);
		return NULL;
	}

	if (PyType_Ready(&PyProcPid) < 0) {
		Py_DECREF(m);
		return NULL;
	}

	if (PyType_Ready(&PyPidEntry) < 0) {
		Py_DECREF(m);
		return NULL;
	}

	if (PyModule_AddObject(m, "DiskStats", (PyObject *)&PyDiskStats) < 0) {
		Py_DECREF(m);
		return NULL;
	}

	if (PyModule_AddObject(m, "ProcPid", (PyObject *)&PyProcPid) < 0) {
		Py_DECREF(m);
		return NULL;
	}

	return m;
}

PyMODINIT_FUNC PyInit_ixprocfs(void)
{
	return module_init();
}

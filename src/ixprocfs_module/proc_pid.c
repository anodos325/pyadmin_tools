/*check_open_path_state’
 * Python language bindings for libgfapi
 *
 * Copyright (C) Andrew Walker, 2022
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <Python.h>
#include "proc_pid.h"
#include "../utils/iter.h"
#include "../utils/parser.h"

static PyObject *py_pid_obj_new(PyTypeObject *obj,
			       PyObject *args_unused,
			       PyObject *kwargs_unused)
{
	py_proc_pid_t *self = NULL;

	self = (py_proc_pid_t *)obj->tp_alloc(obj, 0);
	if (self == NULL) {
		return NULL;
	}
	return (PyObject *)self;
}

static int py_pid_obj_init(PyObject *obj,
			  PyObject *args,
			  PyObject *kwargs)
{
	return 0;
}

void py_pid_obj_dealloc(py_proc_pid_t *self)
{
	Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *py_pid_getpid(PyObject *obj,
			       PyObject *args,
			       PyObject *kwargs_unused)
{
	int pid = -1;

	if (!PyArg_ParseTuple(args, "|i", &pid)) {
		return NULL;
	}

	if (pid == -1) {
		pid = getpid();
	}

	return init_pidstats(pid);
}

static PyMethodDef py_pid_obj_methods[] = {
	{
		.ml_name = "get_pid",
		.ml_meth = (PyCFunction)py_pid_getpid,
		.ml_flags = METH_VARARGS,
		.ml_doc = "Retrieve PidEntry by id"
	},
	{ NULL, NULL, 0, NULL }
};

static PyGetSetDef py_pid_obj_getsetters[] = {
	{ .name = NULL }
};

PyDoc_STRVAR(py_pid_handle__doc__,
"procfd handle\n"
);

PyTypeObject PyProcPid = {
	.tp_name = "ixprocfs.ProcPid",
	.tp_basicsize = sizeof(py_proc_pid_t),
	.tp_methods = py_pid_obj_methods,
	.tp_getset = py_pid_obj_getsetters,
	.tp_new = py_pid_obj_new,
	.tp_init = py_pid_obj_init,
	.tp_doc = py_pid_handle__doc__,
	.tp_dealloc = (destructor)py_pid_obj_dealloc,
	.tp_flags = Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,
};

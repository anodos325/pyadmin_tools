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
#include "proc_fd.h"
#include "../utils/iter.h"
#include "../utils/parser.h"

static PyObject *py_fd_obj_new(PyTypeObject *obj,
			       PyObject *args_unused,
			       PyObject *kwargs_unused)
{
	py_procfd_base_t *self = NULL;

	self = (py_procfd_base_t *)obj->tp_alloc(obj, 0);
	if (self == NULL) {
		return NULL;
	}
	return (PyObject *)self;
}

static int py_fd_obj_init(PyObject *obj,
			  PyObject *args,
			  PyObject *kwargs)
{
	return 0;
}

void py_fd_obj_dealloc(py_procfd_base_t *self)
{
	Py_TYPE(self)->tp_free((PyObject *)self);
}

PyDoc_STRVAR(py_fd_read__doc__,
"read()\n"
"--\n\n"
"Stat the glfs object. Performs fresh stat and updates\n"
"cache for object.\n\n"
"Parameters\n"
"----------\n"
"None\n\n"
"Returns\n"
"-------\n"
"stat_result\n"
);

struct path_entry {
	procfd_path_t path;
	struct stat st;
};

struct check_open_path_state {
	struct path_entry *paths;
	Py_ssize_t path_cnt;
	bool fast;
	iter_procfd_cb_t *wrapper; /* backpointer to callback */
	int (*strcmp_fn)(const char *s1, const char *s2);
	int (*strncmp_fn)(const char *s1, const char *s2, size_t n);
	PyObject *result;
};

static bool init_open_path_state(PyObject *path_list,
				 struct check_open_path_state *state)
{
	Py_ssize_t sz, i;

	if (!PyList_Check(path_list)) {
		PyErr_SetString(
			PyExc_TypeError,
			"Must be a list."
		);
		return false;
	}

	sz = PyList_Size(path_list);
	if (sz == 0) {
		return true;
	}

	state->paths = calloc(sz, sizeof(struct path_entry));
	if (state->paths == NULL) {
		PyErr_SetString(
			PyExc_MemoryError,
			"Failed to allocate paths array."
		);
		return false;
	}
	state->path_cnt = sz;

	for (i = 0; i < sz; i++) {
		PyObject *entry = NULL;
		Py_ssize_t entry_sz;
		const char *entry_str;

		entry = PyList_GetItem(path_list, i);

		if (entry == NULL) {
			free(state->paths);
			return false;
		}

		if (!PyUnicode_Check(entry)) {
			PyErr_SetString(
				PyExc_TypeError,
				"List entries must be strings."
			);
			free(state->paths);
			return false;
		}

		entry_str = PyUnicode_AsUTF8AndSize(entry, &entry_sz);
		if (entry_str == NULL) {
			free(state->paths);
			return false;
		}

		if (entry_sz > (Py_ssize_t)sizeof(procfd_path_t)) {
			PyErr_SetString(
				PyExc_ValueError,
				"Path string too long."
			);
			free(state->paths);
			return false;
		}

		if (stat(entry_str, &state->paths[i].st) != 0) {
			PyErr_SetString(
				PyExc_RuntimeError,
				"stat() failed"
			);
			free(state->paths);
			return false;
		}

		strlcpy(state->paths[i].path, entry_str, sizeof(procfd_path_t));
	}

	return true;
}

static bool path_check(struct check_open_path_state *state,
		       procfd_path_t path)
{
	Py_ssize_t i;

	if (state->path_cnt == 0) {
		return false;
	}

	for (i = 0; i < state->path_cnt; i++) {
		struct path_entry *entry = NULL;
		entry = &state->paths[i];

		if (S_ISDIR(entry->st.st_mode)) {
			if (state->strncmp_fn(entry->path, path, strlen(entry->path)) == 0) {
				return true;
			}
		} else {
			if (state->strcmp_fn(entry->path, path) == 0) {
				return true;
			}
		}
	}

	return false;
}

static bool format_output_impl(struct check_open_path_state *state,
			       const char *proc_fd_path,
			       procfd_info_t *info)
{
	PyObject *entry = NULL;

	entry = Py_BuildValue(
		"{s:s,s:s,s:s}",
		"procfd_path", proc_fd_path,
		"file_name", info->readlink,
		"pid_path", state->wrapper->_dir_internal
	);

	if (entry == NULL) {
		return false;
	}

	if (PyList_Append(state->result, entry) != 0) {
		Py_DECREF(entry);
		return false;
	}

	return true;
}

static int check_open_path_impl(const char *proc_fd_path, procfd_info_t *info, void *priv)
{
	struct check_open_path_state *state = (struct check_open_path_state *)priv;
	bool ok;

	if (!path_check(state, info->readlink)) {
		return ITER_STATE_CONTINUE;
	}

	ITER_END_ALLOW_THREADS(state->wrapper);
	ok = format_output_impl(state, proc_fd_path, info);
	ITER_ALLOW_THREADS(state->wrapper);

	if (!ok) {
		return ITER_STATE_ERROR;
	}

	if (state->fast) {
		return ITER_STATE_BREAK;
	}

	return ITER_STATE_CONTINUE;
}

static PyObject *py_fd_check_open_path(PyObject *obj,
				       PyObject *args,
				       PyObject *kwargs)
{
	PyObject *pypaths = NULL;
	int rv;
	bool case_insensitive = false, do_stat = false;
	struct check_open_path_state state = { .fast = true, .paths = NULL };
	iter_procfd_cb_t cb = {
		.fn = check_open_path_impl,
		.state = &state,
		.desired_info = PROCFD_INFO_READLINK
	};
	iter_procfd_cb_t *cbp = &cb;
	const char *kwnames [] = {
		"paths_to_check",
		"fast",
		"case_insensitive",
		"do_stat",
		NULL
	};

	state.wrapper = &cb;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs,
					 "O|bbb",
					 discard_const_p(char *, kwnames),
					 &pypaths,
					 &state.fast,
					 &case_insensitive,
					 &do_stat)) {
		return NULL;
	}

	if (case_insensitive) {
		state.strcmp_fn = strcasecmp;
		state.strncmp_fn = strncasecmp;
	} else {
		state.strcmp_fn = strcmp;
		state.strncmp_fn = strncmp;
	}

	if (do_stat) {
		cb.desired_info |= PROCFD_INFO_STAT;
	}

	if (!init_open_path_state(pypaths, &state)) {
		return NULL;
	}

	state.result = Py_BuildValue("[]");
	if (state.result == NULL) {
		free(state.paths);
		return NULL;
	}
	ITER_ALLOW_THREADS(cbp);
	rv = iter_proc_fd_paths(NULL, &cb);
	ITER_END_ALLOW_THREADS(cbp);

	if (rv == ITER_STATE_ERROR) {
		return NULL;
	}

	return state.result;
}

static PyMethodDef py_fd_obj_methods[] = {
	{
		.ml_name = "check_open_paths",
		.ml_meth = (PyCFunction)py_fd_check_open_path,
		.ml_flags = METH_VARARGS | METH_KEYWORDS,
		.ml_doc = py_fd_read__doc__
	},
	{ NULL, NULL, 0, NULL }
};

static PyGetSetDef py_fd_obj_getsetters[] = {
	{ .name = NULL }
};


PyDoc_STRVAR(py_procfd_handle__doc__,
"procfd handle\n"
);

PyTypeObject PyProcFd = {
	.tp_name = "ixprocfs.ProcFd",
	.tp_basicsize = sizeof(py_procfd_base_t),
	.tp_methods = py_fd_obj_methods,
	.tp_getset = py_fd_obj_getsetters,
	.tp_new = py_fd_obj_new,
	.tp_init = py_fd_obj_init,
	.tp_doc = py_procfd_handle__doc__,
	.tp_dealloc = (destructor)py_fd_obj_dealloc,
	.tp_flags = Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,
};

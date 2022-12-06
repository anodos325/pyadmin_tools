/*
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
#include "diskstats.h"
#include "../utils/iter.h"
#include "../utils/parser.h"

static PyObject *py_ds_obj_new(PyTypeObject *obj,
			       PyObject *args_unused,
			       PyObject *kwargs_unused)
{
	py_diskstats_t *self = NULL;

	self = (py_diskstats_t *)obj->tp_alloc(obj, 0);
	if (self == NULL) {
		return NULL;
	}
	return (PyObject *)self;
}

static int py_ds_obj_init(PyObject *obj,
			  PyObject *args,
			  PyObject *kwargs)
{
	py_diskstats_t *self = (py_diskstats_t *)obj;

	self->stats_file = fopen(DISKSTATS_PATH, "r");
	if (self->stats_file == NULL) {
		PyErr_SetString(
			PyExc_RuntimeError,
			"Failed to open stats file."
		);
		return -1;
	}
	self->stats = calloc(100, sizeof(diskstats_t));
	if (self->stats == NULL) {
		fclose(self->stats_file);
		self->stats_file = NULL;
		PyErr_SetString(
			PyExc_MemoryError,
			"Failed to allocate stats array."
		);
		return -1;
	}
	self->stats_alloc = 100;
	self->stats_cnt = 0;
	return 0;
}

void py_ds_obj_dealloc(py_diskstats_t *self)
{
	if (self->stats_file) {
		fclose(self->stats_file);
		self->stats_file = NULL;
	}
	free(self->stats);
	self->stats_alloc = 0;
	Py_TYPE(self)->tp_free((PyObject *)self);
}

static inline bool parse_major(char *token, diskstats_t *stat)
{
	return parse_major_minor(token, &stat->major);
}

static inline bool parse_minor(char *token, diskstats_t *stat)
{
	return parse_major_minor(token, &stat->minor);
}

static inline bool parse_dev_name(char *token, diskstats_t *stat)
{
	strlcpy(stat->name, token, sizeof(stat->name));
	return true;
}

static inline bool parse_reads_completed(char *token, diskstats_t *stat)
{
	return parse_ulong(token, &stat->reads_completed);
}

static inline bool parse_reads_merged(char *token, diskstats_t *stat)
{
	return parse_ulong(token, &stat->reads_merged);
}

static inline bool parse_sectors_read(char *token, diskstats_t *stat)
{
	return parse_ulong(token, &stat->sectors_read);
}

static inline bool parse_reading_ms(char *token, diskstats_t *stat)
{
	return parse_uint(token, &stat->time_reading_ms);
}

static inline bool parse_writes_completed(char *token, diskstats_t *stat)
{
	return parse_ulong(token, &stat->writes_completed);
}

static inline bool parse_writes_merged(char *token, diskstats_t *stat)
{
	return parse_ulong(token, &stat->writes_merged);
}

static inline bool parse_sectors_written(char *token, diskstats_t *stat)
{
	return parse_ulong(token, &stat->sectors_written);
}

static inline bool parse_writing_ms(char *token, diskstats_t *stat)
{
	return parse_uint(token, &stat->time_writing_ms);
}

static inline bool parse_ios_in_progress(char *token, diskstats_t *stat)
{
	return parse_uint(token, &stat->num_ios_in_progress);
}

static inline bool parse_time_doing_ios(char *token, diskstats_t *stat)
{
	return parse_uint(token, &stat->time_doing_ios_ms);
}

static inline bool parse_weighted_time_doing_ios(char *token, diskstats_t *stat)
{
	return parse_uint(token, &stat->weighted_time_doing_ios_ms);
}

static inline bool parse_discards_completed(char *token, diskstats_t *stat)
{
	return parse_ulong(token, &stat->discards_completed);
}

static inline bool parse_discards_merged(char *token, diskstats_t *stat)
{
	return parse_ulong(token, &stat->discards_merged);
}

static inline bool parse_sectors_discarded(char *token, diskstats_t *stat)
{
	return parse_ulong(token, &stat->sectors_discarded);
}

static inline bool parse_time_spent_discarding(char *token, diskstats_t *stat)
{
	return parse_uint(token, &stat->time_spent_discarding_ms);
}

static inline bool parse_flush_requests_completed(char *token, diskstats_t *stat)
{
	return parse_ulong(token, &stat->flush_requests_completed);
}

static inline bool parse_time_spent_flushing(char *token, diskstats_t *stat)
{
	return parse_uint(token, &stat->time_spent_flushing_ms);
}

static const struct {
	bool (*fn)(char *token, diskstats_t *statp);
} functable[] = {
	{ parse_major },
	{ parse_minor },
	{ parse_dev_name },
	{ parse_reads_completed },
	{ parse_reads_merged },
	{ parse_sectors_read },
	{ parse_reading_ms },
	{ parse_writes_completed },
	{ parse_writes_merged },
	{ parse_sectors_written },
	{ parse_writing_ms },
	{ parse_ios_in_progress },
	{ parse_time_doing_ios },
	{ parse_weighted_time_doing_ios },
	{ parse_discards_completed },
	{ parse_discards_merged },
	{ parse_sectors_discarded },
	{ parse_time_spent_discarding },
	{ parse_flush_requests_completed },
	{ parse_time_spent_flushing },
};

int parse_disk_line(char *token, int idx, void *state)
{
	py_diskstats_t *self = (py_diskstats_t *)state;
	diskstats_t *stats = &self->stats[self->stats_cnt - 1];

	if (idx > (int)ARRAY_SIZE(functable)) {
		errno = ERANGE;
		return ITER_STATE_ERROR;
	}

	if (!functable[idx].fn(token, stats)) {
		return ITER_STATE_ERROR;
	}

	return ITER_STATE_CONTINUE;
}

int read_disk_line(char *line, int idx, ssize_t line_len, void *state)
{
	py_diskstats_t *self = (py_diskstats_t *)state;
	iter_line_cb_t cb = {
		.fn = parse_disk_line,
		.state = self,
	};

	if (idx == self->stats_alloc) {
		diskstats_t *new = NULL;
		new = realloc(self->stats, (self->stats_alloc * 4 * sizeof(diskstats_t)));
		if (new == NULL) {
			return ITER_STATE_ERROR;
		}

		self->stats = new;
	}

	self->stats_cnt = idx + 1;
	return iter_line(line, " ", &cb);
}

int read_disk_stats_impl(py_diskstats_t *self)
{
	iter_file_cb_t cb = {
		.fn = read_disk_line,
		.state = self,
	};

	self->stats_cnt = 0;
	return iter_file(self->stats_file, &cb);
}

PyDoc_STRVAR(py_ds_read__doc__,
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

static PyObject *diskstats_to_py_diskstats(py_diskstats_t *self)
{
	PyObject *out = NULL;
	int i;

	out = PyList_New(self->stats_cnt);
	if (out == NULL) {
		return NULL;
	}

	for (i = 0; i < self->stats_cnt; i++) {
		PyObject *entry = NULL;

		entry = init_diskstats(&self->stats[i]);
		if (entry == NULL) {
			Py_DECREF(out);
			return NULL;
		}

		if (PyList_SetItem(out, i, entry)) {
			Py_DECREF(out);
			return NULL;
		}
	}

	return out;
}

static PyObject *py_ds_obj_read(PyObject *obj,
				PyObject *args_unused,
				PyObject *kwargs_unused)
{
	py_diskstats_t *self = (py_diskstats_t *)obj;
	int rv;

	Py_BEGIN_ALLOW_THREADS
	rv = read_disk_stats_impl(self);
	Py_END_ALLOW_THREADS

	if (rv == ITER_STATE_ERROR) {
		return NULL;
	}

	return diskstats_to_py_diskstats(self);
}

static PyMethodDef py_ds_obj_methods[] = {
	{
		.ml_name = "read_data",
		.ml_meth = (PyCFunction)py_ds_obj_read,
		.ml_flags = METH_NOARGS,
		.ml_doc = py_ds_read__doc__
	},
	{ NULL, NULL, 0, NULL }
};

static PyGetSetDef py_ds_obj_getsetters[] = {
	{ .name = NULL }
};


PyDoc_STRVAR(py_diskstats_handle__doc__,
"GLFS object handle\n"
"This handle provides methods to work with glusster objects (files and\n"
"directories), instead of absolute paths.\n"
"The intention using glfs objects is to operate based on parent parent\n"
"object and looking up or creating objects within, OR to be used on the\n"
"actual object thus looked up or created, and retrieve information regarding\n"
"the same.\n"
"The object handles may be used to open / create a gluster FD object for\n"
"gluster FD - based operations\n"
"The object handle is automatically closed when it is deallocated.\n"
);

PyTypeObject PyDiskStats = {
	.tp_name = "ixprocfs.DiskStats",
	.tp_basicsize = sizeof(py_diskstats_t),
	.tp_methods = py_ds_obj_methods,
	.tp_getset = py_ds_obj_getsetters,
	.tp_new = py_ds_obj_new,
	.tp_init = py_ds_obj_init,
	.tp_doc = py_diskstats_handle__doc__,
	.tp_dealloc = (destructor)py_ds_obj_dealloc,
	.tp_flags = Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,
};

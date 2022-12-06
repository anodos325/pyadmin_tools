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

static PyObject *py_dse_obj_new(PyTypeObject *obj,
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

static int py_dse_obj_init(PyObject *obj,
			   PyObject *args,
			   PyObject *kwargs)
{
	return 0;
}

static PyObject *py_dse_obj_counters(PyObject *obj,
				     PyObject *args_unused,
				     PyObject *kwargs_unused)
{
	py_diskstats_entry_t *self = (py_diskstats_entry_t *)obj;
	return Py_BuildValue(
		"(IIskkkIkkkIIIIIkkkIkI)",
		self->stat.major,
		self->stat.minor,
		self->stat.name,
		self->stat.reads_completed,
		self->stat.reads_merged,
		self->stat.sectors_read,
		self->stat.time_reading_ms,
		self->stat.writes_completed,
		self->stat.writes_merged,
		self->stat.sectors_written,
		self->stat.time_writing_ms,
		self->stat.num_ios_in_progress,
		self->stat.time_doing_ios_ms,
		self->stat.weighted_time_doing_ios_ms,
		self->stat.discards_completed,
		self->stat.discards_merged,
		self->stat.sectors_discarded,
		self->stat.time_spent_discarding_ms,
		self->stat.flush_requests_completed,
		self->stat.time_spent_flushing_ms
	);
}

static PyObject *py_dse_obj_counters_dict(PyObject *obj,
					  PyObject *args_unused,
					  PyObject *kwargs_unused)
{
	py_diskstats_entry_t *self = (py_diskstats_entry_t *)obj;
	return Py_BuildValue(
		"{sIsIsssksksksIsksksksIsIsIsIsIsksksksIsksI}",
		"major", self->stat.major,
		"minor", self->stat.minor,
		"device_name", self->stat.name,
		"reads_completed", self->stat.reads_completed,
		"reads_merged", self->stat.reads_merged,
		"sectors_read", self->stat.sectors_read,
		"time_reading_ms", self->stat.time_reading_ms,
		"writes_completed", self->stat.writes_completed,
		"writes_merged", self->stat.writes_merged,
		"sectors_written", self->stat.sectors_written,
		"time_writing_ms", self->stat.time_writing_ms,
		"num_ios_in_progress", self->stat.num_ios_in_progress,
		"time_doing_ios_ms", self->stat.time_doing_ios_ms,
		"weighted_time_doing_ios_ms", self->stat.weighted_time_doing_ios_ms,
		"discards_completed", self->stat.discards_completed,
		"discards_merged", self->stat.discards_merged,
		"sectors_discarded", self->stat.sectors_discarded,
		"discarding_ms", self->stat.time_spent_discarding_ms,
		"requests_completed", self->stat.flush_requests_completed,
		"time_spent_flushing_ms", self->stat.time_spent_flushing_ms
	);
}

static PyObject *py_dse_obj_name(PyObject *obj, void *closure)
{
	py_diskstats_entry_t *self = (py_diskstats_entry_t *)obj;
        return Py_BuildValue("s", self->stat.name);
}

static PyMethodDef py_dse_obj_methods[] = {
	{
		.ml_name = "counters_tuple",
		.ml_meth = (PyCFunction)py_dse_obj_counters,
		.ml_flags = METH_NOARGS,
		.ml_doc = "diskstats as tuple"
	},
	{
		.ml_name = "counters_dict",
		.ml_meth = (PyCFunction)py_dse_obj_counters_dict,
		.ml_flags = METH_NOARGS,
		.ml_doc = "diskstats as dict"
	},
	{ NULL, NULL, 0, NULL }
};

static PyGetSetDef py_dse_obj_getsetters[] = {
	{
		.name	= discard_const_p(char, "device_name"),
		.get	= (getter)py_dse_obj_name,
		.doc	= "device name",
	},
	{ .name = NULL }
};

static PyObject *py_dse_obj_repr(PyObject *obj)
{
	py_diskstats_entry_t *self = (py_diskstats_entry_t *)obj;
	return PyUnicode_FromFormat(
		"ixprocfs.DiskStatsEntry(major=%u, minor:%u, device_name=%s)",
		self->stat.major, self->stat.minor, self->stat.name
        );
}

PyObject *init_diskstats(diskstats_t *stats_in)
{
	py_diskstats_entry_t *out = NULL;
	out = PyObject_New(py_diskstats_entry_t, &PyDiskStatsEntry);
	if (out == NULL) {
		return NULL;
	}
	memcpy(&out->stat, stats_in, sizeof(diskstats_t));
	return (PyObject *)out;
}

PyTypeObject PyDiskStatsEntry = {
	.tp_name = "ixprocfs.DiskStatsEntry",
	.tp_basicsize = sizeof(py_diskstats_entry_t),
	.tp_methods = py_dse_obj_methods,
	.tp_getset = py_dse_obj_getsetters,
	.tp_new = py_dse_obj_new,
	.tp_init = py_dse_obj_init,
	.tp_repr = py_dse_obj_repr,
	.tp_doc = "Diskstats entry object",
	.tp_flags = Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,
};

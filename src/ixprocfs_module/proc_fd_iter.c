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
#include "proc_pid.h"
#include "proc_fd.h"
#include "../utils/iter.h"
#include "../utils/parser.h"

static void set_exc_from_errno(const char *func)
{
        PyErr_Format(
                PyExc_RuntimeError,
                "%s failed: %s", func, strerror(errno)
        );
}

static int _iter_procfds_cb(struct dirent *entry, void *priv)
{
	iter_procfd_cb_t *cb = NULL;
	procfd_info_t info;
	uint fd;
	char path[PATH_MAX];

	cb = (iter_procfd_cb_t *)priv;

	if ((strcmp(entry->d_name, "0") == 0) ||
	    (strcmp(entry->d_name, "1") == 0) ||
	    (strcmp(entry->d_name, "2") == 0)) {
		return ITER_STATE_CONTINUE;
	}
	if (!parse_uint(entry->d_name, &fd)) {
		return ITER_STATE_ERROR;
	}

	cb->_cnt_internal++;

	info = (procfd_info_t) {
		.fd = fd
	};

	snprintf(path, sizeof(path), "%s/%s", cb->_dir_internal, entry->d_name);

	if (cb->desired_info & PROCFD_INFO_READLINK) {
		ssize_t sz;

		sz = readlink(path, info.readlink, sizeof(info.readlink));
		if (sz == -1) {
			ITER_END_ALLOW_THREADS(cb);
			PyErr_Format(
				PyExc_RuntimeError,
				"%s: readlink() failed: %s",
				entry->d_name, strerror(errno)
			);
			ITER_ALLOW_THREADS(cb);
			return ITER_STATE_ERROR;
		}
		info.valid_data |= PROCFD_INFO_READLINK;
	}

	if (cb->desired_info & PROCFD_INFO_STAT) {
		if (stat(entry->d_name, &info.st) == -1) {
			ITER_END_ALLOW_THREADS(cb);
			PyErr_Format(
				PyExc_RuntimeError,
				"%s: stat() failed: %s",
				entry->d_name, strerror(errno)
			);
			ITER_ALLOW_THREADS(cb);
			return ITER_STATE_ERROR;
		}
		info.valid_data |= PROCFD_INFO_STAT;
	}

        return cb->fn(path, &info, cb->state);
}

/*
 * This callback gets called by iterator for "/proc" and provides
 * "/proc/<pid>" directories. In this function we need to open
 * the "/proc/<pid>/fd" directory and iterate its contents to check
 * pass on the procfd paths "/proc/<pid>/fd/<fd>".
 */
static int __iter_pid_cb(const char *pid_path, pid_t pid, void *state)
{
	int rv;
	DIR *base = NULL;
	char path[PATH_MAX];
	iter_procfd_cb_t *cb_in = (iter_procfd_cb_t *)state;

	cb_in->_dir_internal = path;
	cb_in->_pid_internal = pid;
	cb_in->_cnt_internal = 0;

	iter_dir_cb_t cb = {
		.fn = _iter_procfds_cb,
		._save = cb_in->_save,
		.state = cb_in,
	};
	snprintf(path, sizeof(procfd_path_t), "%s/fd", pid_path);

	// TODO: add filters here for pids

	base = opendir(path);
	if (base == NULL) {
		ITER_END_ALLOW_THREADS(cb_in);
		PyErr_Format(
			PyExc_RuntimeError,
			"%s: opendir() failed: %s",
			path, strerror(errno)
		);
		ITER_ALLOW_THREADS(cb_in);
		return ITER_STATE_ERROR;
	}

	rv = iter_dir(base, &cb);
	closedir(base);

	/*
	 * allow ITER_STATE_BREAK to stop iterating pid
	 * and move on to next one
	 */
	if (rv == ITER_STATE_BREAK) {
		rv = ITER_STATE_CONTINUE;
	}
	return rv;
}

int iter_proc_fd_paths(struct pid_list *pids, iter_procfd_cb_t *cb_in)
{
	iter_proc_pid_cb_t cb = {
		.fn = __iter_pid_cb,
		.pids = pids,
		._save = cb_in->_save,
		.state = cb_in
	};

	return iter_proc_pids(&cb);
}

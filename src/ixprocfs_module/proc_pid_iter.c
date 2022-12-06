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
#include "../utils/iter.h"
#include "../utils/parser.h"

/*
 * The following two functions are for iterating contents of
 * /proc directory and calling API-user provided callback function
 * with the ("/proc/<pid>", <pid>, <void *>) for every pid in the
 * directory.
 */
static int __iter_proc_pid_paths_impl(struct dirent *entry, void *state)
{
	iter_proc_pid_cb_t *cb = (iter_proc_pid_cb_t *)state;
	int pid;
	char procfd_path[PATH_MAX];

	if (!parse_int(entry->d_name, &pid)) {
		return ITER_STATE_CONTINUE;
	}

	if (cb->pids) {
		size_t i;
		bool found = false;

		for (i = 0; i < cb->pids->cnt; i++) {
			if (cb->pids->pids[i] == pid) {
				found = true;
				break;
			}
		}

		if (!found) {
			return ITER_STATE_CONTINUE;
		}
	}

	snprintf(procfd_path, sizeof(procfd_path), "/proc/%s", entry->d_name);
	return cb->fn(procfd_path, (pid_t)pid, cb->state);
}

int iter_proc_pids(iter_proc_pid_cb_t *cb_in)
{
	int rv;
	DIR *base = NULL;
	iter_dir_cb_t cb = {
		.fn = __iter_proc_pid_paths_impl,
		._save = cb_in->_save,
		.state = cb_in,
	};

	base = opendir("/proc");
	if (base == NULL) {
		ITER_END_ALLOW_THREADS(cb_in);
		PyErr_Format(
			PyExc_RuntimeError,
			"/proc: opendir() failed: %s", strerror(errno)
		);
		ITER_ALLOW_THREADS(cb_in);
		return ITER_STATE_ERROR;
	}

	rv = iter_dir(base, &cb);
	closedir(base);
	return rv;
}

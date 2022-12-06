/*
 * Python language bindings for procfs-diskstats
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

#ifndef _PROCFD_H_
#define _PROCFD_H_
#include "proc_pid.h"

extern PyTypeObject PyProcFd;

#define PROCFD_INFO_READLINK 0x01
#define PROCFD_INFO_STAT 0x02

typedef char procfd_path_t[PATH_MAX];

typedef struct {
	PyObject_HEAD
} py_procfd_base_t;

typedef struct {
	uint fd;
	procfd_path_t readlink;
	struct stat st;
	int valid_data;
} procfd_info_t;

typedef struct {
        PyThreadState *_save;
	const char *_dir_internal; /* stack in iterator "/proc/<pid>" path */
	uint _pid_internal;
	int _cnt_internal; /* internal pid fd counter */
	int (*fn)(const char *proc_fd_path, procfd_info_t *info,  void *state);
	procfd_info_t data_out;
	int desired_info;
	void *state;
} iter_procfd_cb_t;

extern int iter_proc_fd_paths(struct pid_list *, iter_procfd_cb_t *);
#endif /* _PROCFD_H_ */

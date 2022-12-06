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

#ifndef _PROC_PID_H_
#define _PROC_PID_H_

#include <Python.h>
#include "../common/includes.h"
/* proc_pid.c */
typedef struct {
	PyObject_HEAD
} py_proc_pid_t;

/* proc_pid_iter.c */
struct pid_list {
	pid_t *pids;
	size_t cnt;
};

typedef struct {
	PyThreadState *_save;
	struct pid_list *pids;
	int (*fn)(const char *proc_pid_path, pid_t pid, void *state);
	void *state;
} iter_proc_pid_cb_t;

extern int iter_proc_pids(iter_proc_pid_cb_t *);

/* proc_pid_parse.c */
typedef struct procfs_pid_stat {
	int pid;
	char comm[18]; /* TASK_COMM_LEN + 2 */
	char state;
	int ppid;
	int pgrp;
	int session;
	int tty_nr;
	int tpgid;
	uint flags;
	unsigned long minflt;
	unsigned long cminflt;
	unsigned long majflt;
	unsigned long cmajflt;
	unsigned long utime;
	unsigned long stime;
	long cutime;
	long cstime;
	long priority;
	long nice;
	long num_threads;
	// long itrealvalue; /* hardcoded as zero */
	unsigned long long starttime;
	unsigned long vsize;
	long rss; /* inaccurate */
	unsigned long rsslim;
	unsigned long startcode; /* PT */
	unsigned long endcode; /* PT */
	unsigned long startstack; /* PT */
	unsigned long kstkesp; /* PT */
	unsigned long kstkeip; /* PT */
	// unsigned long signal; /* obsolete */
	// unsigned long blocked; /* obsolete */
	// unsigned long sigignore; /* obsolete */
	// unsigned long sigcatch; /* obsolete */
	unsigned long wchan;
	// unsigned long nswap; /* not maintained */
	// unsigned long cnswap; /* not maintained */
	int exit_signal;
	int processor;
	unsigned rt_priority;
	unsigned policy;
	unsigned long long delayacct_blkio_ticks;
	unsigned long guest_time;
	long cguest_time;
	unsigned long start_data; /* PT */
	unsigned long end_data; /* PT */
	unsigned long start_brk; /* PT */
	unsigned long arg_start; /* PT */
	unsigned long arg_end; /* PT */
	unsigned long env_start; /* PT */
	unsigned long env_end; /* PT */
	int exit_code;
} pidstat_t;

typedef struct procfs_pid_statm {
	unsigned long size;
	unsigned long resident;
	unsigned long shared;
	// TODO uint text;
	// library /* unused since Linux 2.6 */
	unsigned long data;
	// dt /* unused since Linux 2.6 */
} pidstatm_t;

typedef struct {
	PyObject_HEAD
	pid_t pid;
	pidstat_t pidstat;
	pidstatm_t pidstatm;
} py_proc_pid_entry_t;

extern PyTypeObject PyProcPid;
extern PyTypeObject PyPidEntry;
extern int read_pid_stats(FILE *statsfile, pidstat_t *stats_out);
extern int read_pid_statm(FILE *statsfile, pidstatm_t *stats_out);
extern PyObject *init_pidstats(pid_t pid);
#endif /* _PROC_PID_H_ */

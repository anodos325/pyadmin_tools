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
 * /proc/<pid>/stat parser
 */

static inline bool parse_skip(char *token, pidstat_t *statp)
{
	/*
	 * kernel stats have deprecated / obsolete fields.
	 * we skip setting them, but have to have function
	 * in our functables to avoid more complex logic
	 * in iterator.
	 */
	return true;
}

static inline bool parse_pid(char *token, pidstat_t *statp)
{
	return parse_int(token, &statp->pid);
}

static inline bool parse_comm(char *token, pidstat_t *statp)
{
	strlcpy(statp->comm, token, sizeof(statp->comm));
	return true;
}

static inline bool parse_state(char *token, pidstat_t *statp)
{
	statp->state = *token;
	return true;
}

static inline bool parse_ppid(char *token, pidstat_t *statp)
{
	return parse_int(token, &statp->ppid);
}

static inline bool parse_pgrp(char *token, pidstat_t *statp)
{
	return parse_int(token, &statp->pgrp);
}

static inline bool parse_session(char *token, pidstat_t *statp)
{
	return parse_int(token, &statp->session);
}

static inline bool parse_tty_nr(char *token, pidstat_t *statp)
{
	return parse_int(token, &statp->tty_nr);
}

static inline bool parse_tpgid(char *token, pidstat_t *statp)
{
	return parse_int(token, &statp->tpgid);
}

static inline bool parse_flags(char *token, pidstat_t *statp)
{
	return parse_uint(token, &statp->flags);
}

static inline bool parse_minflt(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->minflt);
}

static inline bool parse_cminflt(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->cminflt);
}

static inline bool parse_majflt(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->majflt);
}

static inline bool parse_cmajflt(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->cmajflt);
}

static inline bool parse_utime(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->utime);
}

static inline bool parse_stime(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->stime);
}

static inline bool parse_cutime(char *token, pidstat_t *statp)
{
	return parse_long(token, &statp->cutime);
}

static inline bool parse_cstime(char *token, pidstat_t *statp)
{
	return parse_long(token, &statp->cstime);
}

static inline bool parse_priority(char *token, pidstat_t *statp)
{
	return parse_long(token, &statp->priority);
}

static inline bool parse_nice(char *token, pidstat_t *statp)
{
	return parse_long(token, &statp->nice);
}

static inline bool parse_num_threads(char *token, pidstat_t *statp)
{
	return parse_long(token, &statp->num_threads);
}

static inline bool parse_starttime(char *token, pidstat_t *statp)
{
	return parse_ulonglong(token, &statp->starttime);
}

static inline bool parse_vsize(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->vsize);
}

static inline bool parse_rss(char *token, pidstat_t *statp)
{
	return parse_long(token, &statp->rss);
}

static inline bool parse_rsslim(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->rsslim);
}

static inline bool parse_startcode(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->startcode);
}

static inline bool parse_endcode(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->endcode);
}

static inline bool parse_startstack(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->startstack);
}

static inline bool parse_kstkesp(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->kstkesp);
}

static inline bool parse_kstkeip(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->kstkeip);
}

static inline bool parse_wchan(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->wchan);
}

static inline bool parse_exit_signal(char *token, pidstat_t *statp)
{
	return parse_int(token, &statp->exit_signal);
}

static inline bool parse_processor(char *token, pidstat_t *statp)
{
	return parse_int(token, &statp->processor);
}

static inline bool parse_rt_priority(char *token, pidstat_t *statp)
{
	return parse_uint(token, &statp->rt_priority);
}

static inline bool parse_policy(char *token, pidstat_t *statp)
{
	return parse_uint(token, &statp->policy);
}

static inline bool parse_delayacct_blkio_ticks(char *token, pidstat_t *statp)
{
	return parse_ulonglong(token, &statp->delayacct_blkio_ticks);
}

static inline bool parse_guest_time(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->guest_time);
}

static inline bool parse_cguest_time(char *token, pidstat_t *statp)
{
	return parse_long(token, &statp->cguest_time);
}

static inline bool parse_start_data(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->start_data);
}

static inline bool parse_end_data(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->end_data);
}

static inline bool parse_start_brk(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->start_brk);
}

static inline bool parse_arg_start(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->arg_start);
}

static inline bool parse_arg_end(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->arg_end);
}

static inline bool parse_env_start(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->env_start);
}

static inline bool parse_env_end(char *token, pidstat_t *statp)
{
	return parse_ulong(token, &statp->env_end);
}

static inline bool parse_exit_code(char *token, pidstat_t *statp)
{
	if (strlen(token) != 2) {
		abort();
	}
	token[1] = '\0';
	return parse_int(token, &statp->exit_code);
}

static const struct {
	bool (*fn)(char *token, pidstat_t *statp);
} pidstat_functable[] = {
	{ parse_pid },
	{ parse_comm },
	{ parse_state },
	{ parse_ppid },
	{ parse_pgrp },
	{ parse_session },
	{ parse_tty_nr },
	{ parse_tpgid },
	{ parse_flags },
	{ parse_minflt },
	{ parse_cminflt },
	{ parse_majflt },
	{ parse_cmajflt },
	{ parse_utime },
	{ parse_stime },
	{ parse_cutime },
	{ parse_cstime },
	{ parse_priority },
	{ parse_nice },
	{ parse_num_threads },
	{ parse_skip }, /* itrealvalue */
	{ parse_starttime },
	{ parse_vsize },
	{ parse_rss },
	{ parse_rsslim },
	{ parse_startcode },
	{ parse_endcode },
	{ parse_startstack },
	{ parse_kstkesp },
	{ parse_kstkeip },
	{ parse_skip }, /* signal */
	{ parse_skip }, /* blocked */
	{ parse_skip }, /* sigignore */
	{ parse_skip }, /* sigcatch */
	{ parse_wchan },
	{ parse_skip }, /* nswap */
	{ parse_skip }, /* cnswap */
	{ parse_exit_signal },
	{ parse_processor },
	{ parse_rt_priority },
	{ parse_policy },
	{ parse_delayacct_blkio_ticks },
	{ parse_guest_time },
	{ parse_cguest_time },
	{ parse_start_data },
	{ parse_end_data },
	{ parse_start_brk },
	{ parse_arg_start },
	{ parse_arg_end },
	{ parse_env_start },
	{ parse_env_end },
	{ parse_exit_code },
};

struct stat_state {
	pidstat_t *stats;
	iter_error_t err;
};

static int parse_pidstats_line(char *token, int idx, void *state)
{
	struct stat_state *st = (struct stat_state *)state;

        if (idx > (int)ARRAY_SIZE(pidstat_functable)) {
		abort();
        }

        if (!pidstat_functable[idx].fn(token, st->stats)) {
		st->err.saved_errno = errno;
		snprintf(st->err.errstr, sizeof(st->err.errstr),
			 "%s: failed to parse value at idx %d in line",
			 token, idx);
                return ITER_STATE_ERROR;
        }

        return ITER_STATE_CONTINUE;
}

static int read_pidstats_line(char *line, int idx, ssize_t line_len, void *state)
{
	iter_line_cb_t cb = {
		.fn = parse_pidstats_line,
		.state = state,
	};

	return iter_line(line, " ", &cb);
}

int read_pid_stats(FILE *statsfile, pidstat_t *stats)
{
	int rv;
	struct stat_state state = {
		.stats = stats
	};
	iter_file_cb_t cb = {
		.fn = read_pidstats_line,
		.state = &state,
	};

	Py_BEGIN_ALLOW_THREADS
	rewind(statsfile);
	rv = iter_file(statsfile, &cb);
	Py_END_ALLOW_THREADS

	if (rv == ITER_STATE_ERROR) {
		PyErr_Format(
			PyExc_RuntimeError,
			"read_pid_stats(): %s: %s",
			cb.err.errstr,
			strerror(cb.err.saved_errno)
		);
	}
	return rv;
}

/*
 * /proc/<pid>/statm parser
 */

static inline bool parse_skipm(char *token, pidstatm_t *statp)
{
	return true;
}

static inline bool parse_size(char *token, pidstatm_t *statp)
{
	return parse_ulong(token, &statp->size);
}

static inline bool parse_resident(char *token, pidstatm_t *statp)
{
	return parse_ulong(token, &statp->resident);
}

static inline bool parse_shared(char *token, pidstatm_t *statp)
{
	return parse_ulong(token, &statp->shared);
}

static inline bool parse_data(char *token, pidstatm_t *statp)
{
	return parse_ulong(token, &statp->data);
}

static const struct {
	bool (*fn)(char *token, pidstatm_t *statp);
} pidstatm_functable[] = {
	{ parse_size },
	{ parse_resident },
	{ parse_shared },
	{ parse_skipm }, /* text */
	{ parse_skipm }, /* library */
	{ parse_data },
	{ parse_skipm }, /* dt */
};

struct statm_state {
	pidstatm_t *stats;
	iter_error_t err;
};

static int parse_pidstatm_line(char *token, int idx, void *state)
{
	struct statm_state *st = (struct statm_state *)state;

        if (idx > (int)ARRAY_SIZE(pidstatm_functable)) {
		abort();
        }

        if (!pidstatm_functable[idx].fn(token, st->stats)) {
		st->err.saved_errno = errno;
		snprintf(st->err.errstr, sizeof(st->err.errstr),
			 "%s: failed to parse value at idx %d in line",
			 token, idx);
                return ITER_STATE_ERROR;
        }

        return ITER_STATE_CONTINUE;
}

static int read_pidstatm_line(char *line, int idx, ssize_t line_len, void *state)
{
	iter_line_cb_t cb = {
		.fn = parse_pidstatm_line,
		.state = state,
	};

	return iter_line(line, " ", &cb);
}

int read_pid_statm(FILE *statsfile, pidstatm_t *stats)
{
	int rv;
	struct statm_state state = {
		.stats = stats
	};
	iter_file_cb_t cb = {
		.fn = read_pidstatm_line,
		.state = &state,
	};

	Py_BEGIN_ALLOW_THREADS
	rewind(statsfile);
	rv = iter_file(statsfile, &cb);
	Py_END_ALLOW_THREADS

	if (rv == ITER_STATE_ERROR) {
		PyErr_Format(
			PyExc_RuntimeError,
			"read_pid_statm(): %s: %s",
			cb.err.errstr,
			strerror(cb.err.saved_errno)
		);
	}
	return rv;
}

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

#ifndef _ITER_H_
#define _ITER_H_
#include <dirent.h>

#define ITER_STATE_ERROR -1
#define ITER_STATE_DONE -2
#define ITER_STATE_BREAK -3
#define ITER_STATE_CONTINUE 0
#define ERRSTR_MAX_LEN 256

typedef struct iter_error {
	int saved_errno;
	char errstr[ERRSTR_MAX_LEN];
} iter_error_t;

typedef struct iter_line_cb {
	PyThreadState *_save;
	int (*fn) (char *token, int idx, void *state);
	void *state;
	iter_error_t err;
} iter_line_cb_t;

typedef struct iter_file_cb {
	PyThreadState *_save;
	int (*fn)(char *line, int idx, ssize_t linelen, void *state);
	void *state;
	iter_error_t err;
} iter_file_cb_t;

typedef struct iter_dir_cb {
	PyThreadState *_save;
	int (*fn)(struct dirent *entry, void *state);
	void *state;
	iter_error_t err;
} iter_dir_cb_t;

extern int iter_line(char *line, const char *delim, iter_line_cb_t *cb);
extern int iter_file(FILE *in_file, iter_file_cb_t *cb);
extern int iter_dir(DIR *in_file, iter_dir_cb_t *cb);
extern char *get_iter_error(void);

/*
 * Macros to take / release GIL in iterator
 * This allows us to re-take GIL inside the callback function if needed.
 */
#define ITER_ALLOW_THREADS(cb) { cb->_save = PyEval_SaveThread(); }
#define ITER_END_ALLOW_THREADS(cb) { PyEval_RestoreThread(cb->_save); }

#endif /* _ITER_H_ */

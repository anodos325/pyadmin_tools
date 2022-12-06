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
#include "../common/includes.h"
#include "iter.h"

int iter_line(char *line, const char *delim, iter_line_cb_t *cb)
{
	char *saveptr = NULL, *token = NULL;
	int i = 0;
	int rv;

	token = strtok_r(line, delim, &saveptr);
	rv = cb->fn(token, i, cb->state);
	if (rv != ITER_STATE_CONTINUE) {
		return rv;
	}

	for (i++; rv == ITER_STATE_CONTINUE ; i++) {
		token = strtok_r(NULL, delim, &saveptr);
		if (token == NULL) {
			break;
		}
		rv = cb->fn(token, i, cb->state);
	}

	return rv;
}

int iter_file(FILE *in_file, iter_file_cb_t *cb)
{
	char *line = NULL;
	size_t linecap = 0;
	int rv = ITER_STATE_CONTINUE;
	int line_no = 0;
	ssize_t linelen;

	rewind(in_file);

	for (line_no = 0; rv == ITER_STATE_CONTINUE; line_no++) {
		linelen = getline(&line, &linecap, in_file);
		if (linelen == -1) {
			if (errno) {
				cb->err.saved_errno = errno;
				strlcpy(cb->err.errstr, "getline() failed",
					sizeof(cb->err.errstr));
				rv = ITER_STATE_ERROR;
			}
			break;
		}

		rv = cb->fn(line, line_no, linelen, cb->state);
	}
	free(line);
	return rv;
}

int iter_dir(DIR *dirp, iter_dir_cb_t *cb)
{
	struct dirent *entry = NULL;
	int rv = ITER_STATE_CONTINUE;

	errno = 0;
	for (rv = ITER_STATE_CONTINUE; rv == ITER_STATE_CONTINUE;) {
		entry = readdir(dirp);
		if (entry == NULL) {
			if (errno) {
				cb->err.saved_errno = errno;
				strlcpy(cb->err.errstr, "readdir() failed",
					sizeof(cb->err.errstr));
				rv = ITER_STATE_ERROR;
			}
			break;
		}

		if ((strcmp(entry->d_name, ".") == 0) ||
		    (strcmp(entry->d_name, "..") == 0)) {
			continue;
		}

		rv = cb->fn(entry, cb->state);
	}
	return rv;
}

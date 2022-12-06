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

#ifndef _DISKSTATS_H_
#define _DISKSTATS_H_

#include <Python.h>
#include "../common/includes.h"

#define DISKSTATS_PATH "/proc/diskstats"
#define DISKSTATS_NAME_BUF 32 /* DBEV_NAME_SIZE */
typedef struct procfs_diskstats {
	uint major;
	uint minor;
	char name[DISKSTATS_NAME_BUF];
	unsigned long reads_completed;
	unsigned long reads_merged;
	unsigned long sectors_read;
	uint time_reading_ms;
	unsigned long writes_completed;
	unsigned long writes_merged;
	unsigned long sectors_written;
	uint time_writing_ms;
	uint num_ios_in_progress;
	uint time_doing_ios_ms;
	uint weighted_time_doing_ios_ms;
	unsigned long discards_completed;
	unsigned long discards_merged;
	unsigned long sectors_discarded;
	uint time_spent_discarding_ms;
	unsigned long flush_requests_completed;
	uint time_spent_flushing_ms;
} diskstats_t;

typedef struct {
	PyObject_HEAD
	FILE *stats_file;
	diskstats_t *stats;
	int stats_alloc;
	int stats_cnt;
} py_diskstats_t;

typedef struct {
	PyObject_HEAD
	diskstats_t stat;
} py_diskstats_entry_t;

extern PyTypeObject PyDiskStats;
extern PyTypeObject PyDiskStatsEntry;
PyObject *init_diskstats(diskstats_t *stats_in);
#endif /* _DISKSTATS_H_ */

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

#ifndef _PARSERS_H_
#define _PARSERS_H_
#include "../common/includes.h"

extern bool parse_major_minor(char *token, uint *out);
extern bool parse_ulonglong(char *token, unsigned long long *out);
extern bool parse_ulong(char *token, unsigned long *out);
extern bool parse_long(char *token, long *out);
extern bool parse_uint(char *token, uint *out);
extern bool parse_int(char *token, int *out);

#endif /* _PARSERS_H_ */

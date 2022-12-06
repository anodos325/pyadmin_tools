
#include <Python.h>
#include "parser.h"
#include "../common/includes.h"

bool parse_ulonglong(char *token, unsigned long long *outp)
{
	unsigned long long val = 0;
	char *endp = NULL;

	errno = 0;
	val = strtoull(token, &endp, 10);
	if ((*endp != '\0') ||
	    ((val == ULLONG_MAX) && (errno == ERANGE))) {
		return false;
	}

	*outp = val;
	return true;
}

bool parse_ulong(char *token, unsigned long *outp)
{
	unsigned long val = 0;
	char *endp = NULL;

	errno = 0;
	val = strtoul(token, &endp, 10);
	if ((*endp != '\0') ||
	    ((val == ULONG_MAX) && (errno == ERANGE))) {
		return false;
	}

	*outp = val;
	return true;
}

bool parse_long(char *token, long *outp)
{
	unsigned long val;

	if (!parse_ulong(token, &val)) {
		return false;
	}

	*outp = (long)val;
	return true;
}

bool parse_uint(char *token, uint *outp)
{
	unsigned long val;

	if (!parse_ulong(token, &val)) {
		return false;
	}

	if (val >= UINT_MAX) {
		errno = ERANGE;
		return false;
	}

	*outp = (uint)val;
	return true;
}

bool parse_int(char *token, int *outp)
{
	long val;

	if (!parse_long(token, &val)) {
		return false;
	}

	if (val >= INT_MAX) {
		errno = ERANGE;
		return false;
	}

	*outp = (int)val;
	return true;
}

bool parse_major_minor(char *token, uint *out)
{
	return parse_uint(token, out);
}

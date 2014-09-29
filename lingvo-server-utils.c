#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "lingvo-server-utils.h"




static int make_message(char **dst_str, int *dst_str_len, int *size_alloc, const char *fmt, va_list ap)
{
	char *np;
	va_list aq;


	*size_alloc = 128;
	*dst_str = malloc(*size_alloc);
	if (*dst_str == NULL)
		return -1;

	for (;;) {
		va_copy(aq, ap);
		*dst_str_len = vsnprintf(*dst_str, *size_alloc, fmt, aq);
		va_end(aq);

		if (*dst_str_len > -1 && *dst_str_len < *size_alloc)
			return 1;

		if (*dst_str_len > -1)
			*size_alloc = *dst_str_len + 1;
		else
			*size_alloc *= 2;

		np = realloc(*dst_str, *size_alloc);
		if (np == NULL) {
			free(*dst_str);
			return -1;
		}
		
		*dst_str = np;
	}
}

int parameter_parse(const char *s1, const char *s1_end, const char *s2)
{
	for ( ; ; ++s1, ++s2) {
		if (s1 == s1_end) {
			if (*s2 == '\0')
				return 0;
			else
				return *s2;
		}
		if (*s2 == '\0')
			return -*s1;
		if (*s1 != *s2)
			return *s2 - *s1;
	}
}

/* -1 -- system error
 *  0 -- not parameter
 *  1 -- not match with 'name'
 *  2 -- parameter matched with 'name'
 */

int parameter_save(const char *p1, const char *p1_end,
		const char *name, char **value, const char **next)
{
	const char *p2, *p3, *p4;


	for (p2 = p1; ; ++p2) {
		if (p2 == p1_end) {
			*next = p1;
			return PAR_SAVE_NOT_PAR;
		}
		if (*p2 == ' ' || *p2 == '=')
			break;
	}

	for (p3 = p2; ; ++p3) {
		if (p3 == p1_end) {
			*next = p1;
			return PAR_SAVE_NOT_PAR;
		}
		if (*p3 != ' ')
			break;
	}

	if (*p3 != '=') {
		*next = p1;
		return PAR_SAVE_NOT_PAR;
	}

	for (++p3; ; ++p3) {
		if (p3 == p1_end) {
			*next = p1;
			return PAR_SAVE_NOT_PAR;
		}
		if (*p3 != ' ')
			break;
	}

	if (*p3 == '"') ++p3;

	for (p4 = p3; ; ++p4) {
		if (p4 == p1_end) {
			*next = p1;
			return PAR_SAVE_NOT_PAR;
		}
		if (*p4 == '"')
			break;
	}
	*next = p4 + 1;

	if (parameter_parse(p1, p2, name) != 0) {
		*next = p4;
		return PAR_SAVE_NOT_MATCH;
	}

	if (*value != NULL)
		free(*value);

	*value = strndup(p3, p4 - p3);
	if (*value == NULL)
		return PAR_SAVE_SYSERR;

	return PAR_SAVE_MATCH;
}

int send_response(int sock, const char *str, ...)
{
	va_list ap;
	char *data = NULL;
	int data_len;
	int data_alloc;
	int ret = 1, res;

	va_start(ap, str);
	res = make_message(&data, &data_len, &data_alloc, str, ap);
	va_end(ap);

	if (res == -1) {
		ret = -1; goto END;
	}

	if (data_len == 0)
		return 1;

	if (write(sock, data, data_len) != data_len) {
		printf("write(): %s (%u)\n", strerror(errno), errno);
		ret = -1; goto END;
	}

END:	if (data != NULL)
		free(data);
	return ret;
}

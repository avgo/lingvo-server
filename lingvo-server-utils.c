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

int send_all(int sock, const char *buf, int buf_size)
{
	int bytes;


	while (buf_size > 0) {
		bytes = write(sock, buf, buf_size);

		if (bytes == -1) {
			if (errno == EAGAIN)
				continue;
			return -1;
		}

		buf += bytes;
		buf_size -= bytes;
	}

	return 1;
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

int unescape_string(const char *s, const char *s_end, char **unescaped_string)
{
	int ret = 1;
	const char *p1 = s;
	char *result = NULL, *r1;
	int count = 0;


	if (s_end == NULL && s != NULL) {
		for (s_end = s; *s_end != '\0'; ++s_end)
			;
	}

#define is_x_digit(c) \
	(('0' <= c && c <= '9') || \
	('a' <= c && c <= 'f') || \
	('A' <= c && c <= 'F'))

	for (p1 = s; p1 != s_end; ++count) {
		if (*p1 == '%') {
			if (s_end - p1 < 3) {
				ret = 0; goto END;
			}
			char c1 = p1[1], c2 = p1[2];
			if (!is_x_digit(c1) || !is_x_digit(c2)) {
				ret = 0; goto END;
			}
			p1 += 3;
		}
		else
			++p1;
	}

	result = malloc(count+1);
	if (result == NULL) {
		ret = -1; goto END;
	}
	for (p1 = s, r1 = result; p1 != s_end; ++r1) {
		if (*p1 == '%') {
			char c1 = p1[1], c2 = p1[2];
			if ('0' <= c1 && c1 <= '9') {
				*r1 = (c1 - '0') << 4;
			}
			else
			if ('A' <= c1 && c1 <= 'F') {
				*r1 = (c1 - 'A' + 0xA) << 4;
			}
			else
			if ('a' <= c1 && c1 <= 'f') {
				*r1 = (c1 - 'a' + 0xA) << 4;
			}
			if ('0' <= c2 && c2 <= '9') {
				*r1 |= c2 - '0';
			}
			else
			if ('A' <= c2 && c2 <= 'F') {
				*r1 |= c2 - 'A' + 0xA;
			}
			else
			if ('a' <= c2 && c2 <= 'f') {
				*r1 |= c2 - 'a' + 0xA;
			}
			p1 += 3;
		}
		else {
			*r1 = *p1;
			++p1;
		}
	}

	*r1 = '\0';

END:	if (ret > 0)
		*unescaped_string = result;
	else {
		if (result != NULL)
			free(result);
		*unescaped_string = NULL;
	}

	return ret;
}

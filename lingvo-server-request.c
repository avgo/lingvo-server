#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "lingvo-server-request.h"
#include "lingvo-server-utils.h"




static const char* get_boundary(const char *str, int str_len, const char *boundary);
static const char* get_terminator(const char *str, int str_len);
static void lingvo_server_request_parse(lingvo_server_request *request);
static void lingvo_server_request_parse_method(lingvo_server_request *request);
static void lingvo_server_request_parse_multipart_form_data(
		lingvo_server_request *request,
		const char *s, const char *s_end);
static int lingvo_server_request_post_data(lingvo_server_request *request);




static lingvo_server_method methods[] = {
	{ "GET",  LINGVO_SERVER_GET  },
	{ "POST", LINGVO_SERVER_POST },
};
int methods_count = sizeof(methods) / sizeof(*methods);




static const char* get_boundary(const char *str, int str_len, const char *boundary)
{
	const char *b, *b_end, *s, *str_end;
	int b_len;


	b_len = strlen(boundary);
	if (str_len < b_len)
		return NULL;

	str_end = str + str_len - b_len + 1;
	b_end = boundary + b_len;

	for ( ; str != str_end; ++str)

	for (s = str, b = boundary; ; ++s, ++b) {
		if (b == b_end)
			return str;
		if (*s != *b)
			break;
	}

	return NULL;
}

static const char* get_terminator(const char *str, int str_len)
{
	const char *str_end = str + str_len - 3;

	for ( ; str < str_end; ++str) {
		if (str[0] == '\r' && str[1] == '\n' &&
			str[2] == '\r' && str[3] == '\n')
			return str + 4;
		if (str[0] == '\n' && str[1] == '\n')
			return str + 2;
	}
	for (str_end += 2; str < str_end; ++str) {
		if (str[0] == '\n' && str[1] == '\n')
			return str + 2;
	}

	return NULL;
}

void lingvo_server_request_free(lingvo_server_request *request)
{
	if (request->request_string != NULL)
		free(request->request_string);
	if (request->query != NULL)
		free(request->query);
	if (request->mp_data_boundary != NULL)
		free(request->mp_data_boundary);
	multipart_data_free(&request->mp_data);
}

void lingvo_server_request_init(lingvo_server_request *request)
{
	request->request_string = NULL;
	request->query = NULL;
	request->content_length = 0;
	request->shutdown = 0;
	request->mp_data_boundary = NULL;

	multipart_data_init(&request->mp_data);
}

static void lingvo_server_request_parse(lingvo_server_request *request)
{
	const char *c1, *c2, *c3, *c4, *rs_end;


	rs_end = request->request_string + request->request_string_len;
	lingvo_server_request_parse_method(request);

	c1 = request->request_string;
	for ( ; ; ++c1) {
		if (c1 == rs_end) {
			printf("warning: incomplete request.\n");
			return ;
		}
		if (*c1 == ' ')
			break;
	}
	while (*c1 == ' ')
		++c1;
	for (c2 = c1; ; ++c2) {
		if (c2 == rs_end) {
			printf("warning: incomplete request.\n");
			return ;
		}
		if (*c2 == '\r' || *c2 == '\n')
			break;
	}
	for (c3 = c2; ; --c2) {
		if (c2 == c1) {
			printf("warning: incomplete request.\n");
			return ;
		}
		if (*c2 == ' ')
			break;
	}

	request->query = malloc(c2 - c1 + 1);
	if (request->query == NULL)
		return ;
	memcpy(request->query, c1, c2 - c1);
	request->query[c2 - c1] = '\0';

	c1 = c3;
	if (*c1 == '\r')
		++c1;
	if (*c1 == '\n')
		++c1;
	else {
		printf("warning: bad request.\n");
		return ;
	}

	printf("%s\n", request->query);

	for ( ; *c1 != '\r' && *c1 != '\n'; ) {
		for (c2 = c1; *c2 != ':'; ++c2) {
			if (c2 == rs_end) {
				printf("warning: bad request.\n");
				return ;
			}
		}
		for (c3 = c2 + 1; *c3 == ' '; ++c3)
			;
		for (c4 = c3; *c4 != '\r' && *c4 != '\n'; ++c4) {
			if (c4 == rs_end) {
				printf("warning: bad request.\n");
				return ;
			}
		}

		if (parameter_parse(c1, c2, "Content-Length") == 0) {
			int number = 0;

			for ( ; c3 != c4 && isdigit(*c3); ++c3)
				number = number * 10 + *c3 - '0';

			request->content_length = number;
		}
		else
		if (parameter_parse(c1, c2, "Content-Type") == 0)
			lingvo_server_request_parse_multipart_form_data(request, c3, c4);

		c1 = c4;
		if (*c1 == '\r')
			++c1;
		if (*c1 == '\n')
			++c1;
		else {
			printf("warning: bad request.\n");
			return ;
		}
	}
}

static void lingvo_server_request_parse_multipart_form_data(
		lingvo_server_request *request,
		const char *str, const char *str_end)
{
	const char *p1, *p2;


	for (p1 = str; ; ++p1) {
		if (p1 == str_end)
			return ;
		if (*p1 == ';')
			break;
	}
	if (parameter_parse(str, p1, "multipart/form-data") != 0)
		return ;
	for (++p1; ; ++p1) {
		if (p1 == str_end)
			return ;
		if (*p1 != ' ')
			break;
	}

	for (p2 = p1; ; ++p2) {
		if (p2 == str_end)
			return ;
		if (*p2 == ' ' || *p2 == '=')
			break;
	}

#if 0
	printf("'%.*s'\n", (int) (str_end - p1), p1);
#endif

	if (parameter_parse(p1, p2, "boundary") != 0)
		return ;

	for ( ; ; ++p2) {
		if (p2 == str_end)
			return ;
		if (*p2 != ' ')
			break;
	}

	if (*p2 != '=')
		return ;

	for (p1 = p2 + 1; ; ++p1) {
		if (p1 == str_end)
			return ;
		if (*p1 != ' ')
			break;
	}
#if 0
	printf("'%.*s'\n", (int) (str_end - p1), p1);
#endif
	request->mp_data_boundary = malloc(str_end - p1 + 4 + 1);
	if (request->mp_data_boundary == NULL) {
		return ;
	}

	sprintf(request->mp_data_boundary, "\r\n--%.*s", (int) (str_end - p1), p1);
#if 0
	printf("'%s'\n", request->mp_data_boundary);
#endif
}

static void lingvo_server_request_parse_method(lingvo_server_request *request)
{
	const char *b, *e, *rs_end;
	lingvo_server_method *method;
	lingvo_server_method *methods_end = methods + methods_count;


	rs_end = request->request_string + request->request_string_len;
	b = e = request->request_string;
	while (e != rs_end && *e != ' ')
		++e;

	for (method = methods; ; ++method) {
		if (method == methods_end) {
			printf("warning: unknown method '%.*s'.\n",
					(int) (e - b), b);
			return ;
		}

		if (parameter_parse(b, e, method->method) == 0)
			break;

	}

	memcpy(&request->method, method, sizeof(lingvo_server_method));
}

static int lingvo_server_request_post_data(lingvo_server_request *request)
{
	const char *str, *str_end;
	int str_len;
	int b_len;


	if (request->mp_data_boundary == NULL)
		return 1;

	str = request->request_string;
	str_len = request->request_string_len;

	b_len = strlen(request->mp_data_boundary);

	str = get_boundary(str, str_len, request->mp_data_boundary);

	if (str == NULL)
		return 1;

	str += b_len;
	if (*str == '\r') ++str;
	if (*str == '\n') ++str;
	str_len = request->request_string_len -
			(str - request->request_string);

	while (str_end = get_boundary(str, str_len, request->mp_data_boundary)) {
		if (multipart_data_add_frame(&request->mp_data, str, str_end) == -1) {
			return -1;
		}

		str = str_end + b_len;
		if (*str == '\r') ++str;
		if (*str == '\n') ++str;
		str_len = request->request_string_len -
				(str - request->request_string);
	}

	return 1;
}

int lingvo_server_request_read(lingvo_server_request *request, int s)
{
	const char *terminator;
	char buf[50];
	int bytes_read, ret = 1;
	char *buffer = NULL;
	int buffer_size = 0;
	int buffer_size_alloc = 0;


	for (terminator = NULL; terminator == NULL; ) {
		bytes_read = read(s, buf, sizeof(buf));

		if (bytes_read == -1) {
			if (errno == EAGAIN)
				continue;
			else {
				printf("read(): %s (%d).\n", strerror(errno), errno);
				ret = -1; goto END;
			}
		}

		if (bytes_read == 0)
			continue;

		int buffer_size_new = buffer_size + bytes_read;

		if (buffer_size_new > buffer_size_alloc) {
			buffer_size_alloc = 1;
			while (buffer_size_alloc < buffer_size_new)
				buffer_size_alloc <<= 1;
			buffer = realloc(buffer, buffer_size_alloc);
			if (buffer == NULL)
				return -1;
		}

		char *new_data = buffer + buffer_size;

		memcpy(new_data, buf, bytes_read);

		if (buffer_size > 3)
			terminator = get_terminator(
					new_data - 3,
					bytes_read + 3);
		else
			terminator = get_terminator(buffer, buffer_size_new);

		buffer_size = buffer_size_new;
	}

	request->request_string = buffer;
	request->request_string_len = buffer_size;

	lingvo_server_request_parse(request);

	if (request->method.id == LINGVO_SERVER_POST) {
		int bytes_post_loaded = buffer + buffer_size - terminator;
		if (bytes_post_loaded >= request->content_length)
			return ret;
		int bytes_post_remain = request->content_length - bytes_post_loaded;

		while (bytes_post_remain > 0) {
			bytes_read = read(s, buf,
				bytes_post_remain < sizeof(buf) ?
				bytes_post_remain : sizeof(buf));

			if (bytes_read == -1) {
				if (errno == EAGAIN)
					continue;
				else {
					printf("read(): %s (%d).\n", strerror(errno), errno);
					ret = -1; goto END;
				}
			}

			if (bytes_read == 0)
				continue;

			int buffer_size_new = buffer_size + bytes_read;

			if (buffer_size_new > buffer_size_alloc) {
				buffer_size_alloc = 1;
				while (buffer_size_alloc < buffer_size_new)
					buffer_size_alloc <<= 1;
				buffer = realloc(buffer, buffer_size_alloc);
				if (buffer == NULL)
					return -1;
			}

			char *new_data = buffer + buffer_size;

			memcpy(new_data, buf, bytes_read);
			buffer_size = buffer_size_new;
			bytes_post_remain -= bytes_read;
		}
	}

	request->request_string = buffer;
	request->request_string_len = buffer_size;

	if (lingvo_server_request_post_data(request) == -1)
		return -1;

END:	return ret;
}

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "lingvo-server-request.h"




static const char* get_terminator(const char *str, int str_len);
static int parameter_parse(const char *s1, const char *s1_end, const char *s2);
static void lingvo_server_request_handler_get(lingvo_server_request *request);
static void lingvo_server_request_handler_post(lingvo_server_request *request);
static void lingvo_server_request_parse(lingvo_server_request *request);
static void lingvo_server_request_parse_method(lingvo_server_request *request);




static lingvo_server_method handlers[] = {
	{ "GET",  LINGVO_SERVER_GET,  lingvo_server_request_handler_get  },
	{ "POST", LINGVO_SERVER_POST, lingvo_server_request_handler_post },
};
int handlers_count = sizeof(handlers) / sizeof(*handlers);




static const char* get_terminator(const char *str, int str_len)
{
	const char *str_end;


	if (str_len < 4)
		return NULL;

	str_end = str + str_len - 3;

	for ( ; str != str_end; ++str) {
		if (str[0] == '\r' && str[1] == '\n' &&
			str[2] == '\r' && str[3] == '\n')
			return str + 4;
		if (str[0] == '\n' && str[1] == '\n')
			return str + 2;
	}
	for (str_end += 2; str != str_end; ++str) {
		if (str[0] == '\n' && str[1] == '\n')
			return str + 2;
	}

	return NULL;
}

static int parameter_parse(const char *s1, const char *s1_end, const char *s2)
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
	}
}

void lingvo_server_request_free(lingvo_server_request *request)
{
	domutils_string_free(&request->request_string);
	if (request->query != NULL)
		free(request->query);
}

static void lingvo_server_request_handler_get(lingvo_server_request *request)
{
	printf("GET!\n");
}

static void lingvo_server_request_handler_post(lingvo_server_request *request)
{
	printf("POST!\n");
}

void lingvo_server_request_init(lingvo_server_request *request)
{
	domutils_string_init(&request->request_string);
	request->query = NULL;
	request->content_length = 0;
}

static void lingvo_server_request_parse(lingvo_server_request *request)
{
	const char *c1, *c2, *c3, *c4;


	lingvo_server_request_parse_method(request);

	c1 = request->request_string.data;
	for ( ; ; ++c1) {
		if (*c1 == '\0') {
			printf("warning: incomplete request.\n");
			return ;
		}
		if (*c1 == ' ')
			break;
	}
	while (*c1 == ' ')
		++c1;
	for (c2 = c1; ; ++c2) {
		if (*c2 == '\0') {
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
			if (*c2 == '\0') {
				printf("warning: bad request.\n");
				return ;
			}
		}
		for (c3 = c2 + 1; *c3 == ' '; ++c3)
			;
		for (c4 = c3; *c4 != '\r' && *c4 != '\n'; ++c4) {
			if (*c4 == '\0') {
				printf("warning: bad request.\n");
				return ;
			}
		}

		if (parameter_parse(c1, c2, "Content-Length") == 0) {
			int number = 0;

			for ( ; c3 != c4 && isdigit(*c3); ++c3)
				number = number * 10 + *c3 - '0';

			request->content_length = number;
			printf("c-l: %d\n", request->content_length);
		}

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

static void lingvo_server_request_parse_method(lingvo_server_request *request)
{
	const char *b, *e;
	lingvo_server_method *handler;
	lingvo_server_method *handlers_end = handlers + handlers_count;


	b = e = request->request_string.data;
	while (*e != '\0' && *e != ' ')
		++e;

	for (handler = handlers; ; ++handler) {
		if (handler == handlers_end) {
			printf("warning: unknown method '%.*s'.\n",
					(int) (e - b), b);
			return ;
		}

		if (parameter_parse(b, e, handler->method) == 0)
			break;

	}

	memcpy(&request->method, handler, sizeof(lingvo_server_method));
}

int lingvo_server_request_read(lingvo_server_request *request, int s)
{
	const char *terminator;
	char buf[1000];
	int bytes_read, ret = 1;


	for (terminator = NULL; terminator == NULL; ) {
		bytes_read = read(s, buf, sizeof(buf) - 1);

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
		buf[bytes_read] = '\0';
		domutils_string_append(&request->request_string, buf);
		terminator = get_terminator(request->request_string.data,
				request->request_string.size - 1);
	}

	lingvo_server_request_parse(request);

END:	return ret;
}

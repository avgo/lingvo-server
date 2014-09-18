#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "lingvo-server-request.h"




static void lingvo_server_request_handler_get(lingvo_server_request *request);
static void lingvo_server_request_handler_post(lingvo_server_request *request);
static void lingvo_server_request_parse(lingvo_server_request *request);
static void lingvo_server_request_parse_method(lingvo_server_request *request);




static lingvo_server_method handlers[] = {
	{ "GET",  LINGVO_SERVER_GET,  lingvo_server_request_handler_get  },
	{ "POST", LINGVO_SERVER_POST, lingvo_server_request_handler_post },
};
int handlers_count = sizeof(handlers) / sizeof(*handlers);




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
}

static void lingvo_server_request_parse(lingvo_server_request *request)
{
	const char *c1, *c2, *c3;


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
}

static void lingvo_server_request_parse_method(lingvo_server_request *request)
{
	const char *b, *e, *s1, *s2;
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

		s1 = b;
		s2 = handler->method;

		for ( ; ; ++s1, ++s2) {
			if (s1 == e) {
				if (*s2 == '\0')
					goto SUCCESS;
				else
					break;
			}
			if (*s2 == '\0')
				break;
		}
	}

SUCCESS:

	memcpy(&request->method, handler, sizeof(lingvo_server_method));
}

int lingvo_server_request_read(lingvo_server_request *request, int s)
{
	char buf[1000];
	int bytes_read, ret = 1;


	for (;;) {
		bytes_read = read(s, buf, sizeof(buf) - 1);

		if (bytes_read == -1) {
			if (errno == EAGAIN)
				continue;
			else {
				printf("read(): %s (%d).\n", strerror(errno), errno);
				ret = -1; goto END;
			}
		}

		buf[bytes_read] = '\0';
		domutils_string_append(&request->request_string, buf);

		if (strstr(request->request_string.data, "\r\n\r"))
			break;
		if (strstr(request->request_string.data, "\n\n"))
			break;
	}

	lingvo_server_request_parse(request);

END:	return ret;
}

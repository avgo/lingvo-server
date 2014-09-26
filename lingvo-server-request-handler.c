#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "lingvo-server-request-handler.h"
#include "lingvo-server-utils.h"




struct req_handler_ {
	char *command;
	int (*proc)(lingvo_server_request*, int);
};

typedef struct req_handler_ req_handler;




int handler_default(lingvo_server_request *request, int s);
int handler_err(lingvo_server_request *request, int s);
int handler_shutdown(lingvo_server_request *request, int s);
int handler_test(lingvo_server_request *request, int s);




static req_handler handlers[] = {
	{ "",         handler_default  },
	{ "shutdown", handler_shutdown },
	{ "test",     handler_test     },
};
static int handlers_count = sizeof(handlers) / sizeof(*handlers);




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

static int send_response(int sock, const char *str, ...)
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

int lingvo_server_request_handler(lingvo_server_request *request, int s)
{
	req_handler *handlers_end = handlers + handlers_count;
	char *q = request->query, *q_end;

	for ( ; *q == '/'; ++q)
		;
	for (q_end = q; *q_end != '\0' && *q_end != '/'; ++q_end)
		;

	for (req_handler *h = handlers; h != handlers_end; ++h) {
		if (parameter_parse(q, q_end, h->command) == 0) {
			if (send_response(s,
					"HTTP/1.1 200 OK\n"
					"Content-Type: text/html\n"
					"\n") == -1)
				return -1;
			if (h->proc(request, s) == -1)
				return -1;
			return 1;
		}
	}

	if (send_response(s,
			"HTTP/1.1 200 OK\n"
			"Content-Type: text/html\n"
			"\n") == -1)
		return -1;

	if (handler_err(request, s) == -1)
		return -1;

	return 1;
}

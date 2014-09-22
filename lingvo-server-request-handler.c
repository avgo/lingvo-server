#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "lingvo-server-request-handler.h"




struct req_handler_ {
	char *command;
	int (*proc)(lingvo_server_request*, int);
};

typedef struct req_handler_ req_handler;




static int handler_default(lingvo_server_request *request, int s);
static int handler_err(lingvo_server_request *request, int s);
static int handler_shutdown(lingvo_server_request *request, int s);




static req_handler handlers[] = {
	{ "",         handler_default  },
	{ "shutdown", handler_shutdown },
};
static int handlers_count = sizeof(handlers) / sizeof(*handlers);




static char* get_time_str()
{
	static char timebuf[20];
	time_t t;


	t = time(NULL);
	strftime(timebuf, sizeof(timebuf), "%F %T", localtime(&t));

	return timebuf;
}

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

static int handler_default(lingvo_server_request *request, int s)
{
	char *str =
		"HTTP/1.1 200 OK\n"
		"Content-Type: text/html\n"
		"\n"
		"<html>\n"
		"<head>\n"
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf8\">\n"
		"</head>\n"
		"<body>\n"
		"%s<br>\n"
		"<a href=\"/\">home</a><br>\n"
		"<a href=\"/shutdown\">Завершить работу сервера</a><br>\n"
		"<form action=\"/\" method=\"post\">\n"
		"<input type=\"text\" name=\"parameter1\" value=\"value1\">\n"
		"<input type=\"text\" name=\"parameter2\" value=\"value2\">\n"
		"<input type=\"submit\" value=\"Go\">\n"
		"</form>\n"
		"<form enctype=\"multipart/form-data\" method=\"post\">\n"
		"файл:\n"
		"<input name=\"textfile\" type=\"file\" size=\"50\">\n"
		"<input type=\"submit\" value=\"Отправить\">\n"
		"</form>\n"
		"<pre>\n"
		"%.*s"
		"</pre>\n"
		"</body>\n"
		"</html>\n";

	int ret = 1;


	if (send_response(s, str,
			get_time_str(),
			request->request_string_len,
			request->request_string) == -1)
	{
		ret = -1; goto END;
	}

END:	return ret;
}

static int handler_err(lingvo_server_request *request, int s)
{
	char *str =
		"HTTP/1.1 200 OK\n"
		"Content-Type: text/html\n"
		"\n"
		"<html>\n"
		"<head>\n"
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf8\">\n"
		"</head>\n"
		"<body>\n"
		"Неизвестное действие '%s'<br>\n"
		"<a href=\"/\">home</a><br>\n"
		"<a href=\"/shutdown\">Завершить работу сервера</a><br>\n"
		"</body>\n"
		"</html>\n";

	int ret = 1;


	send_response(s, str, request->query);

END:	return ret;
}

static int handler_shutdown(lingvo_server_request *request, int s)
{
	char *str =
		"HTTP/1.1 200 OK\n"
		"Content-Type: text/html\n"
		"\n"
		"<html>\n"
		"<head>\n"
		"<title>Завершение работы</title>\n"
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf8\">\n"
		"</head>\n"
		"<body>\n"
		"Работа сервера будет остановлена.<br>\n"
		"<a href=\"/\">home</a><br>\n"
		"</body>\n"
		"</html>\n";

	int ret = 1;


	if (send_response(s, str) == -1)
	{
		ret = -1; goto END;
	}
	request->shutdown = 1;

END:	return ret;
}

int lingvo_server_request_handler(lingvo_server_request *request, int s)
{
	req_handler *handlers_end = handlers + handlers_count;
	char *q = request->query;

	for ( ; *q == '/'; ++q)
		;

	for (req_handler *h = handlers; h != handlers_end; ++h) {
		if (strcmp(h->command, q) == 0) {
			h->proc(request, s);
			return 1;
		}
	}

	if (handler_err(request, s) == -1)
		return -1;

	return 1;
}

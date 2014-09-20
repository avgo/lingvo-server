#include <time.h>
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




static req_handler handlers[] = {
	{ "", handler_default },
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
	char buf[10000];
	int buf_len;


	sprintf(buf, str, get_time_str(), request->request_string_len, request->request_string);

	buf_len = strlen(buf);
	if (write(s, buf, buf_len) != buf_len) {
		printf("write(): %s (%u)\n", strerror(errno), errno);
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
	char buf[10000];
	int buf_len;


	sprintf(buf, str, request->query);

	buf_len = strlen(buf);
	if (write(s, buf, buf_len) != buf_len) {
		printf("write(): %s (%u)\n", strerror(errno), errno);
		ret = -1; goto END;
	}

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

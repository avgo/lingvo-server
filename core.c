#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <lingvo/lingvo.h>

#include "core.h"
#include "lingvo-server-request.h"




static int do_accept(int s);
static int write_response(lingvo_server_request *request, int s);




int create_server()
{
	int s = -1, ret = 1;
	struct sockaddr_in addr;

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == -1) {
		printf("error\n");
		ret = -1; goto END;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(10000);

	printf("binding .. "); fflush(stdout);
	while (bind(s, (struct sockaddr*) &addr,
			sizeof(addr)) == -1)
	{
		if (errno == EADDRINUSE) {
			sleep(1);
		}
		else {
			printf("bind(): %d (%s).\n", errno, strerror(errno));
			ret = -1; goto END;
		}
	}
	printf("ok\n");

	if (listen(s, 10) == -1) {
		printf("listen(): %s (%d).\n", strerror(errno), errno);
		ret = -1; goto END;
	}

	while ((ret = do_accept(s)) > 0)
		;

END:	if (s != -1)
		close(s);

	return ret;
}

static int do_accept(int s)
{
	int acc = -1, ret = 1;
	lingvo_server_request request;


	lingvo_server_request_init(&request);

	acc = accept(s, NULL, NULL);
	if (acc == -1) {
		printf("error: %s (%d).\n", strerror(errno), errno);
		ret = -1; goto END;
	}

	fcntl(acc, F_SETFL, fcntl(acc, F_GETFL, 0) | O_NONBLOCK);

	if (lingvo_server_request_read(&request, acc) == -1) {
		ret = -1; goto END;
	}

	if (write_response(&request, acc) == -1) {
		ret = -1; goto END;
	}

	if (strcmp(request.query, "/shutdown") == 0) {
		printf("shutting down.\n");
		ret = 0;
	}

END:	if (acc != -1)
		close(acc);
	lingvo_server_request_free(&request);

	return ret;
}

static char* get_time_str()
{
	static char timebuf[20];
	time_t t;


	t = time(NULL);
	strftime(timebuf, sizeof(timebuf), "%F %T", localtime(&t));

	return timebuf;
}

static int write_response(lingvo_server_request *request, int s)
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
		"%s"
		"</pre>\n"
		"</body>\n"
		"</html>\n";

	int ret = 1;
	char buf[10000];
	int buf_len;


	sprintf(buf, str, get_time_str(), request->request_string.data);

	buf_len = strlen(buf);
	if (write(s, buf, buf_len) != buf_len) {
		printf("write(): %s (%u)\n", strerror(errno), errno);
		ret = -1; goto END;
	}

END:	return ret;
}

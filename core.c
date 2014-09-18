#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <lingvo/lingvo.h>
#include <domutils/string.h>

#include "core.h"




struct lingvo_server_request_ {
};

typedef struct lingvo_server_request_ lingvo_server_request;




static int do_accept(int s);
static int read_request(int s);
static int write_response(int s);




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

	while (do_accept(s) != -1) ; ret = -1;

END:	if (s != -1)
		close(s);

	return ret;
}

static int do_accept(int s)
{
	int acc = -1, ret = 1;
	lingvo_server_request request;


	acc = accept(s, NULL, NULL);
	if (acc == -1) {
		printf("error: %s (%d).\n", strerror(errno), errno);
		ret = -1; goto END;
	}

	fcntl(acc, F_SETFL, fcntl(acc, F_GETFL, 0) | O_NONBLOCK);

	if (read_request(acc) == -1) {
		ret = -1; goto END;
	}

	if (write_response(acc) == -1) {
		ret = -1; goto END;
	}

END:	if (acc != -1)
		close(acc);

	return ret;
}

static int read_request(int s)
{
	char buf[1000];
	domutils_string str;
	int bytes_read, ret = 1;


	domutils_string_init(&str);

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
		domutils_string_append(&str, buf);

		if (strstr(str.data, "\r\n\r"))
			break;
		if (strstr(str.data, "\n\n"))
			break;
	}

#if 1
	printf("Request: '%s'\n", str.data);
#endif

END:	domutils_string_free(&str);

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

static int write_response(int s)
{
	char *str =
		"HTTP/1.1 200 OK\n"
		"Content-Type: text/html\n"
		"\n"
		"<html>\n"
		"<head>\n"
		"</head>\n"
		"<body>\n"
		"%s\n"
		"</body>\n"
		"</html>\n";

	int ret = 1;
	char buf[1000];
	int buf_len;


	sprintf(buf, str, get_time_str());

	buf_len = strlen(buf);
	if (write(s, buf, buf_len) != buf_len) {
		printf("write(): %s (%u)\n", strerror(errno), errno);
		ret = -1; goto END;
	}

END:	return ret;
}

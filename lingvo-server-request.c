#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "lingvo-server-request.h"




void lingvo_server_request_free(lingvo_server_request *request)
{
	domutils_string_free(&request->request_string);
}

void lingvo_server_request_init(lingvo_server_request *request)
{
	domutils_string_init(&request->request_string);
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

END:	return ret;
}

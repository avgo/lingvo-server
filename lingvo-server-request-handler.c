#include "lingvo-server-request-handler.h"
#include "lingvo-server-utils.h"




struct req_handler_ {
	char *command;
	int (*proc)(lingvo_server_request*, int);
};

typedef struct req_handler_ req_handler;




int handler_default(lingvo_server_request *request, int s);
int handler_dictionary_add(lingvo_server_request *request, int s);
int handler_err(lingvo_server_request *request, int s);
int handler_file(lingvo_server_request *request, int s);
int handler_shutdown(lingvo_server_request *request, int s);
int handler_test(lingvo_server_request *request, int s);




static req_handler handlers[] = {
	{ "",                handler_default        },
	{ "dictionary_add",  handler_dictionary_add },
	{ "file",            handler_file           },
	{ "shutdown",        handler_shutdown       },
	{ "test",            handler_test           },
};
static int handlers_count = sizeof(handlers) / sizeof(*handlers);




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

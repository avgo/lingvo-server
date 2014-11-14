#include "lingvo-server-request-handler.h"
#include "lingvo-server-utils.h"




#define CONTENT_TYPE_TEXT_HTML                   "text/html"
#define CONTENT_TYPE_APPLICATION_X_JAVASCRIPT    "application/x-javascript"




struct req_handler_ {
	const char *command;
	int (*proc)(lingvo_server_request*, int);
	const char *content_type;
};

typedef struct req_handler_ req_handler;




int handler_default(lingvo_server_request *request, int s);
int handler_dictionary(lingvo_server_request *request, int s);
int handler_err(lingvo_server_request *request, int s);
int handler_file(lingvo_server_request *request, int s);
int handler_shutdown(lingvo_server_request *request, int s);
int handler_test(lingvo_server_request *request, int s);
int handler_wordtypes(lingvo_server_request *request, int s);




static req_handler handlers[] = {
	{ "",                handler_default,       CONTENT_TYPE_TEXT_HTML                },
	{ "dictionary",      handler_dictionary,    CONTENT_TYPE_TEXT_HTML                },
	{ "file",            handler_file,          CONTENT_TYPE_TEXT_HTML                },
	{ "shutdown",        handler_shutdown,      CONTENT_TYPE_TEXT_HTML                },
	{ "test",            handler_test,          CONTENT_TYPE_TEXT_HTML                },
	{ "wordtypes.js",    handler_wordtypes,     CONTENT_TYPE_APPLICATION_X_JAVASCRIPT },
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
			if (h->content_type != NULL) {
				if (send_response(s,
						"HTTP/1.1 200 OK\n"
						"Content-Type: %s\n"
						"\n",
						h->content_type) == -1)
				{
					return -1;
				}
			}

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

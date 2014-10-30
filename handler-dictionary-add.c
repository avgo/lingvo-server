#include <stdio.h>

#include "lingvo-server-request.h"
#include "lingvo-server-utils.h"




int handler_dictionary_add(lingvo_server_request *request, int s)
{
	send_response(s, "hello");

	printf("%.*s\n",
		request->request_string_len,
		request->request_string);

	return 1;
}

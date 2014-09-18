#ifndef LINGVO_SERVER_REQUEST_H
#define LINGVO_SERVER_REQUEST_H

#include <domutils/string.h>




struct lingvo_server_request_ {
	domutils_string request_string;
};

typedef struct lingvo_server_request_ lingvo_server_request;




void lingvo_server_request_free(lingvo_server_request *request);
void lingvo_server_request_init(lingvo_server_request *request);
int lingvo_server_request_read(lingvo_server_request *request, int s);




#endif /* LINGVO_SERVER_REQUEST_H */

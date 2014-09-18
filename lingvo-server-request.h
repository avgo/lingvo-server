#ifndef LINGVO_SERVER_REQUEST_H
#define LINGVO_SERVER_REQUEST_H

#include <domutils/string.h>




struct lingvo_server_request_;

typedef void (*lingvo_server_request_handler)(struct lingvo_server_request_*);

enum lingvo_server_method_id_ {
	LINGVO_SERVER_GET = 1,
	LINGVO_SERVER_POST,
};

typedef enum lingvo_server_method_id_ lingvo_server_method_id;

struct lingvo_server_method_ {
	char method[10];
	lingvo_server_method_id id;
	lingvo_server_request_handler proc;
};

typedef struct lingvo_server_method_ lingvo_server_method;

struct lingvo_server_request_ {
	domutils_string request_string;
	lingvo_server_method method;
	char *query;
};

typedef struct lingvo_server_request_ lingvo_server_request;




void lingvo_server_request_free(lingvo_server_request *request);
void lingvo_server_request_init(lingvo_server_request *request);
int lingvo_server_request_read(lingvo_server_request *request, int s);




#endif /* LINGVO_SERVER_REQUEST_H */

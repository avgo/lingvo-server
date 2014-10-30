#ifndef LINGVO_SERVER_REQUEST_H
#define LINGVO_SERVER_REQUEST_H

#include <netdb.h>
#include <netinet/in.h>
#include <domutils/string.h>

#include "multipart-data.h"




struct lingvo_server_request_;

enum lingvo_server_method_id_ {
	LINGVO_SERVER_GET = 1,
	LINGVO_SERVER_POST,
};

typedef enum lingvo_server_method_id_ lingvo_server_method_id;

struct lingvo_server_method_ {
	char method[10];
	lingvo_server_method_id id;
};

typedef struct lingvo_server_method_ lingvo_server_method;

struct lingvo_server_request_ {
	/* Full HTTP Header and it's length */
	char *request_string;
	int request_string_len;

	lingvo_server_method method;
	int content_length;

	/* Query string in first line of header
	 * For example: "GET /index.html HTTP"
	 * query: "/index.html"               */
	char *query;

	const char *terminator;

	char *mp_data_boundary;
	multipart_data mp_data;
	struct sockaddr_in client_addr;
	char host_buf[NI_MAXHOST];
	char port_buf[NI_MAXSERV];

	int shutdown;
};

typedef struct lingvo_server_request_ lingvo_server_request;




void lingvo_server_request_free(lingvo_server_request *request);
void lingvo_server_request_init(lingvo_server_request *request);
int lingvo_server_request_read(lingvo_server_request *request, int s);




#endif /* LINGVO_SERVER_REQUEST_H */

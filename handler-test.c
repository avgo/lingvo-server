#include "lingvo-server-request.h"
#include "doc-template.h"




int handler_test(lingvo_server_request *request, int s)
{
	int ret = 1;
	doc_template dt;


	doc_template_init(&dt);

	if (doc_template_open(&dt, "templates/test.html") == -1) {
		ret = -1; goto END;
	}
	if (doc_template_send(&dt, s,
			NULL) == -1)
	{
		ret = -1; goto END;
	}

END:	doc_template_free(&dt);

	return ret;
}

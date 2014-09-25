#include <time.h>

#include "lingvo-server-request.h"
#include "doc-template.h"




static char* get_time_str()
{
	static char timebuf[20];
	time_t t;


	t = time(NULL);
	strftime(timebuf, sizeof(timebuf), "%F %T", localtime(&t));

	return timebuf;
}

int handler_shutdown(lingvo_server_request *request, int s)
{
	int ret = 1;
	doc_template dt;


	doc_template_init(&dt);

	if (doc_template_open(&dt, "templates/shutdown.html") == -1) {
		ret = -1; goto END;
	}
	if (doc_template_send(&dt, s,
			"time", get_time_str(),
			NULL) == -1)
	{
		ret = -1; goto END;
	}

	request->shutdown = 1;

END:	doc_template_free(&dt);

	return ret;
}


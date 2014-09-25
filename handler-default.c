#include <string.h>
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

int handler_default(lingvo_server_request *request, int s)
{
	int ret = 1;
	doc_template dt;
	char *str = NULL;


	doc_template_init(&dt);

	if (doc_template_open(&dt, "templates/homepage.html") == -1) {
		ret = -1; goto END;
	}
	str = strndup(request->request_string, request->request_string_len);
	if (str == NULL) {
		ret = -1; goto END;
	}
	if (doc_template_send(&dt, s,
			"time", get_time_str(),
			NULL) == -1)
	{
		ret = -1; goto END;
	}

END:	doc_template_free(&dt);
	if (str != NULL)
		free(str);

	return ret;
}


#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <domutils/string.h>
#include <time.h>

#include "lingvo-server-request.h"
#include "doc-template.h"




static int get_files(const char *dirname, domutils_string *str)
{
	DIR *dir = NULL;
	struct dirent entry, *result;
	int ret = 1;


	domutils_string_append(str, "");

	dir = opendir(dirname);
	if (dir == NULL) {
		if (errno == ENOENT) {
			printf("error: Can't open dir '%s'.\n",
					dirname);
			ret = 0;
		}
		else {
			printf("%s (%d).\n", strerror(errno), errno);
			ret = -1;
		}
		goto END;
	}

	for (;;) {
		if (readdir_r(dir, &entry, &result) > 0) {
			ret = -1; goto END;
		}
		if (result == NULL)
			break;
		if (strcmp(entry.d_name, ".") == 0 ||
			strcmp(entry.d_name, "..") == 0)
		{
			continue;
		}

		domutils_string_append_printf(str,
				"<a href=\"/file/%s\">%s</a><br>",
				entry.d_name, entry.d_name);
	}

END:	if (dir == NULL)
		closedir(dir);

	return ret;
}

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
	int ret = 1, res;
	doc_template dt;
	domutils_string files_str;


	doc_template_init(&dt);
	domutils_string_init(&files_str);


	res = get_files(LINGVO_FILES_DIR, &files_str);
	if (res == -1) {
		ret = -1; goto END;
	}
	else
	if (res == 0) {
		domutils_string_append(&files_str,
				"Файлы не были загружены.");
	}

	if (doc_template_open(&dt, "templates/homepage.html") == -1) {
		ret = -1; goto END;
	}
	if (doc_template_send(&dt, s,
			"time", get_time_str(),
			"files", files_str.data,
			NULL) == -1)
	{
		ret = -1; goto END;
	}

END:	doc_template_free(&dt);
	domutils_string_free(&files_str);

	return ret;
}


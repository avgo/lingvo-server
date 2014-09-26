#include <stdio.h>
#include <lingvo/word_list_stw.h>

#include "lingvo-server-request.h"
#include "lingvo-server-utils.h"
#include "doc-template.h"




static int fill_wordlist(const char *filename, domutils_string *str)
{
	domutils_string_append_printf(str, "файл: '%s'", filename);

	return 1;
}

static const char* get_filename(const char *dir, const char *str)
{
	static char filename[1000];


	if (*str == '/')
		++str;

	for ( ; *str != '\0'; ++str) {
		if (*str == '/') {
			++str;
			break;
		}
	}

	snprintf(filename, sizeof(filename), "%s/%s",
			dir, str);

	return filename;
}

int handler_file(lingvo_server_request *request, int s)
{
	int ret = 1, res;
	doc_template dt;
	domutils_string str;


	doc_template_init(&dt);
	domutils_string_init(&str);


	fill_wordlist(get_filename(LINGVO_FILES_DIR, request->query),
			&str);
	if (doc_template_open(&dt, "templates/file.html") == -1) {
		ret = -1; goto END;
	}

	if (doc_template_send(&dt, s,
			"wordlist", str.data,
			NULL) == -1)
	{
		ret = -1; goto END;
	}

END:	doc_template_free(&dt);
	domutils_string_free(&str);

	return ret;
}

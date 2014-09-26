#include <stdio.h>
#include <lingvo/word_list_stw.h>

#include "lingvo-server-request.h"
#include "lingvo-server-utils.h"
#include "doc-template.h"




static int fill_wordlist(const char *filename, domutils_string *result_str)
{
	int ret = 1;
	domutils_string str;
	lingvo_word_list_stw wl_stw;


	domutils_string_init(&str);
	lingvo_word_list_stw_init(&wl_stw);


	if (domhp_file_to_domutils_str(filename, &str, 1 << 20) == -1) {
		ret = -1; goto END;
	}

	if (lingvo_word_list_stw_create(&wl_stw, str.data) == -1) {
		ret = -1; goto END;
	}

	domutils_string_append(result_str, "");

	lingvo_word_list_stw_node *node;

	for (node = wl_stw.first; node != NULL; node = node->next) {
		domutils_string_append_printf(result_str, "%.*s<br>",
				(int) (node->occ_first->end - node->occ_first->begin),
				str.data + node->occ_first->begin);
	}

END:	domutils_string_free(&str);
	lingvo_word_list_stw_free(&wl_stw);

	return ret;
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

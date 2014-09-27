#include <stdio.h>
#include <lingvo/word_list_stw.h>
#include <unicode/utf8.h>

#include "lingvo-server-request.h"
#include "lingvo-server-utils.h"
#include "doc-template.h"




static int escape_string(domutils_string *esc_str, const char *str, int32_t str_len)
{
	int32_t i_next, i;
	UChar32 c;


	for (i_next = 0; i_next < str_len; ) {
		i = i_next;
		U8_NEXT(str, i_next, str_len, c);

		if (u_isprint(c) && c != L' ') {
			domutils_string_append_n(esc_str,
				str + i, i_next - i);
		}
		else {
			domutils_string_append_printf(esc_str,
				" <span class=\"not_alpha\">U+%X</span>",
				(unsigned int) c);
		}
	}
}

static int fill_wordlist(domutils_string *file_str, domutils_string *result_str)
{
	int ret = 1;
	domutils_string esc_str;
	lingvo_word_list_stw wl_stw;


	domutils_string_init(&esc_str);
	lingvo_word_list_stw_init(&wl_stw);


	if (lingvo_word_list_stw_create(&wl_stw, file_str->data) == -1) {
		ret = -1; goto END;
	}

	domutils_string_append(result_str, "");

	lingvo_word_list_stw_node *node;

	for (node = wl_stw.first; node != NULL; node = node->next) {
		domutils_string_free(&esc_str);
		domutils_string_init(&esc_str);

		int count = 0;

		for (lingvo_word_list_stw_occ_node *occ_node = node->occ_first;
					occ_node != NULL;
					occ_node = occ_node->next)
			++count;
		escape_string(&esc_str,
			file_str->data + node->occ_first->begin,
			(int) (node->occ_first->end - node->occ_first->begin));
		
		domutils_string_append_printf(result_str,
			"%s (%d)<br>", esc_str.data, count);
	}

END:	lingvo_word_list_stw_free(&wl_stw);
	domutils_string_free(&esc_str);

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
	domutils_string file_str;


	doc_template_init(&dt);
	domutils_string_init(&file_str);
	domutils_string_init(&str);


	if (domhp_file_to_domutils_str(get_filename(
			LINGVO_FILES_DIR, request->query),
			&file_str, 1 << 20) == -1) {
		ret = -1; goto END;
	}

	fill_wordlist(&file_str, &str);

	if (doc_template_open(&dt, "templates/file.html") == -1) {
		ret = -1; goto END;
	}

	if (doc_template_send(&dt, s,
			"wordlist", str.data,
			"file",     file_str.data,
			NULL) == -1)
	{
		ret = -1; goto END;
	}

END:	doc_template_free(&dt);
	domutils_string_free(&str);
	domutils_string_free(&file_str);

	return ret;
}

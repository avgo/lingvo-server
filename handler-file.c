#include <stdio.h>
#include <string.h>
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
		{
			++count;
		}

		escape_string(&esc_str,
			file_str->data + node->occ_first->begin,
			(int) (node->occ_first->end - node->occ_first->begin));
		
		domutils_string_append_printf(result_str,
				"<tr>\n"
				"  <td>%s</td>\n"
				"  <td>(%d)</td>\n"
				"</tr>\n", esc_str.data, count);
	}

END:	lingvo_word_list_stw_free(&wl_stw);
	domutils_string_free(&esc_str);

	return ret;
}

static const char* get_filename(const char *dir, const char *str)
{
	static char filename[1000];


	snprintf(filename, sizeof(filename), "%s/%s",
			dir, str);

	return filename;
}

struct handler_file_options_ {
	char *filename;
	char *action;
};

typedef struct handler_file_options_ handler_file_options;

static int handler_file_options_get(handler_file_options *opt, const char *query)
{
	const char *p1, *p2, *p3;


	p1 = query;
	if (*p1 == '/')
		++p1;

	for ( ; ; ++p1) {
		if (*p1 == '\0')
			return 1;
		if (*p1 == '/') {
			++p1;
			break;
		}
	}

	for (p2 = p1; *p2 != '/' && *p2 != '\0'; ++p2)
		;

	if (p1 == p2)
		return 1;

	opt->filename = strndup(p1, p2 - p1);
	if (opt->filename == NULL)
		return -1;

	if (*p2 == '\0')
		return 1;

	p3 = p2 + 1;

	if (*p3 == '\0')
		return 1;

	opt->action = strdup(p3);
	if (opt->action == NULL)
		return -1;

	return 1;
}

static int handler_file_options_free(handler_file_options *opt)
{
	if (opt->filename == NULL)
		free(opt->filename);
	if (opt->action == NULL)
		free(opt->action);

	return 1;
}

static int handler_file_options_init(handler_file_options *opt)
{
	opt->filename = NULL;
	opt->action = NULL;

	return 1;
}

int handler_file(lingvo_server_request *request, int s)
{
	int ret = 1, res;
	doc_template dt;
	domutils_string str;
	domutils_string file_str;
	handler_file_options opt;


	doc_template_init(&dt);
	domutils_string_init(&file_str);
	domutils_string_init(&str);
	handler_file_options_init(&opt);


	handler_file_options_get(&opt, request->query);

	printf(	"file: '%s'\n"
		"action: '%s'\n\n",
		opt.filename, opt.action);

	if (opt.action == NULL) {
		if (doc_template_open(&dt, "templates/file.html") == -1) {
			ret = -1; goto END;
		}
	
		if (doc_template_send(&dt, s,
				"filename", opt.filename,
				NULL) == -1)
		{
			ret = -1; goto END;
		}
	}
	else
	if (strcmp(opt.action, "wordlist") == 0) {
		if (domhp_file_to_domutils_str(get_filename(
				LINGVO_FILES_DIR, opt.filename),
				&file_str, 1 << 20) == -1) {
			ret = -1; goto END;
		}

		fill_wordlist(&file_str, &str);
	
		if (doc_template_open(&dt, "templates/file.wordlist.html") == -1) {
			ret = -1; goto END;
		}
	
		if (doc_template_send(&dt, s,
				"wordlist", str.data,
				NULL) == -1)
		{
			ret = -1; goto END;
		}
	}
	else
	if (strcmp(opt.action, "card") == 0) {
		if (doc_template_open(&dt, "templates/file.card.html") == -1) {
			ret = -1; goto END;
		}
	
		if (doc_template_send(&dt, s,
				NULL) == -1)
		{
			ret = -1; goto END;
		}
	}
	else
	if (strcmp(opt.action, "file") == 0) {
		if (domhp_file_to_domutils_str(get_filename(
				LINGVO_FILES_DIR, opt.filename),
				&file_str, 1 << 20) == -1) {
			ret = -1; goto END;
		}
	
		if (doc_template_open(&dt, "templates/file.file.html") == -1) {
			ret = -1; goto END;
		}
	
		if (doc_template_send(&dt, s,
				"file",     file_str.data,
				NULL) == -1)
		{
			ret = -1; goto END;
		}
	}
	else
	if (strcmp(opt.action, "top") == 0) {
		if (doc_template_open(&dt, "templates/file.top.html") == -1) {
			ret = -1; goto END;
		}
	
		if (doc_template_send(&dt, s,
				NULL) == -1)
		{
			ret = -1; goto END;
		}
	}
	else {
		if (send_response(s,
			"<html>\n"
			"  <head>\n"
			"  </head>\n"
			"  <body>\n"
			"    <h1 style=\"color: #ff0000\">UNKNOWN ACTION '%s'</h1>"
			"  </body>\n"
			"</html>\n",
			opt.action) == -1)
		{
			ret = -1; goto END;
		}
	}

END:	doc_template_free(&dt);
	domutils_string_free(&str);
	domutils_string_free(&file_str);
	handler_file_options_free(&opt);

	return ret;
}

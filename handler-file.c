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

static int escape_string_to_js(domutils_string *esc_str_for_js,
		const char *str, int32_t str_len)
{
	int32_t i_next, i;
	UChar32 c;
	char *ec;


	for (i_next = 0; i_next < str_len; ) {
		i = i_next;
		U8_NEXT(str, i_next, str_len, c);

		switch (c) {
		case L'\b': ec = "\\b"; break;
		case L'\f': ec = "\\f"; break;
		case L'\n': ec = "\\n"; break;
		case L'\r': ec = "\\r"; break;
		case L'\t': ec = "\\t"; break;
		case L'\v': ec = "\\v"; break;
		case L'\'': ec = "\\'"; break;
		case L'\"': ec = "\\\""; break;
		default: ec = NULL; break;
		}

		if (ec == NULL)
			domutils_string_append_n(esc_str_for_js, str + i, i_next - i);
		else
			domutils_string_append(esc_str_for_js, ec);
	}
}

static int fill_wordlist(domutils_string *file_str,
		domutils_string *result_str, domutils_string *file_text)
{
	int ret = 1;
	lingvo_word_list_stw wl_stw;


	lingvo_word_list_stw_init(&wl_stw);


	if (lingvo_word_list_stw_create(&wl_stw, file_str->data) == -1) {
		ret = -1; goto END;
	}

	domutils_string_append(result_str, "");

	lingvo_word_list_stw_node *node;
	const char *next_line = "";

	for (node = wl_stw.first; node != NULL; node = node->next) {
		domutils_string esc_str, esc_str2;


		domutils_string_init(&esc_str);
		domutils_string_init(&esc_str2);


		int count = 0;

		for (lingvo_word_list_stw_occ_node *occ_node = node->occ.first;
					occ_node != NULL;
					occ_node = occ_node->next)
		{
			++count;
		}

		escape_string(&esc_str,
			file_str->data + node->occ.first->begin,
			(int) (node->occ.first->end - node->occ.first->begin));
		escape_string_to_js(&esc_str2, esc_str.data, esc_str.size - 1);

		domutils_string_append_printf(result_str,
				"%s{ word: \"%s\", count: %d, wordpositions: [ ",
				next_line, esc_str2.data, count);

		next_line = "";

		for (lingvo_word_list_stw_occ_node *occ_node = node->occ.first;
					occ_node != NULL;
					occ_node = occ_node->next)
		{
			domutils_string_append_printf(result_str,
					"%s%d", next_line, occ_node->begin);
			next_line = ", ";
		}
		domutils_string_append(result_str, " ] }");

		next_line = ",\n      ";

		domutils_string_free(&esc_str);
		domutils_string_free(&esc_str2);
	}

	for (lingvo_word_list_stw_occ_node *occ_node = wl_stw.occ.first;
			occ_node != NULL; occ_node = occ_node->next)
	{
		domutils_string word;


		domutils_string_init(&word);

		domutils_string_append_printf(&word,
				"<span id=\"word_%d\">%.*s</span>",
				occ_node->begin,
				occ_node->end - occ_node->begin,
				file_str->data + occ_node->begin);
		escape_string_to_js(file_text, word.data, word.size - 1);

		domutils_string_free(&word);
	}

END:	lingvo_word_list_stw_free(&wl_stw);

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
	domutils_string file_text;
	handler_file_options opt;


	doc_template_init(&dt);
	domutils_string_init(&file_str);
	domutils_string_init(&file_text);
	domutils_string_init(&str);
	handler_file_options_init(&opt);


	handler_file_options_get(&opt, request->query);

	printf(	"file: '%s'\n"
		"action: '%s'\n\n",
		opt.filename, opt.action);

	if (opt.action == NULL) {
		if (domhp_file_to_domutils_str(get_filename(
				LINGVO_FILES_DIR, opt.filename),
				&file_str, 1 << 20) == -1) {
			ret = -1; goto END;
		}

		fill_wordlist(&file_str, &str, &file_text);
	
		if (doc_template_open(&dt, "templates/file.html") == -1) {
			ret = -1; goto END;
		}
	
		if (doc_template_send(&dt, s,
				"filename", opt.filename,
				"file_text", file_text.data,
				"wordlist", str.data,
				NULL) == -1)
		{
			ret = -1; goto END;
		}
	}
	else
	if (strcmp(opt.action, "wordlist") == 0) {
		if (doc_template_open(&dt, "templates/file.wordlist.html") == -1) {
			ret = -1; goto END;
		}
	
		if (doc_template_send(&dt, s,
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
		if (doc_template_open(&dt, "templates/file.file.html") == -1) {
			ret = -1; goto END;
		}
	
		if (doc_template_send(&dt, s,
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
	domutils_string_free(&file_text);
	handler_file_options_free(&opt);

	return ret;
}

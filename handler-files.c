#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <domutils/domhp.h>

#include "content-types.h"
#include "doc-template.h"
#include "lingvo-server-request.h"
#include "lingvo-server-utils.h"




struct content_type_ {
	const char *content_type;
	const char *extension;
};

typedef struct content_type_ content_type;

struct handler_files_options_ {
	const char *filename;
};

typedef struct handler_files_options_ handler_files_options;




static const char *get_content_type_by_ext(const char *filename);
static void handler_files_options_get(handler_files_options *opt, const char *query);
static void handler_files_options_init(handler_files_options *opt);




static const content_type content_types[] = {
	{ CONTENT_TYPE_APPLICATION_X_JAVASCRIPT, ".js"   },
	{ CONTENT_TYPE_TEXT_HTML,                ".html" },
	{ CONTENT_TYPE_TEXT_HTML,                ".htm"  },
};

int content_types_count = sizeof(content_types) / sizeof(*content_types);




static const char *get_content_type_by_ext(const char *filename)
{
	const char *ext;


	ext = strrchr(filename, '.');
	if (ext == NULL)
		ext = filename;

	for (int i = 0; i < content_types_count; ++i) {
		if (strcmp(content_types[i].extension, ext) == 0)
			return content_types[i].content_type;
	}

	return NULL;
}

int handler_files(lingvo_server_request *request, int s)
{
	int ret = 1;
	handler_files_options opt;
	const char *content_type;
	domutils_string file_str;


	handler_files_options_init(&opt);
	domutils_string_init(&file_str);


	handler_files_options_get(&opt, request->query);
	content_type = get_content_type_by_ext(opt.filename);

	if (content_type == NULL) {
		printf("error: unknown content-type for file '%s'.\n",
				opt.filename);
		ret = 0; goto END;
	}

	if (send_response(s,
		"Content-type: %s\n"
		"\n",
		content_type) == -1)
	{
		ret = -1; goto END;
	}

	if (domhp_file_to_domutils_str(opt.filename, &file_str, 1 << 20) == -1)
	{
		ret = -1; goto END;
	}

	if (send_all(s, file_str.data, file_str.size - 1) == -1) {
		ret = -1; goto END;
	}

END:	domutils_string_free(&file_str);

	return ret;
}

static void handler_files_options_get(handler_files_options *opt, const char *query)
{
	const char *p1 = query, *p2;

	for ( ; *p1 == '/'; ++p1)
		++p1;

	for (p2 = p1; ; ++p2) {
		if (*p2 == '\0') {
			opt->filename = p2;
			return ;
		}
		if (*p2 == '/') {
			opt->filename = p2 + 1;
			return ;
		}
	}
}

static void handler_files_options_init(handler_files_options *opt)
{
	opt->filename = NULL;
}

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lingvo-server-utils.h"
#include "multipart-data.h"




static int multipart_data_parse_content_disposition(
		multipart_data *mp_data, const char *s1, const char *s1_end,
		char **name, char **filename);




int multipart_data_add_frame(multipart_data *mp_data,
		const char *frame_b, const char *frame_e)
{
	const char *c1, *c2, *c3, *c4;
	char *name = NULL, *filename = NULL, *content_type = NULL;
	int ret = 1;
	multipart_data_frame *new_node = NULL;


	for (c1 = c4 = frame_b; c1 < frame_e && *c1 != '\r' && *c1 != '\n'; ) {
		for ( ; ; ++c4) {
			if (c4 == frame_e) {
				printf("warning: wrong frame ('\\r' or '\\n' expected).\n");
				ret = 0; goto END;
			}
			if (*c4 == '\r' || *c4 == '\n')
				break;
		}
		for (c2 = c1; ; ++c2) {
			if (c2 == c4) {
				printf("warning: wrong frame (':' expected).\n");
				ret = 0; goto END;
			}
			if (*c2 == ':')
				break;
		}
		for (c3 = c2 + 1; ; ++c3) {
			if (c3 == c4) {
				printf("warning: wrong frame (expression expected after ':').\n");
				ret = 0; goto END;
			}
			if (*c3 != ' ')
				break;
		}

		if (parameter_parse(c1, c2, "Content-Disposition") == 0) {
			if (multipart_data_parse_content_disposition(
					mp_data, c3, c4, &name, &filename) == -1)
			{
				ret = -1; goto END;
			}
		}
		else
		if (parameter_parse(c1, c2, "Content-Type") == 0) {
			if (content_type != NULL)
				free(content_type);
			content_type = strndup(c3, c4 - c3);
			if (content_type == NULL) {
				ret = -1; goto END;
			}
		}

		if (*c4 == '\r') ++c4;
		if (*c4 == '\n') ++c4; /* !!! */
		c1 = c4;
	}

	if (*c1 == '\r') ++c1;
	if (*c1 == '\n') ++c1;

	new_node = malloc(sizeof *new_node);
	if (new_node == NULL) {
		ret = -1; goto END;
	}

	new_node->file_b = c1;
	new_node->file_e = frame_e;
	new_node->content_type = content_type;
	new_node->name = name;
	new_node->filename = filename;

	new_node->next = NULL;

	if (mp_data->first == NULL)
		mp_data->first = new_node;
	else
		mp_data->last->next = new_node;

	mp_data->last = new_node;

	return ret;

END:	if (name != NULL)
		free(name);
	if (filename != NULL)
		free(filename);
	if (content_type != NULL)
		free(content_type);

	return ret;
}

void multipart_data_free(multipart_data *mp_data)
{
	multipart_data_frame *f, *next;

	for (f = mp_data->first; f != NULL; f = next) {
		next = f->next;
		free(f);
	}
}

void multipart_data_init(multipart_data *mp_data)
{
	mp_data->first = NULL;
	mp_data->last = NULL;
}

static int multipart_data_parse_content_disposition(
		multipart_data *mp_data, const char *s1, const char *s1_end,
		char **name, char **filename)
{
	const char *c2, *c3;


	for (c2 = s1; ; ++c2) {
		if (c2 == s1_end)
			return 0;
		if (*c2 == ';')
			break;
	}

	if (parameter_parse(s1, c2, "form-data") != 0)
		return 0;

	for (++c2; ; ++c2) {
		if (c2 == s1_end)
			return 0;
		if (*c2 != ' ')
			break;
	}

	for (;;) {
		int result;

		result = parameter_save(c2, s1_end, "name", name, &c3);
		if (result != PAR_SAVE_NOT_MATCH)
			goto L1;
		result = parameter_save(c2, s1_end, "filename", filename, &c3);
	L1:	if (result == PAR_SAVE_SYSERR)
			return -1;
		if (result == PAR_SAVE_NOT_PAR) {
			printf("warning: parameter not matched in '%.*s'.\n",
					(int) (s1_end - c2), c2);
			return 0;
		}

		for (c2 = c3; ; ++c2) {
			if (c2 == s1_end)
				return 0;
			if (*c2 != ' ')
				break;
		}

		if (*c2 != ';')
			return 0;

		for (++c2; ; ++c2) {
			if (c2 == s1_end)
				return 0;
			if (*c2 != ' ')
				break;
		}
	}
}

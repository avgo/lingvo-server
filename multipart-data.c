#include <stdlib.h>
#include <stdio.h>

#include "lingvo-server-utils.h"
#include "multipart-data.h"




int multipart_data_add_frame(multipart_data *mp_data,
		const char *frame_b, const char *frame_e)
{
	const char *c1, *c2, *c3, *c4;


	for (c1 = c4 = frame_b; c1 < frame_e && *c1 != '\r' && *c1 != '\n'; ) {
		for ( ; ; ++c4) {
			if (c4 == frame_e) {
				printf("warning: wrong frame ('\\r' or '\\n' expected).\n");
				return 0;
			}
			if (*c4 == '\r' || *c4 == '\n')
				break;
		}
		for (c2 = c1; ; ++c2) {
			if (c2 == c4) {
				printf("warning: wrong frame (':' expected).\n");
				return 0;
			}
			if (*c2 == ':')
				break;
		}
		for (c3 = c2 + 1; ; ++c3) {
			if (c3 == c4) {
				printf("warning: wrong frame (expression expected after ':').\n");
				return 0;
			}
			if (*c3 != ' ')
				break;
		}

#if 0
'Content-Disposition' 'form-data; name="textfile1"; filename="lingvo-server-request.c"'
'Content-Type' 'text/x-c'
'Content-Disposition' 'form-data; name="hidden1"'
'Content-Disposition' 'form-data; name="text1"'
#endif

		if (parameter_parse(c1, c2, "Content-Disposition") == 0) {
			const char *c33 = c3;
			while (*c33 != ';')
				++c33;
			if (parameter_parse(c3, c33, "form-data") == 0)
				printf("Form data.\n");
		}
		else
		if (parameter_parse(c1, c2, "Content-Type") == 0) {
			printf("'%.*s'\n", (int) (c4 - c3), c3);
		}

		if (*c4 == '\r') ++c4;
		if (*c4 == '\n') ++c4; /* !!! */
		c1 = c4;
	}
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

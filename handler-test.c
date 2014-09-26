#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <domutils/string.h>

#include "lingvo-server-request.h"
#include "doc-template.h"




static int save_file(const char *filename, const char *b, const char *e)
{
	int f = -1;
	char buf[1000];


	snprintf(buf, sizeof(buf), "/tmp/saved_%s", filename);
	f = open(buf, O_CREAT | O_WRONLY, 0644);

	if (f == -1) {
		printf("err\n");
		return -1;
	}

	write(f, b, e - b);
	close(f);
}

int handler_test(lingvo_server_request *request, int s)
{
	int ret = 1;
	doc_template dt;
	domutils_string str;
	int is_files = 0;


	doc_template_init(&dt);
	domutils_string_init(&str);


	for (multipart_data_frame *f = request->mp_data.first;
			f != NULL; f = f->next)
	{
		if (f->filename != NULL && f->filename[0] != '\0') {
			is_files = 1;
			break;
		}
	}

	if (is_files) {
		domutils_string_append(&str, "Пришли следующие файлы:\n");
	}
	else {
		domutils_string_append(&str,
			"Ни одного файла не пришло.\n"
			"Отправьте файл с помощью формы.\n");
	}

	for (multipart_data_frame *f = request->mp_data.first;
			f != NULL; f = f->next)
	{
		if (f->filename != NULL && f->filename[0] != '\0') {
			domutils_string_append_printf(&str,
					"%s (%s, %ld байт)\n",
					f->filename,
					f->content_type,
					(int) (f->file_e - f->file_b));
		}
	}

	if (doc_template_open(&dt, "templates/test.html") == -1) {
		ret = -1; goto END;
	}
	if (doc_template_send(&dt, s,
			"file", str.data,
			NULL) == -1)
	{
		ret = -1; goto END;
	}

END:	doc_template_free(&dt);

	return ret;
}

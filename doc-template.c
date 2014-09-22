#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "doc-template.h"




static int doc_template_add_link(doc_template *dte,
		const char *s1, const char *s2,
		const char *s3, const char *s4)
{
	if (s1 == NULL) {
		printf("dte_text: '%.*s'\n",
			(int) (s4 - s3),
			s3);
	}
	else {
		printf("dte_link: '%.*s'\n",
			(int) (s2 - s1),
			s1);
	}

	return 1;
}

void doc_template_free(doc_template *dte)
{
}

void doc_template_init(doc_template *dte)
{
	dte->first = NULL;
	dte->last = NULL;

	domutils_string_init(&dte->text);
}

int doc_template_open(doc_template *dte, const char *filename)
{
	if (domhp_file_to_domutils_str(filename,
			&dte->text, 1 << 20) == -1) {
		return -1;
	}

	const char *s1, *s2, *s_end;

	s1 = s2 = dte->text.data;
	s_end = dte->text.data + (dte->text.size - 1);

	for (;;) {
		s2 = strstr(s1, "<?");
		if (s2 == NULL)
			s2 = s_end;
		if (doc_template_add_link(dte, NULL, NULL, s1, s2) == -1)
			return -1;
		if (s2 == s_end)
			break;
		s1 = s2 + 2;
		s2 = strstr(s1, "?>");
		if (s2 == NULL) {
			return 1;
		}
		if (doc_template_add_link(dte, s1, s2, NULL, NULL) == -1)
			return -1;
		s1 = s2 + 2;
	}
}

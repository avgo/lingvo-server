#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "doc-template.h"
#include "lingvo-server-utils.h"




static int doc_template_add_link(doc_template *dte,
		const char *s1, const char *s2,
		const char *s3, const char *s4)
{
	dte_link *new_node;


	new_node = malloc(sizeof(dte_link));
	if (new_node == NULL)
		return -1;

	new_node->pat = s1;
	new_node->pat_end = s2;
	new_node->text = s3;
	new_node->text_end = s4;

	new_node->next = NULL;

	if (dte->first == NULL)
		dte->first = new_node;
	else
		dte->last->next = new_node;

	dte->last = new_node;

	return 1;
}

void doc_template_free(doc_template *dte)
{
	dte_link *next;


	domutils_string_free(&dte->text);
	for (dte_link *l = dte->first; l != NULL; l = next) {
		next = l->next;
		free(l);
	}
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

int doc_template_pat_subs_correspondence(doc_template *dte, va_list list)
{
	va_list aq;
	char *pat, *sub;
	int is_absent;


	while (pat = va_arg(list, char*)) {
		sub = va_arg(list, char*);
		if (sub == NULL) {
			return -1;
		}

		is_absent = 1;

		for (dte_link *l = dte->first; l != NULL; l = l->next) {
			if (l->pat != NULL && parameter_parse(
					l->pat,
					l->pat_end,
					pat) == 0)
			{
				l->text = sub;
				l->text_end = sub + strlen(sub);
				is_absent = 0;
			}
		}

		if (is_absent == 1) {
			printf("warning: Pattern '%s' is absent in the document.\n",
					pat);
			break;
		}
	}

	return 1;
}

int doc_template_send(doc_template *dte, int sock, ...)
{
	va_list ap;
	int ret = 1;
	char *str = NULL, *str_p;


	va_start(ap, sock);
	ret = doc_template_pat_subs_correspondence(dte, ap);
	va_end(ap);

	if (ret == -1)
		goto END;

	for (dte_link *l = dte->first; l != NULL; l = l->next) {
		if (l->pat != NULL && l->text == NULL) {
			printf("warning: Pattern '%.*s' will be not substituted.\n",
					(int) (l->pat_end - l->pat),
					l->pat);
			l->text = l->pat - 1;
			l->text_end = l->pat_end + 1;
		}
	}

	int str_len = 0;

	for (dte_link *l = dte->first; l != NULL; l = l->next) {
		str_len += l->text_end - l->text;
	}

	str = malloc(str_len + 1);
	if (str == NULL) {
		ret = -1; goto END;
	}

	str_p = str;
	for (dte_link *l = dte->first; l != NULL; l = l->next) {
		memcpy(str_p, l->text, l->text_end - l->text);
		str_p += l->text_end - l->text;
	}

	str[str_len] = '\0';

	if (write(sock, str, str_len) != str_len) {
		printf("write(): %s (%u)\n", strerror(errno), errno);
		ret = -1; goto END;
	}

END:	if (str != NULL)
		free(str);

	return ret;
}

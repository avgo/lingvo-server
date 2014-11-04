#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "query-string.h"
#include "lingvo-server-utils.h"




typedef int (*query_action_proc)(void*);




int query_string_add(query_string *qs,
		const char *p1, const char *p2,
		const char *p3, const char *p4);




int query_action_do(query_string *qs, query_action *actions, int actions_count, const char *name, void *args)
{
	query_action *actions_end = actions + actions_count;
	const char *action;


	action = query_string_get(qs, name);
	if (action == NULL)
		return QUERY_ACTION_NO_ATTR;

	for (query_action *act = actions;
			act != actions_end; ++act)
	{
		if (strcmp(act->name, action) == 0)
			return ((query_action_proc) act->proc)(args);
	}

	return QUERY_ACTION_NO_ACTION;
}

int query_string_add(query_string *qs,
		const char *p1, const char *p2,
		const char *p3, const char *p4)
{
	int ret = 1;
	query_string_item *new_item = NULL;


	new_item = malloc(sizeof(query_string_item));
	if (new_item == NULL) {
		ret = -1; goto END;
	}

	new_item->attribute = NULL;
	new_item->value = NULL;
	new_item->next = NULL;

	if (unescape_string(p1, p2, &new_item->attribute) < 1) {
		ret = -1; goto END;
	}

	if (unescape_string(p3, p4, &new_item->value) < 1) {
		ret = -1; goto END;
	}

	if (qs->items == NULL)
		qs->items = new_item;
	else
		qs->items_last->next = new_item;

	qs->items_last = new_item;

END:	if (ret == -1 && new_item != NULL) {
		if (new_item->attribute != NULL)
			free(new_item->attribute);
		if (new_item->value != NULL)
			free(new_item->value);
		free(new_item);
	}

	return ret;
}

void query_string_free(query_string *qs)
{
	query_string_item *next;


	for (query_string_item *item = qs->items;
			item != NULL; item = next)
	{
		next = item->next;

		if (item->attribute != NULL)
			free(item->attribute);
		if (item->value != NULL)
			free(item->value);

		free(item);
	}
}

const char* query_string_get(query_string *qs, const char *attribute)
{
	for (query_string_item *item = qs->items;
			item != NULL; item = item->next)
	{
		if (strcmp(item->attribute, attribute) == 0)
			return item->value;
	}

	return NULL;
}

void query_string_init(query_string *qs)
{
	qs->items = NULL;
	qs->items_last = NULL;
}

int query_string_parse(query_string *qs, const char *qs_str, int qs_str_len)
{
	const char *p1, *p2, *p3, *p4, *qs_str_end;


	qs_str_end = qs_str + qs_str_len;

	for (p1 = qs_str; ; ) {
		for (p2 = p1; ; ++p2) {
			if (p2 == qs_str_end) {
				if (query_string_add(qs, p1, p2, NULL, NULL) == -1)
					return -1;
				return 0;
			}
			if (*p2 == '=')
				break;
		}
		p3 = p2 + 1;
		for (p4 = p3; ; ++p4) {
			if (p4 == qs_str_end || *p4 == '&')
				break;
		}

		if (query_string_add(qs, p1, p2, p3, p4) == -1)
			return -1;

		if (p4 == qs_str_end)
			break;
		else
			p1 = p4 + 1;
	}

	return 1;
}

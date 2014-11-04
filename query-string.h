#ifndef QUERY_STRING_H
#define QUERY_STRING_H




#define QUERY_ACTION_SYSERROR    -1
#define QUERY_ACTION_NO_ATTR     0
#define QUERY_ACTION_NO_ACTION   1
#define QUERY_ACTION_OK          2
#define QUERY_ACTION_USR_ERR     3




struct query_string_item_ {
	char *attribute;
	char *value;
	struct query_string_item_ *next;
};

typedef struct query_string_item_ query_string_item;

struct query_string_ {
	query_string_item *items;
	query_string_item *items_last;
};

typedef struct query_string_ query_string;

struct query_action_ {
	const char *name;
	void *proc;
};

typedef struct query_action_ query_action;




int query_action_do(query_string *qs, query_action *actions, int actions_count, const char *name, void *args);
void query_string_free(query_string *qs);
const char* query_string_get(query_string *qs, const char *attribute);
void query_string_init(query_string *qs);
int query_string_parse(query_string *qs, const char *qs_str, int qs_str_len);




#endif /* QUERY_STRING_H */

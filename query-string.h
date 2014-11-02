#ifndef QUERY_STRING_H
#define QUERY_STRING_H




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




void query_string_free(query_string *qs);
const char* query_string_get(query_string *qs, const char *attribute);
void query_string_init(query_string *qs);
int query_string_parse(query_string *qs, const char *qs_str, int qs_str_len);




#endif /* QUERY_STRING_H */

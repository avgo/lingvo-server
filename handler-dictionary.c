#include <lingvo/dict_maria_db.h>
#include <lingvo/lingvo.h>

#include <stdio.h>
#include <string.h>

#include "lingvo-server-request.h"
#include "lingvo-server-utils.h"
#include "query-string.h"




struct dict_action_ {
	int s;
	lingvo_server_request *request;
	query_string *qs;
	lingvo_dictionary *dict;
};

typedef struct dict_action_ dict_action;




int action_add(dict_action *da);
int action_delete(dict_action *da);




query_action actions[] = {
	{ "add",    action_add },
	{ "delete", action_delete }
};

int actions_count = sizeof(actions) / sizeof(*actions);




int action_add(dict_action *da)
{
	int ret = QUERY_ACTION_OK;
	const char *word;
	const char *message = "ok";
	int result;


	word = query_string_get(da->qs, "word");

	if (word == NULL) {
		printf("error: empty 'word' parameter.\n");
		message = "fail";
	}
	else {
		printf("word: '%s'\n", word);

		result = lingvo_dictionary_add_word(
			da->dict, word, WORD_TYPE_P_UNKNOWN);
		if (result == -1) {
			ret = QUERY_ACTION_SYSERROR; goto END;
		}
	}

	if (send_response(da->s, message) == -1) {
		ret = QUERY_ACTION_SYSERROR; goto END;
	}

END:	return ret;
}

int action_delete(dict_action *da)
{
	int ret = QUERY_ACTION_OK;
	const char *word;
	const char *message = "ok";
	int result;


	word = query_string_get(da->qs, "word");

	if (word == NULL) {
		printf("error: empty 'word' parameter.\n");
		message = "fail";
	}
	else {
		printf("delete word: '%s'\n", word);

		result = lingvo_dictionary_delete_word(da->dict, word);
		if (result == -1) {
			ret = QUERY_ACTION_SYSERROR; goto END;
		}
	}

	if (send_response(da->s, message) == -1) {
		ret = QUERY_ACTION_SYSERROR; goto END;
	}

END:	return ret;
}

int handler_dictionary(lingvo_server_request *request, int s)
{
	int ret = 1;
	query_string qs;
	int result;
	dict_action da;
	lingvo_dictionary dict;


	query_string_init(&qs);
	lingvo_dict_maria_db_init(&dict);


	lingvo_dictionary_create(&dict);
	if (query_string_parse(&qs,
			request->terminator,
			request->content_length) == -1)
	{
		ret = -1; goto END;
	}

	da.s = s;
	da.request = request;
	da.qs = &qs;
	da.dict = &dict;

	result = query_action_do(&qs, actions, actions_count, "action", &da);
	switch (result) {
	case QUERY_ACTION_SYSERROR:
		printf("QUERY_ACTION_SYSERROR\n");
		ret = -1; goto END;
	case QUERY_ACTION_NO_ACTION:
		printf("QUERY_ACTION_NO_ACTION\n");
		break;
	case QUERY_ACTION_NO_ATTR:
		printf("QUERY_ACTION_NO_ATTR\n");
		break;
	case QUERY_ACTION_OK:
		break;
	default:
		printf("error: error in action.\n");
	}

END:	query_string_free(&qs);
	lingvo_dictionary_close(&dict);

	return ret;
}

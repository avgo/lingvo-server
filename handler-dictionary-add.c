#include <lingvo/dict_maria_db.h>

#include <stdio.h>
#include <string.h>

#include "lingvo-server-request.h"
#include "lingvo-server-utils.h"
#include "query-string.h"




int handler_dictionary_add(lingvo_server_request *request, int s)
{
	int ret = 1;
	query_string qs;
	int result;
	lingvo_dictionary dict;
	const char *word;
	char *message = NULL;


	query_string_init(&qs);
	lingvo_dict_maria_db_init(&dict);


	if (query_string_parse(&qs,
			request->terminator,
			request->content_length) == -1)
	{
		ret = -1; goto END;
	}

	word = query_string_get(&qs, "word");

	if (word == NULL) {
		printf("error: empty 'word' parameter.\n");
		message = "ok";
	}
	else {
		printf("word: '%s'\n", word);

		lingvo_dictionary_create(&dict);
		result = lingvo_dictionary_add_word(&dict, word);
		if (result > -1) {
			message = "ok";
		}
		else {
			message = "fail";
		}
	}

	putchar('\n');

	if (send_response(s, message) == -1) {
		ret = -1; goto END;
	}

END:	query_string_free(&qs);
	lingvo_dictionary_close(&dict);

	return ret;
}

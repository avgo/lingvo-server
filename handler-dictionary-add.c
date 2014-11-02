#include <lingvo/dict_maria_db.h>

#include <stdio.h>
#include <string.h>

#include "lingvo-server-request.h"
#include "lingvo-server-utils.h"




int handler_dictionary_add(lingvo_server_request *request, int s)
{
	int ret = 1;
	int result;
	lingvo_dictionary dict;
	char *word = NULL;
	char *message = NULL;
	char *unescaped = NULL;


	lingvo_dict_maria_db_init(&dict);


	word = strndup(request->terminator, request->content_length);
	if (word == NULL) {
		ret = -1; goto END;
	}
	printf("word: '%s'\n", word);
	if (unescape_string(word, NULL, &unescaped) == 1) {
		printf("unescaped: '%s'\n", unescaped);
	}
	else {
		printf("error: unescape_string().\n");
		goto END;
	}

	if (unescaped == NULL) {
		printf("error: unescaped == NULL.\n\n");
		goto END;
	}

	putchar('\n');

	lingvo_dictionary_create(&dict);
	result = lingvo_dictionary_add_word(&dict, unescaped);
	if (result > -1) {
		message = "ok";
	}
	else {
		message = "fail";
	}

	if (send_response(s, message) == -1) {
		ret = -1; goto END;
	}

END:	if (word != NULL)
		free(word);
	if (unescaped != NULL)
		free(unescaped);
	lingvo_dictionary_close(&dict);

	return ret;
}

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


	lingvo_dict_maria_db_init(&dict);


	lingvo_dictionary_create(&dict);
	word = strndup(request->terminator, request->content_length);
	if (word == NULL) {
		ret = -1; goto END;
	}

	result = lingvo_dictionary_add_word(&dict, word);
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
	lingvo_dictionary_close(&dict);

	return ret;
}

#include <stdio.h>

#include <lingvo/dict_maria_db.h>

#include "lingvo-server-request.h"
#include "lingvo-server-utils.h"




int handler_wordtypes(lingvo_server_request *request, int s)
{
	lingvo_dictionary dict;


	lingvo_dict_maria_db_init(&dict);


	if (lingvo_dictionary_create(&dict) == -1) {
		return -1;
	}

	if (send_response(s, "var wordtypes = [\n") == -1) {
		return -1;
	}

	for (int i = 0; i < dict.types_count; ++i) {
		send_response(s,
			"  { id: %u, name: \"%s\", name_short: \"%s\", name_lat: \"%s\" },\n",
			dict.types[i].id,
			dict.types[i].name,
			dict.types[i].name_short,
			dict.types[i].name_lat);
	}

	if (send_response(s, "];\n") == -1) {
		return -1;
	}

END:	lingvo_dictionary_close(&dict);

	return 1;
}

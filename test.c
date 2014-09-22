#include <stdio.h>
#include <string.h>

#include "doc-template.h"




int main(int argc, char *argv[])
{
	char *cmd;

	cmd = strrchr(argv[0], '/');
	if (cmd == NULL)
		cmd = argv[0];
	else
		++cmd;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <template>\n",
				argv[0]);
		return 1;
	}


	doc_template dt;

	doc_template_init(&dt);
	doc_template_open(&dt, argv[1]);
	doc_template_free(&dt);

	return 0;
}

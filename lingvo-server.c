#include <stdio.h>

#include "core.h"




int main(int argc, char *argv[])
{
	if (create_server() == -1)
		return 1;

	return 0;
}

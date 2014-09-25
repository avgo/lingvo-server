#include <stdlib.h>
#include <string.h>

#include "lingvo-server-utils.h"




int parameter_parse(const char *s1, const char *s1_end, const char *s2)
{
	for ( ; ; ++s1, ++s2) {
		if (s1 == s1_end) {
			if (*s2 == '\0')
				return 0;
			else
				return *s2;
		}
		if (*s2 == '\0')
			return -*s1;
		if (*s1 != *s2)
			return *s2 - *s1;
	}
}

/* -1 -- system error
 *  0 -- not parameter
 *  1 -- not match with 'name'
 *  2 -- parameter matched with 'name'
 */

int parameter_save(const char *p1, const char *p1_end,
		const char *name, char **value, const char **next)
{
	const char *p2, *p3, *p4;


	for (p2 = p1; ; ++p2) {
		if (p2 == p1_end) {
			*next = p1;
			return PAR_SAVE_NOT_PAR;
		}
		if (*p2 == ' ' || *p2 == '=')
			break;
	}

	for (p3 = p2; ; ++p3) {
		if (p3 == p1_end) {
			*next = p1;
			return PAR_SAVE_NOT_PAR;
		}
		if (*p3 != ' ')
			break;
	}

	if (*p3 != '=') {
		*next = p1;
		return PAR_SAVE_NOT_PAR;
	}

	for (++p3; ; ++p3) {
		if (p3 == p1_end) {
			*next = p1;
			return PAR_SAVE_NOT_PAR;
		}
		if (*p3 != ' ')
			break;
	}

	if (*p3 == '"') ++p3;

	for (p4 = p3; ; ++p4) {
		if (p4 == p1_end) {
			*next = p1;
			return PAR_SAVE_NOT_PAR;
		}
		if (*p4 == '"')
			break;
	}
	*next = p4 + 1;

	if (parameter_parse(p1, p2, name) != 0) {
		*next = p4;
		return PAR_SAVE_NOT_MATCH;
	}

	if (*value != NULL)
		free(*value);

	*value = strndup(p3, p4 - p3);
	if (*value == NULL)
		return PAR_SAVE_SYSERR;

	return PAR_SAVE_MATCH;
}

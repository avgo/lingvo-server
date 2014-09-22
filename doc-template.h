#ifndef DOC_TEMPLATE_H
#define DOC_TEMPLATE_H

#include <domutils/domhp.h>




struct dte_link_ {
	char *name;
	char *name_end;
	char *value;
};

typedef struct dte_link_ dte_link;

struct doc_template_ {
	domutils_string text;
	dte_link *first;
	dte_link *last;
};

typedef struct doc_template_ doc_template;




void doc_template_free(doc_template *dte);
void doc_template_init(doc_template *dte);
int doc_template_open(doc_template *dte, const char *filename);




#endif /* DOC_TEMPLATE_H */

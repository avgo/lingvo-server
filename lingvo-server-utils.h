#ifndef LINGVO_SERVER_UTILS_H
#define LINGVO_SERVER_UTILS_H

#define PAR_SAVE_SYSERR    -1
#define PAR_SAVE_NOT_PAR    0
#define PAR_SAVE_NOT_MATCH  1
#define PAR_SAVE_MATCH      2




int parameter_parse(const char *s1, const char *s1_end, const char *s2);
int parameter_save(const char *str, const char *str_end,
		const char *name, char **value, const char **next);




#endif /* LINGVO_SERVER_UTILS_H */

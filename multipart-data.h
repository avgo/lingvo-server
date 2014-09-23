#ifndef MULTIPART_DATA_H
#define MULTIPART_DATA_H




struct multipart_data_frame_ {
	char *file_b;
	char *file_e;
	char *content_type;
	char *name;
	char *filename;
	struct multipart_data_frame_ *next;
};

typedef struct multipart_data_frame_ multipart_data_frame;

struct multipart_data_ {
	multipart_data_frame *first;
	multipart_data_frame *last;
};

typedef struct multipart_data_ multipart_data;




int multipart_data_add_frame(multipart_data *mp_data,
		const char *frame_b, const char *frame_e);
void multipart_data_free(multipart_data *mp_data);
void multipart_data_init(multipart_data *mp_data);




#endif /* MULTIPART_DATA_H */

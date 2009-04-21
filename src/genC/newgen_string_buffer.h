/* $Id$ */

/* minimal a la java StringBuffer... */

#ifndef STRING_BUFFER_INCLUDED
#define STRING_BUFFER_INCLUDED

struct __string_buffer_head;
typedef struct __string_buffer_head * string_buffer;

string_buffer string_buffer_make(void);
void string_buffer_free(string_buffer *, bool);
string string_buffer_to_string(string_buffer);
void string_buffer_to_file(string_buffer, FILE *);
void string_buffer_append(string_buffer, string);
void string_buffer_append_dup(string_buffer, string);

#endif /* STRING_BUFFER_INCLUDED */


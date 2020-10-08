#ifndef HELPERS_H
#define HELPERS_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "./main.h"

void init_string(struct string *s);
void init_post_req(struct open_post_req *post, const char *name);
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s);
int str_startswith(const char *str, char *tocheck);
int postreq_exists(const char *name);
struct open_post_req *new_postreq(char *name);
int write_to_postreq(struct open_post_req *postreq, const char *string, size_t str_length);
void cleanup_postreqs();
#endif

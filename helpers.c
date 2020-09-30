#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "./main.h"



void init_string(struct string *s) {
	s->len = 0;
	s->ptr = malloc(s->len + 1);
	s->error = 0;
	if (s->ptr == NULL) {
		fprintf(stderr, "malloc() failed\n");
		exit(EXIT_FAILURE);
	}
	s->ptr[0] = '\0';
}

void init_post_req(struct open_post_req *post, const char *name) {
	post->name = malloc(strlen(name));
	strcpy(post->name, name);
	post->content_len = 0;
	post->content = malloc(post->content_len + 1);
	if(post->name == NULL | post->content == NULL){
		fprintf(stderr, "malloc() failed\n");
		exit(EXIT_FAILURE);
	}
	post->content[0] = '\0';
}

int postreq_exists(const char *name) {
	for (int i = 0; i < open_post_requests_length; i++) {
		printf("[postreq_exists]testing:%s, len:%li, i:%i\n", name, strlen(name), i);
		if(strcmp(name, open_post_requests[i]->name) == 0){
			return i;
		}
	}
	return -1;
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s) {
	size_t new_len = s->len + size * nmemb;
	s->ptr = realloc(s->ptr, new_len + 1);
	if (s->ptr == NULL) {
		fprintf(stderr, "realloc() failed\n");
		exit(EXIT_FAILURE);
	}
	memcpy(s->ptr + s->len, ptr, size * nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size * nmemb;
}

int str_startswith(const char *str, char *tocheck) {
	for (int i = 0; i < strlen(tocheck) - 1; i++) {
		if (*(str + i) != *(tocheck + i))
			return 1;
	}
	return 0;
}

struct open_post_req *new_postreq(char *name) {
	int ret = postreq_exists(name);
	if (ret == -1) {
		struct open_post_req *newpost = malloc(sizeof(struct open_post_req));
		init_post_req(newpost, name);
		open_post_requests_length += 1;
		open_post_requests = realloc(open_post_requests, open_post_requests_length * sizeof(struct open_post_req*));
		open_post_requests[open_post_requests_length - 1] = newpost;
		return newpost;
	} else {
		init_post_req(open_post_requests[ret], name);	
	}
}

int write_to_postreq(struct open_post_req *postreq, const char *string) {
	size_t new_len = postreq->content_len + strlen(string);
	postreq->content = realloc(postreq->content, new_len + 1);
	if(postreq->content == NULL) {
		fprintf(stderr, "realloc() failed\n");
		exit(EXIT_FAILURE);
	}
	memcpy(postreq->content + postreq->content_len, string, strlen(string));
	postreq->content[new_len] = '\n';
	postreq->content_len = new_len;

	return 0;
}


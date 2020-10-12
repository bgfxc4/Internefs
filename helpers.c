#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "helpers.h"
#include "main.h"

void init_string(struct string *s) {
	s->len = 0;
	s->ptr = NULL;
	s->error = 0;
	s->already_requested = 1;
}

void init_post_req(struct open_post_req *post, const char *name) {
	post->name = malloc(strlen(name) + 1);
	post->name_len = strlen(name);
	strcpy(post->name, name);
	post->content_len = 0;
	post->content = NULL;
	post->answ = malloc(sizeof(*(post->answ)));
	init_string(post->answ);
	post->next_element = NULL;
	post->prev_element = NULL;

	if(post->name == NULL || post->answ == NULL){
		fprintf(stderr, "malloc() failed\n");
		exit(EXIT_FAILURE);
	}
}

struct open_post_req *postreq_exists(const char *name) {
	for (struct open_post_req *i = open_post_requests_first; i != NULL; i = i->next_element) {
		printf("[postreq_exists]testing:%s, len:%li, i:%i\n", name, strlen(name), i);
		if(strcmp(name, i->name) == 0){
			return i;
		}
	}
	return NULL;
}

size_t writefunc_get(void *ptr, size_t size, size_t nmemb, struct string *s) {
	printf("[writefunc_get] called from curl\n");
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

size_t writefunc_post(void *ptr, size_t size, size_t nmemb, struct string *s) {
	printf("[writefunc_post] called from curl\n");
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

void delete_postreq(struct open_post_req *req) {
	printf("[delete_postreq] called\n\tdeleting: %s\n", req->name);

	if(open_post_requests_last == open_post_requests_first && open_post_requests_first == req) {
		printf("case1\n");
		open_post_requests_last = NULL;
		open_post_requests_first = NULL;
	} else if(open_post_requests_first == req) {
		printf("case2\n");
		req->next_element->prev_element = NULL;
	} else if(open_post_requests_last == req) {
		printf("case3\n");
		req->prev_element->next_element = NULL;
	} else {
		printf("case4\n");
		req->prev_element->next_element = req->next_element;
		req->next_element->prev_element = req->prev_element;
	}
	free(req->answ->ptr);
	free(req->answ);
	free(req->content);
	free(req->name);
	free(req);
}

struct open_post_req *new_postreq(const char *name) {
	struct open_post_req *ret = postreq_exists(name);
	if (ret == NULL) {
		struct open_post_req *newpost = malloc(sizeof(struct open_post_req));
		init_post_req(newpost, name);
		if(open_post_requests_first == NULL) {
			open_post_requests_first = newpost;
			open_post_requests_last = newpost;
		} else {
			newpost->prev_element = open_post_requests_last;
			open_post_requests_last->next_element = newpost;
			open_post_requests_last = newpost;
		}
		return newpost;
	} else {
		delete_postreq(ret);
		struct open_post_req *newpost = malloc(sizeof(struct open_post_req));
		init_post_req(newpost, name);
		if(open_post_requests_first == NULL) {
			open_post_requests_first = newpost;
			open_post_requests_last = newpost;
		} else {
			newpost->prev_element = open_post_requests_last;
			open_post_requests_last->next_element = newpost;
			open_post_requests_last = newpost;
		}
		return newpost;
	}
}

int write_to_postreq(struct open_post_req *postreq, const char *string, size_t str_length) {
	size_t new_len = postreq->content_len + str_length;
	postreq->content = realloc(postreq->content, new_len + 1);
	if(postreq->content == NULL) {
		fprintf(stderr, "realloc() failed\n");
		exit(EXIT_FAILURE);
	}
	memcpy(postreq->content + postreq->content_len, string, str_length);
	postreq->content[new_len] = '\0';
	postreq->content_len = new_len;
	return 0;
}

void cleanup_postreqs() {
	printf("[cleaning up] called\n\tcleaning up postreqs\n");
	for (struct open_post_req *i = open_post_requests_first; i != NULL; i = i->next_element) {
		if(i->prev_element != NULL) free(i->prev_element);
		free(i->answ->ptr);
		free(i->answ);
		free(i->content);
		free(i->name);
	}
	free(open_post_requests_last);
}


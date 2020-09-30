#define FUSE_USE_VERSION 30

#ifndef MAIN_H
#define MAIN_H

#define min(x, y) (x < y ? x : y)

#include <curl/curl.h>
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

extern struct open_post_req **open_post_requests;
extern int open_post_requests_length;

struct string {
  char *ptr;
  size_t len;
  int error;
};

struct open_post_req {
	char *name;
	char *content;
	int content_len;
};

enum HTTP_KEK_ERRORS {
	HTTP_ERROR_UNKNOWN = -1,
	HTTP_ERROR_PROTOCOL_ERROR = -2,
	HTTP_ERROR_INVALID_URL = -3,
	HTTP_ERROR_CANT_CONNECT_TO_SERVER = -4,
	HTTP_ERROR_NON_EXISTING_DOMAIN = -5
};

int http_get(const char *url, int urllength, struct string *s);
#endif

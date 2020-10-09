#ifndef HTTP_H
#define HTTP_H

#include <curl/curl.h>
#include <stdio.h>
#include "helpers.h"
#include "./main.h"

int http_get(const char *url, int urllength, struct string *s);
int http_post(struct open_post_req *req);
#endif

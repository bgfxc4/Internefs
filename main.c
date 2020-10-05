#define FUSE_USE_VERSION 30

#include <curl/curl.h>
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include "./main.h"
#include "./helpers.h"
#include "./fuse_ops.h"

CURL *curl;

struct open_post_req **open_post_requests;
int open_post_requests_length = 0;

int http_get(const char *url, int urllength, struct string *s) {

	CURLcode res;

	char *encodedURL = curl_easy_unescape(curl, url, urllength, NULL);

	printf("url: %s, length: %i, encodedURL: %s \n", url, urllength, encodedURL);

	init_string(s);

	if (curl) {

		curl_easy_setopt(curl, CURLOPT_URL, encodedURL);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, s);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed KEK: %s\n",
				  curl_easy_strerror(res));
			if (res == CURLE_UNSUPPORTED_PROTOCOL)
				s->error = -2;
			else if (res == CURLE_URL_MALFORMAT)
				s->error = -3;
			else if (res == CURLE_COULDNT_CONNECT)
				s->error = -4;
			else if (res == CURLE_COULDNT_RESOLVE_HOST ||
				   res == CURLE_COULDNT_RESOLVE_PROXY)
				s->error = -5;
			else
				s->error = -1;
		}
		// printf("s: %s slen: %i\n", s.ptr, s.len);
	}
	curl_free(encodedURL);
	return 0;
}


void testing() {
	new_postreq("test");
	new_postreq("test2");

	/*for (int i = 0; i < open_post_requests_length; i ++) {
		printf("%p test\n", open_post_requests[i]);
		if(open_post_requests[i] != NULL) {
			printf("name: %s\n", open_post_requests[i]->name);
		}	
	}*/
}

int main(int argc, char *argv[]) {
	//testing();
	curl = curl_easy_init();
	int ret = fuse_main(argc, argv, &operations, NULL);
	curl_easy_cleanup(curl);
	printf("[cleaning up]\n");
	cleanup_postreqs();
	return ret;
}

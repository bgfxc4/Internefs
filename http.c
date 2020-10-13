
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdio.h>
#include "helpers.h"
#include "main.h"

int http_get(const char *url, int urllength, struct string *s) {
	
	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();

	char *encodedURL = curl_easy_unescape(curl, url, urllength, NULL);

	printf("url: %s, length: %i, encodedURL: %s \n", url, urllength, encodedURL);

	init_string(s);

	if (curl) {

		curl_easy_setopt(curl, CURLOPT_URL, encodedURL);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc_get);
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
	}
	curl_free(encodedURL);
	curl_easy_cleanup(curl);
	return 0;
}

int http_post(struct open_post_req *req) {
	printf("[http_post] called\n\tname: %s content: %s\n", req->name, req->content);
	CURL *curl;
	CURLcode res;
	
	curl = curl_easy_init();
	init_string(req->answ);
	char *encodedURL = curl_easy_unescape(curl, req->name, req->name_len, NULL);
	if  (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, encodedURL);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc_post);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, req->answ);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req->content);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
 
		curl_easy_cleanup(curl);
	}
	curl_free(encodedURL);
	curl_global_cleanup();
	return 0;
}

int http_head(const char *url, int urllength, struct string *s) {
	
	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();

	char *encodedURL = curl_easy_unescape(curl, url, urllength, NULL);

	printf("url: %s, length: %i, encodedURL: %s \n", url, urllength, encodedURL);

	init_string(s);

	if (curl) {

		curl_easy_setopt(curl, CURLOPT_URL, encodedURL);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc_get);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, s);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
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
	}
	curl_free(encodedURL);
	curl_easy_cleanup(curl);
	return 0;
	
}


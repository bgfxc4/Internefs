#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include "main.h"
#include "helpers.h"
#include "fuse_ops.h"
#include "http.h"

struct open_post_req *open_post_requests_first = NULL;
struct open_post_req *open_post_requests_last = NULL;

void testing() {
	struct open_post_req *testreq = new_postreq("https://postman-echo.com/post");
	write_to_postreq(testreq, "foo1=bar1&foo2=bar2", 20);
	http_post(testreq);
}

int main(int argc, char *argv[]) {
	int ret = fuse_main(argc, argv, &operations, NULL);
	printf("[cleaning up]\n");
	cleanup_postreqs();
	return ret;
}

#include <string.h>
#define FUSE_USE_VERSION 30


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fuse.h>

#include "main.h"
#include "helpers.h"
#include "http.h"

int do_getattr(const char *path, struct stat *st) {
	printf("[getattr] Called\n");
	printf("\tAttributes of %s requested\n", path);	

	st->st_uid = getuid();
	st->st_gid = getgid();
	st->st_atime = time(NULL);
	st->st_mtime = time(NULL);
	if (strcmp(path, "/") == 0 || strcmp(path, "/get") == 0 || strcmp(path, "/post") == 0 || strcmp(path, "/head") == 0) {

		st->st_mode = S_IFDIR | 0755; // directory
		st->st_nlink = 2;
	} else if (str_startswith(path, "/get/") == 0) {
		st->st_mode = S_IFREG | 0664;
		st->st_nlink = 1;     // file
	} else if (str_startswith(path, "/post/") == 0) {
		const char *filename = path;
		filename += 6;
		if (postreq_exists(filename) == NULL) return -ENOENT;
		st->st_mode = S_IFREG | 0664;
		st->st_nlink = 1;     // file
	} else if (str_startswith(path, "/head") == 0) {
		st->st_mode = S_IFREG | 0664;
		st->st_nlink = 1;     // file
	} else {
		return -ENOENT;
	}

	return 0;
}

int do_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	printf("--> Getting The List of Files of %s\n", path);

	filler(buffer, ".", NULL, 0);  // Current Directory
	filler(buffer, "..", NULL, 0); // Parent Directory

	if (strcmp(path, "/") == 0) {
		filler(buffer, "get", NULL, 0);
		filler(buffer, "post", NULL, 0);
		filler(buffer, "head", NULL, 0);
	} else if (strcmp(path, "/post") == 0) {
		for (struct open_post_req *i = open_post_requests_first; i != NULL; i = i->next_element) {
			printf("%p test\n", i);
			if (i != NULL) {
				printf("%s\n", i->name);
				filler(buffer, i->name, NULL, 0);	
			}	
		}
	}
	
	return 0;
}

int	do_unlink(const char *path) {
	printf("[unlink] called\n\tdelete %s\n", path);
	if(str_startswith(path, "/post/") == 0) {
		path += 6;
		struct open_post_req *ret = postreq_exists(path);
		if(ret == NULL) {
			return -ENOENT;
		} else {
			delete_postreq(ret);
		}
	}
	return 0;
}

int do_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
	printf("--> Trying to read %s, %lu, %lu\n", path, offset, size);
struct string *answ;
	if (str_startswith(path, "/get/") == 0) {
		path += 5;
		answ = (struct string *)fi->fh;
	} else if (str_startswith(path, "/head/") == 0) {
		path += 6;
		answ = (struct string *)fi->fh;
	} else if (str_startswith(path, "/post/") == 0) {
		path += 6;
		struct open_post_req *p_ret = postreq_exists(path);
		if (p_ret != NULL) {
			answ = (struct string *)fi->fh;
			if (offset == 0) {
				http_post(p_ret);
				answ->len = p_ret->answ->len;
				answ->ptr = realloc(answ->ptr, answ->len + 1);
				strcpy(answ->ptr, p_ret->answ->ptr);
			}
			answ->error = 0;

		} else {
			return -ENOENT;
		}
	} else {
		return -1;
	}

	int ret;
	if (answ->error == 0) {
		memcpy(buffer, answ->ptr + offset, min(answ->len - offset, size));
		ret = min(answ->len - offset, size);
	} else {
		char *error;
		if (answ->error == HTTP_ERROR_UNKNOWN) {
			printf("unknown\n");
			return -1;
		}
		else if (answ->error == HTTP_ERROR_PROTOCOL_ERROR) {
			printf("prot err\n");
			return -EPROTONOSUPPORT;
		}
		else if (answ->error == HTTP_ERROR_INVALID_URL) {
			printf("invalid url\n");
			return -EFAULT;
		}
		else if (answ->error == HTTP_ERROR_CANT_CONNECT_TO_SERVER) {
			printf("cant connect to server\n");
			return -ENETUNREACH;
		}
		else if (answ->error == HTTP_ERROR_NON_EXISTING_DOMAIN) {
			printf("non existing domain\n");
			return -ENETUNREACH;
		}

		memcpy(buffer, error, strlen(error));
		ret = strlen(error);
			printf("unknown\n");
		if (answ->already_requested == 1) {
			answ->already_requested = 0;
			return ret;
		} else {
			return 0;
		}
	}
	return ret;
}

int do_write( const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info ) {
	printf("[write] called\n\twriting to %s\n", path);	
	
	if (str_startswith(path, "/post/") == 0) {
		char *reqname = malloc(strlen(path) + 1);
		strcpy(reqname, path);
		reqname += 6;
		struct open_post_req *ret = postreq_exists(reqname);
		reqname -= 6;
		free(reqname);
		if (ret != NULL) {
			write_to_postreq(ret, buffer, size);
		} else {
			return -1;
		} 
	}
	return size;
}

int do_open(const char *path, struct fuse_file_info *fi) {
	fi->direct_io = 1;
	printf("[open] called\n\topening  %s\n", path);

	if (str_startswith(path, "/get/") == 0) {
		struct string *answ = malloc(sizeof(*answ));
		path += 5;
		http_get(path, strlen(path), answ);
		fi->fh = (uint64_t)answ;
	} else if (str_startswith(path, "/head/") == 0) {
		struct string *answ = malloc(sizeof(*answ));
		path += 6;
		http_head(path, strlen(path), answ);
		fi->fh = (uint64_t)answ;
	} else if (str_startswith(path, "/post/") == 0) {
		struct string *answ = malloc(sizeof(*answ));
		init_string(answ);
		fi->fh = (uint64_t)answ;	
	}
	return 0;
}

int do_create(const char *path, mode_t mode, struct fuse_file_info *fi){
	printf("[create] called\n\tcreating  %s\n", path);
	if (str_startswith(path, "/post/") == 0) {
		const char *file_name = path;
		file_name += 6;
		new_postreq(file_name);
		struct string *answ = malloc(sizeof(*answ));
		init_string(answ);
		fi->fh = (uint64_t)answ;
	}
	return 0;
}

int do_truncate(const char *path, off_t offset) {
	printf("[truncate] called\n\ton %s with offset %li\n", path, offset);
	if (str_startswith(path, "/post/") == 0) {
		const char *file_name = path;
		file_name += 6;
		new_postreq(file_name);
	}
	return 0;
}

int do_release(const char *path, struct fuse_file_info *fi) {
	printf("[release] called\n\treleasing %s\n", path);
	
	if (str_startswith(path, "/get/") == 0) {
		struct string *tofree = (struct string *)fi->fh;
		free(tofree->ptr);
		free(tofree);
	} else if (str_startswith(path, "/head/") == 0) {
		struct string *tofree = (struct string *)fi->fh;
		free(tofree->ptr);
		free(tofree);	
	} else if (str_startswith(path, "/post/") == 0) {
		struct string *tofree = (struct string *)fi->fh;
		free(tofree->ptr);
		free(tofree);
	}
	return 0; 
}

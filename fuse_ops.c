#define FUSE_USE_VERSION 30

#include <stdio.h>
#include <stdlib.h>
#include <fuse.h>

#include "./helpers.h"
#include "./main.h"

int do_getattr(const char *path, struct stat *st) {
	printf("[getattr] Called\n");
	printf("\tAttributes of %s requested\n", path);

	st->st_uid = getuid();
	st->st_gid = getgid();
	st->st_atime = time(NULL);
	st->st_mtime = time(NULL);
	if (strcmp(path, "/") == 0 || strcmp(path, "/get") == 0 || strcmp(path, "/post") == 0) {

		st->st_mode = S_IFDIR | 0755; // directory
		st->st_nlink = 2;
	} else if (str_startswith(path, "/get/") == 0) {
		st->st_mode = S_IFREG | 0664;
		st->st_nlink = 1;     // file
	} else if (str_startswith(path, "/post/") == 0) {
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
	}else if (strcmp(path, "/post") == 0) {
		for (int i = 0; i < open_post_requests_length; i ++) {
			printf("%p test\n", open_post_requests[i]);
			if(open_post_requests[i] != NULL) {
				printf("%s\n", open_post_requests[i]->name);
				filler(buffer, open_post_requests[i]->name, NULL, 0);	
			}	
		}
	}
	
	return 0;
}

int do_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
	printf("--> Trying to read %s, %lu, %lu\n", path, offset, size);

	struct string answ;
	char *url = malloc(strlen(path) + 1);
	if (str_startswith(path, "/get/") == 0) {
		strcpy(url, path);
		url += 5;
		http_get(url, strlen(url), &answ);
		url -= 5;
		free(url);
	} else if (str_startswith(path, "/post/") == 0) {
		strcpy(url, path);
		url += 6;
		int p_ret = postreq_exists(url);
		if(p_ret != -1) {
			answ.len = open_post_requests[p_ret]->content_len;
			answ.ptr = malloc(answ.len);
			strcpy(answ.ptr, open_post_requests[p_ret]->content);
			printf("reading:%s\n", answ.ptr);
			answ.error = 0;
			url -= 6;
			free(url);
		} else {
			url -= 6;
			free(url);
			return -ENOENT;
		}
	} else {
		free(url);
		return -1;
	}

	int ret;
	if (answ.error == 0) {
		// printf("KE:LEN:%i\n", answ.len);
		memcpy(buffer, answ.ptr + offset, min(answ.len - offset, size));
		ret = min(answ.len - offset, size);
		printf("%li + %li + %li", size, answ.len, strlen(answ.ptr));
		// printf("%s", answ.ptr);
	} else {
		char *error;
		if (answ.error == HTTP_ERROR_UNKNOWN)
			error = "Internefs goes BLUB BLUB: something went wrong!\n";
		else if (answ.error == HTTP_ERROR_PROTOCOL_ERROR)
			error = "Internefs goes BLUB BLUB: KEK use another Protocol!\n";
		else if (answ.error == HTTP_ERROR_INVALID_URL)
			error = "Internefs goes BLUB BLUB: KEK format your URL right!\n";
		else if (answ.error == HTTP_ERROR_CANT_CONNECT_TO_SERVER)
			error = "Internefs goes BLUB BLUB: couldnt connect to the server!\n";
		else if (answ.error == HTTP_ERROR_NON_EXISTING_DOMAIN)
			error = "Internefs goes BLUB BLUB: KEK use an existing domain!\n";

		memcpy(buffer, error, strlen(error));
		ret = strlen(error);
	}
	if(size + offset >= answ.len) {
		free(answ.ptr);
	}
	//free(url);
	//free(answ);
	return ret;
}

int do_write( const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info ) {
	printf("[write] called\n\twriting to %s\n", path);	
	
	if (str_startswith(path, "/post/") == 0) {
		printf("writing: %s\n", buffer);
		char *reqname = malloc(strlen(path) + 1);
		strcpy(reqname, path);
		reqname += 6;
		int ret = postreq_exists(reqname);
		reqname -= 6;
		free(reqname);
		if(ret != -1) {
			write_to_postreq(open_post_requests[ret], buffer);
		} else {
			return -1;
		} 
	}
	return size;
}

int do_open(const char *path, struct fuse_file_info *fi) {
	fi->direct_io = 1;
	printf("[open] called\n\topening  %s\n", path);
	return 0;
}

int do_create (const char *path, mode_t mode, struct fuse_file_info *fi){
	printf("[create] called\n\tcreating  %s\n", path);
	return 0;
}

int do_truncate (const char *path, off_t offset) {
	printf("[truncate] called\n\ton %s with offset %li\n", path, offset);
	if(str_startswith(path, "/post/") == 0) {
		char *file_name = malloc(strlen(path));
		strcpy(file_name, path);
		file_name += 6;
		new_postreq(file_name);
		file_name -= 6;
		free(file_name);
	}
	return 0;
}

int	do_unlink(const char *path) {
	printf("[unlink] called\n\tdelete %s\n", path);
	return 0;
}

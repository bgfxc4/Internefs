#define FUSE_USE_VERSION 30

#include <curl/curl.h>
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len + 1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
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

int decode_url(char *url, int urllength) {}

struct string http_get(char *url, int urllength) {

  printf("url: %s, length: %i", url, urllength);

  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();

  struct string s;
  init_string(&s);

  if (curl) {

    curl_easy_setopt(curl, CURLOPT_URL, "https://example.com");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    }
    curl_easy_cleanup(curl);
    // printf("%s\n", s.ptr);
  }
  return s;
}

int str_startswith(const char *str, char *tocheck) {
  for (int i = 0; i < strlen(tocheck) - 1; i++) {
    if (*(str + i) != *(tocheck + i))
      return 1;
  }
  return 0;
}

static int do_getattr(const char *path, struct stat *st) {
  printf("[getattr] Called\n");
  printf("\tAttributes of %s requested\n", path);

  st->st_uid = getuid();
  st->st_gid = getgid();
  st->st_atime = time(NULL);
  st->st_mtime = time(NULL);
  if (strcmp(path, "/") == 0 || strcmp(path, "/get") == 0 ||
      strcmp(path, "/post") == 0) {

    st->st_mode = S_IFDIR | 0755; // directory
    st->st_nlink = 2;
  } else {
    st->st_mode = S_IFREG;
    st->st_nlink = 1; // file
    st->st_size = 40960;
  }

  return 0;
}

static int do_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
                      off_t offset, struct fuse_file_info *fi) {
  printf("--> Getting The List of Files of %s\n", path);

  filler(buffer, ".", NULL, 0);  // Current Directory
  filler(buffer, "..", NULL, 0); // Parent Directory

  if (strcmp(path, "/") == 0) {
    filler(buffer, "get", NULL, 0);
    filler(buffer, "post", NULL, 0);
  }

  return 0;
}

static int do_read(const char *path, char *buffer, size_t size, off_t offset,
                   struct fuse_file_info *fi) {
  printf("--> Trying to read %s, %u, %u\n", path, offset, size);

  fi->direct_io = 1;
  // printf("hjfdgbdfhgbfhg+++++%i++++++dfslkjndfjdnf", fi->direct_io);

  struct string answ;
  printf("%i", str_startswith(path, "/get"));
  char *url = malloc(strlen(path));
  if (str_startswith(path, "/get") == 0) {
    strcpy(url, path);
    url += 5;
    answ = http_get(url, strlen(url));

  } else
    return -1;

  memcpy(buffer, answ.ptr + offset, size);
  int ret = answ.len - offset;
  // free(answ.ptr);
  printf("%i + %i + %i", size, answ.len, strlen(answ.ptr));
  printf("%s", answ.ptr);
  return ret;
}

static struct fuse_operations operations = {
    .getattr = do_getattr,
    .readdir = do_readdir,
    .read = do_read,
};

int main(int argc, char *argv[]) {
  return fuse_main(argc, argv, &operations, NULL);
}

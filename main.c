#define min(x, y) (x < y ? x : y)

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
  int error;
};

enum HTTP_KEK_ERRORS {
  HTTP_ERROR_UNKNOWN = -1,
  HTTP_ERROR_PROTOCOL_ERROR = -2,
  HTTP_ERROR_INVALID_URL = -3,
  HTTP_ERROR_CANT_CONNECT_TO_SERVER = -4,
  HTTP_ERROR_NON_EXISTING_DOMAIN = -5
};

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

struct string http_get(char *url, int urllength) {

  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();

  char *encodedURL = curl_easy_unescape(curl, url, urllength, NULL);

  printf("url: %s, length: %i, encodedURL: %s ", url, urllength, encodedURL);

  struct string s;
  init_string(&s);

  if (curl) {

    curl_easy_setopt(curl, CURLOPT_URL, encodedURL);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed KEK: %s\n",
              curl_easy_strerror(res));
      if (res == CURLE_UNSUPPORTED_PROTOCOL)
        s.error = -2;
      else if (res == CURLE_URL_MALFORMAT)
        s.error = -3;
      else if (res == CURLE_COULDNT_CONNECT)
        s.error = -4;
      else if (res == CURLE_COULDNT_RESOLVE_HOST ||
               res == CURLE_COULDNT_RESOLVE_PROXY)
        s.error = -5;
      else
        s.error = -1;
    }
    curl_easy_cleanup(curl);
    printf("s: %s slen: %i\n", s.ptr, s.len);
  }
  curl_free(encodedURL);
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
    st->st_nlink = 1;     // file
    st->st_size = 100000; // //1MB
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

  struct string answ;
  printf("%i", str_startswith(path, "/get"));
  char *url = malloc(strlen(path) + 1);
  if (str_startswith(path, "/get") == 0) {
    strcpy(url, path);
    url += 5;
    answ = http_get(url, strlen(url));

  } else {
    return -1;
  }

  int ret;
  if (answ.error == 0) {
    // printf("KE:LEN:%i\n", answ.len);
    memcpy(buffer, answ.ptr + offset, min(answ.len - offset, size));
    ret = min(answ.len - offset, size);
    printf("%i + %i + %i", size, answ.len, strlen(answ.ptr));
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
  free(answ.ptr);
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

COMPILER = gcc
FILESYSTEM_FILES = main.c helpers.c fuse_ops.c http.c

build: $(FILESYSTEM_FILES)
	#$(COMPILER) $(FILESYSTEM_FILES) -o main `pkg-config fuse --cflags --libs` -lcurl
	$(COMPILER) -lcurl $(FILESYSTEM_FILES) -o internefs `pkg-config fuse --cflags --libs` #if the other one gives errors
clean:
	rm main

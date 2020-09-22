COMPILER = gcc
FILESYSTEM_FILES = main.c

build: $(FILESYSTEM_FILES)
	$(COMPILER) -lcurl $(FILESYSTEM_FILES) -o main `pkg-config fuse --cflags --libs`

clean:
	rm main

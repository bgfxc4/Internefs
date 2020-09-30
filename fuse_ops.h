#define FUSE_USE_VERSION 30

#ifndef FUSE_OPS_H
#define FUSE_OPS_H

#include <stdio.h>
#include <stdlib.h>
#include <fuse.h>
#include <stdint.h>

#include "./helpers.h"
#include "./main.h"

int do_getattr(const char *path, struct stat *st);
int do_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int do_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi);
int do_write( const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info );
int do_open(const char *path, struct fuse_file_info *fi);
int do_create (const char *path, mode_t mode, struct fuse_file_info *fi);
int do_truncate (const char *path, off_t offset);
int	do_unlink(const char *path);
int do_release(const char *path, struct fuse_file_info *fi);

struct fuse_operations operations = {
    .getattr = do_getattr,
    .readdir = do_readdir,
    .read = do_read,
    .write = do_write,
    .open = do_open,
	.create = do_create,
	.truncate = do_truncate,
	.unlink = do_unlink,
	.release = do_release,
};

#endif

#define FUSE_USE_VERSION 31

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libclipboard.h>

clipboard_c *clip;

clipboard_opts clipfs_opts;

const char *clipfs_path = "/clip";

static void *clipfs_init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
	cfg->kernel_cache = 1;

	clip = clipboard_new(&clipfs_opts);

	return NULL;
}

static void clipfs_destroy(void *userdata)
{
	clipboard_free(clip);
}

static int clipfs_getattr(const char *path, struct stat *stbuf,
			  struct fuse_file_info *fi)
{
	int res = 0;
	int length;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, clipfs_path) == 0) {
		stbuf->st_mode = S_IFREG | 0644;
		stbuf->st_nlink = 1;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();

		char *cliptext =
			clipboard_text_ex(clip, &length, LCB_CLIPBOARD);
		if (!cliptext)
			return -EIO;
		free(cliptext);

		stbuf->st_size = length;
	} else
		res = -ENOENT;

	return res;
}

static int clipfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			  off_t offset, struct fuse_file_info *fi,
			  enum fuse_readdir_flags flags)
{
	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);
	filler(buf, clipfs_path + 1, NULL, 0, 0);

	return 0;
}

static int clipfs_open(const char *path, struct fuse_file_info *fi)
{
	if (strcmp(path, clipfs_path) != 0)
		return -ENOENT;

	return 0;
}

static int clipfs_read(const char *path, char *buf, size_t size, off_t offset,
		       struct fuse_file_info *fi)
{
	char *cliptext;
	int clipfs_size;

	if (strcmp(path, clipfs_path) != 0)
		return -ENOENT;

	cliptext = clipboard_text_ex(clip, &clipfs_size, LCB_CLIPBOARD);
	if (!cliptext)
		return -EIO;

	if (offset < clipfs_size) {
		if (offset + size > clipfs_size)
			size = clipfs_size - offset;
		memcpy(buf, cliptext + offset, size);
	} else
		size = 0;

	free(cliptext);
	return size;
}

static int clipfs_write(const char *path, const char *buf, size_t size,
			off_t offset, struct fuse_file_info *fi)
{
	if (strcmp(path, clipfs_path) != 0)
		return -ENOENT;

	bool rv = clipboard_set_text_ex(clip, buf, size, LCB_CLIPBOARD);
	if (!rv)
		return -EIO;

	return size;
}

static const struct fuse_operations clipfs_oper = {
	.init = clipfs_init,
	.destroy = clipfs_destroy,
	.getattr = clipfs_getattr,
	.readdir = clipfs_readdir,
	.open = clipfs_open,
	.read = clipfs_read,
	.write = clipfs_write,
};

int main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &clipfs_oper, NULL);
}
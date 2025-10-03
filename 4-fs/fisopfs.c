#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include "fs_utils.h"

#define DEFAULT_FILE_DISK "persistence_file.fisopfs"

char *filedisk = DEFAULT_FILE_DISK;
char *fisop_file_contenidos;

super_block_t superblock;


// =================== Inicialización ==========================


void
create_root()
{
	printf("[debug] create_root\n");

	memset(superblock.inodes, 0, sizeof(superblock.inodes));
	memset(superblock.status_inodes, 0, sizeof(superblock.status_inodes));

	inode_t root = {
		.file_size = MAX_CONTENT,
		.type = FILE_TYPE_DIRECTORY,
		.mode = __S_IFDIR | 0755,
		.nlink = 2,
		.uid = getuid(),
		.gid = getgid(),
		.ctime = time(NULL),
		.mtime = time(NULL),
		.atime = time(NULL),
	};

	strcpy(root.name, "Root");
	strcpy(root.path, ROOT);
	strcpy(root.file_parent, NO_PARENT);
	memset(root.file_content, 0, sizeof(root.file_content));

	superblock.inodes[0] = root;
	superblock.status_inodes[0] = OCCUPIED;
}


static void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] fisopfs_init\n");

	super_block_t *fs = read_persistence(filedisk);

	if (fs == NULL) {
		printf("[debug] No persistence found, creating new Root\n");
		create_root();
		write_persistence(filedisk, &superblock);
	} else {
		superblock = *fs;
		free(fs);
	}

	return NULL;
}


// ================= Creación y eliminación ===================


int
set_inode_in_superblock(inode_t *inode)
{
	for (int i = 0; i < MAX_INODES; i++) {
		if (superblock.status_inodes[i] == OCCUPIED &&
		    strcmp(superblock.inodes[i].name, inode->name) == 0 &&
		    strcmp(superblock.inodes[i].file_parent,
		           inode->file_parent) == 0) {
			superblock.inodes[i].nlink++;
			return i;
		}
	}

	int free_idx = search_free_inode(superblock);
	if (free_idx < 0)
		return -ENOSPC;

	superblock.inodes[free_idx] = *inode;
	superblock.status_inodes[free_idx] = OCCUPIED;
	return free_idx;
}


static int
new_inode(const char *path, mode_t mode, int type)
{
	printf("[debug] create_inode -> path: %s - mode: %d - type: %d\n",
	       path,
	       mode,
	       type);

	inode_t new_node = {
		.file_size = 0,
		.type = type,
		.nlink = (type == FILE_TYPE_REGULAR) ? 1 : 2,
		.uid = getuid(),
		.gid = getgid(),
		.mode = (type == FILE_TYPE_REGULAR) ? (__S_IFREG | 0644)
		                                    : (__S_IFDIR | 0755),
		.atime = time(NULL),
		.mtime = time(NULL),
		.ctime = time(NULL),
	};

	char padre[MAX_PATH_LEN];
	get_parent_dir(path, padre);

	char *filename = get_file_name(path);
	if (!filename)
		return -ENOMEM;

	strncpy(new_node.name, filename, MAX_PATH_LEN - 1);
	new_node.name[MAX_PATH_LEN - 1] = '\0';
	free(filename);

	strncpy(new_node.path, path, MAX_PATH_LEN - 1);
	new_node.path[MAX_PATH_LEN - 1] = '\0';

	strncpy(new_node.file_parent, padre, MAX_PATH_LEN - 1);
	new_node.file_parent[MAX_PATH_LEN - 1] = '\0';

	memset(new_node.file_content, 0, sizeof(new_node.file_content));

	int free_idx = set_inode_in_superblock(&new_node);
	return (free_idx < 0) ? -ENOSPC : 0;
}


static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *file_info)
{
	printf("[debug] fisopfs_create -> path: %s - mode: %d\n", path, mode);
	return new_inode(path, mode, FILE_TYPE_REGULAR);
}


static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("[debug] fisopfs_mkdir -> path: %s - mode: %d\n", path, mode);
	return new_inode(path, mode, FILE_TYPE_DIRECTORY);
}


static int
fisopfs_rmdir(const char *path)
{
	printf("[debug] fisopfs_rmdir -> path: %s\n", path);

	int index = get_inode_index(superblock, path);
	if (index < 0)
		return -ENOENT;

	inode_t inode = superblock.inodes[index];
	if (inode.type != FILE_TYPE_DIRECTORY)
		return -ENOTDIR;

	for (int i = 0; i < MAX_INODES; i++) {
		if (superblock.status_inodes[i] == OCCUPIED &&
		    strcmp(superblock.inodes[i].file_parent, path) == 0) {
			return -ENOTEMPTY;
		}
	}

	superblock.status_inodes[index] = FREE;
	return 0;
}


// ================== Operaciones FUSE =======================


static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);
	memset(st, 0, sizeof(struct stat));

	int idx = search_inode_by_path(superblock, path);
	if (idx == -1) {
		return -ENOENT;
	}

	inode_t inode = superblock.inodes[idx];
	st->st_uid = inode.uid;
	st->st_gid = inode.gid;
	st->st_nlink = inode.nlink;
	st->st_size = inode.file_size;
	st->st_ctime = inode.ctime;
	st->st_mtime = inode.mtime;
	st->st_atime = inode.atime;
	st->st_mode = inode.mode;

	return 0;
}


static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s\n", path);

	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	for (int i = 0; i < MAX_INODES; i++) {
		if (superblock.status_inodes[i] == OCCUPIED &&
		    strcmp(superblock.inodes[i].file_parent, path) == 0) {
			filler(buffer, superblock.inodes[i].name, NULL, 0);
		}
	}

	return 0;
}


static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	int idx = search_inode_by_path(superblock, path);
	if (idx == -1)
		return -ENOENT;

	inode_t inode = superblock.inodes[idx];

	if (offset >= inode.file_size)
		return 0;

	if (offset + size > inode.file_size)
		size = inode.file_size - offset;

	memcpy(buffer, inode.file_content + offset, size);

	return size;
}


int
fisopfs_write(const char *path,
              const char *buffer,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)

{
	printf("[debug] fisopfs_write - path: %s - size: %lu - offset: %lu\n",
	       path,
	       size,
	       offset);

	int index = get_inode_index(superblock, path);
	if (index == -1) {
		printf("No index\n");
		return -ENOENT;
	}

	inode_t *inode = &superblock.inodes[index];
	struct fuse_context *context = fuse_get_context();
	if (inode->uid != context->uid) {
		printf("Error fisopfs_write\n");
		return -ENOENT;
	}

	if (inode->uid != context->uid) {
		printf("Error: permission denied for writing\n");
		return -EACCES;
	}

	if (offset > MAX_CONTENT) {
		return -EFBIG;
	}

	strncpy(inode->file_content + offset, buffer, size);
	inode->file_size = offset + size;
	inode->file_content[inode->file_size] = '\0';

	for (int i = 0; i < MAX_INODES; i++) {
		if (superblock.status_inodes[i] == OCCUPIED &&
		    superblock.inodes[i].nlink > 1 &&
		    strcmp(superblock.inodes[i].file_content,
		           inode->file_content) != 0) {
			strncpy(superblock.inodes[i].file_content,
			        inode->file_content,
			        MAX_CONTENT);
			superblock.inodes[i].file_size = inode->file_size;
		}
	}

	return (int) size;
}


void
fisopfs_destroy(void *private_data)
{
	printf("[debug] fisopfs_destroy - destroying root\n");

	if (write_persistence(filedisk, &superblock) == -1) {
		printf("Error writing persistance\n");
		exit(1);
	}
}


int
fisopfs_unlink(const char *path)
{
	printf("[debug] fisopfs_unlink - path: %s\n", path);

	int index = get_inode_index(superblock, path);
	if (index == -1)
		return -ENOENT;

	inode_t *inode = &superblock.inodes[index];

	if (inode->type != FILE_TYPE_REGULAR && !S_ISLNK(inode->mode))
		return -EISDIR;

	superblock.status_inodes[index] = FREE;
	return 0;
}


int
fisopfs_utimens(const char *path, const struct timespec tv[2])
{
	printf("[debug] fisopfs_utimens - path: %s\n", path);
	int index = get_inode_index(superblock, path);
	if (index == -1) {
		printf("No index\n");
		return -ENOENT;
	}

	superblock.inodes[index].atime = tv[0].tv_sec;
	superblock.inodes[index].mtime = tv[1].tv_sec;

	return 0;
}


int
fisopfs_truncate(const char *path, long offset)
{
	printf("[debug] fisopfs_truncate - path: %s - offset: %ld\n", path, offset);

	int index = get_inode_index(superblock, path);
	if (index < 0)
		return -ENOENT;

	inode_t *inode = &superblock.inodes[index];

	if (inode->type != FILE_TYPE_REGULAR)
		return -EISDIR;

	if (offset > MAX_CONTENT * sizeof(char))
		return -EISDIR;

	if (offset < inode->file_size) {
		memset(inode->file_content + offset, 0, inode->file_size - offset);
	} else if (offset > inode->file_size) {
		memset(inode->file_content + inode->file_size,
		       0,
		       offset - inode->file_size);
	}

	inode->file_size = offset;

	return 0;
}


int
fisopfs_symlink(const char *oldpath, const char *newpath)
{
	printf("[debug] fisopfs_symlink - oldpath: %s - newpath: %s\n",
	       oldpath,
	       newpath);

	int index_new = search_free_inode(superblock);
	if (index_new < 0)
		return -ENOSPC;

	char parent[MAX_PATH_LEN];
	get_parent_dir(newpath, parent);

	char *filename = get_file_name(newpath);
	if (!filename)
		return -ENOMEM;

	inode_t symlink_inode = {
		.file_size = strlen(oldpath),
		.type = FILE_TYPE_REGULAR,
		.mode = __S_IFLNK | 0777,
		.nlink = 1,
		.uid = getuid(),
		.gid = getgid(),
		.ctime = time(NULL),
		.mtime = time(NULL),
		.atime = time(NULL),
	};

	strncpy(symlink_inode.name, filename, MAX_PATH_LEN - 1);
	symlink_inode.name[MAX_PATH_LEN - 1] = '\0';
	free(filename);

	strncpy(symlink_inode.path, newpath, MAX_PATH_LEN - 1);
	symlink_inode.path[MAX_PATH_LEN - 1] = '\0';

	strncpy(symlink_inode.file_parent, parent, MAX_PATH_LEN - 1);
	symlink_inode.file_parent[MAX_PATH_LEN - 1] = '\0';

	memset(symlink_inode.file_content, 0, sizeof(symlink_inode.file_content));
	strncpy(symlink_inode.file_content, oldpath, MAX_CONTENT - 1);
	symlink_inode.file_content[MAX_CONTENT - 1] = '\0';

	superblock.inodes[index_new] = symlink_inode;
	superblock.status_inodes[index_new] = OCCUPIED;

	return 0;
}


int
fisopfs_readlink(const char *path, char *buffer, size_t size)
{
	printf("[debug] fisopfs_readlink - path: %s\n", path);

	int idx = search_inode_by_path(superblock, path);
	if (idx == -1)
		return -ENOENT;

	inode_t inode = superblock.inodes[idx];

	if (!S_ISLNK(inode.mode))
		return -EINVAL;

	size_t oldpath_len = strlen(inode.file_content);
	if (oldpath_len >= size)
		oldpath_len = size - 1;

	memcpy(buffer, inode.file_content, oldpath_len);
	buffer[oldpath_len] = '\0';

	return 0;
}


int
fisopfs_link(const char *oldpath, const char *newpath)
{
	printf("[debug] fisopfs_link - oldpath: %s - newpath: %s\n",
	       oldpath,
	       newpath);

	int index_old = get_inode_index(superblock, (const char *) oldpath);
	if (index_old < 0)
		return -ENOENT;

	inode_t *old_inode = &superblock.inodes[index_old];

	if (old_inode->type != FILE_TYPE_REGULAR)
		return -EISDIR;

	int index_new = search_free_inode(superblock);
	if (index_new < 0)
		return -ENOSPC;

	inode_t new_inode = *old_inode;

	char parent[MAX_PATH_LEN];
	get_parent_dir(newpath, parent);

	char *filename = get_file_name(newpath);
	if (!filename)
		return -ENOMEM;

	memcpy(new_inode.name, filename, MAX_PATH_LEN - 1);
	new_inode.name[MAX_PATH_LEN - 1] = '\0';
	free(filename);

	memcpy(new_inode.path, newpath, MAX_PATH_LEN - 1);
	new_inode.path[MAX_PATH_LEN - 1] = '\0';

	memcpy(new_inode.file_parent, parent, MAX_PATH_LEN - 1);
	new_inode.file_parent[MAX_PATH_LEN - 1] = '\0';

	new_inode.nlink = old_inode->nlink + 1;

	superblock.inodes[index_new] = new_inode;
	superblock.status_inodes[index_new] = OCCUPIED;

	old_inode->nlink++;

	return 0;
}


// ===================== Main y FUSE ops ========================


static struct fuse_operations operations = { .getattr = fisopfs_getattr,
	                                     .readdir = fisopfs_readdir,
	                                     .read = fisopfs_read,

	                                     // new operations
	                                     .write = fisopfs_write,
	                                     .init = fisopfs_init,
	                                     .create = fisopfs_create,
	                                     .mkdir = fisopfs_mkdir,
	                                     .rmdir = fisopfs_rmdir,
	                                     .unlink = fisopfs_unlink,
	                                     .utimens = fisopfs_utimens,
	                                     .truncate = fisopfs_truncate,
	                                     .link = fisopfs_link,
	                                     .symlink = fisopfs_symlink,
	                                     .readlink = fisopfs_readlink,
	                                     .destroy = fisopfs_destroy };

int
main(int argc, char *argv[])
{
	for (int i = 1; i < argc - 1; i++) {
		if (strcmp(argv[i], "--filedisk") == 0) {
			filedisk = argv[i + 1];

			// We remove the argument so that fuse doesn't use our
			// argument or name as folder.
			// Equivalent to a pop.
			for (int j = i; j < argc - 1; j++) {
				argv[j] = argv[j + 2];
			}

			argc = argc - 2;
			break;
		}
	}

	return fuse_main(argc, argv, &operations, NULL);
}
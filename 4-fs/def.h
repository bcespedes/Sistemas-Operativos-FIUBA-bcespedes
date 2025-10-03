#ifndef DEF_H
#define DEF_H

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define MAX_PATH_LEN 128
#define MAX_NAME_LEN 128
#define MAX_DIR_SIZE 1024
#define MAX_INODES 64
#define MAX_CONTENT 100

#define OCCUPIED 1
#define FREE 0

#define NO_PARENT ""
#define ROOT "/"

#define FILE_TYPE_REGULAR 1
#define FILE_TYPE_DIRECTORY 2
#define FILE_TYPE_SYMLINK 3

typedef struct inode {
	char name[MAX_NAME_LEN];  // File or directory name
	int file_size;
	int type;
	char path[MAX_PATH_LEN];
	char file_content[MAX_CONTENT];
	char file_parent[MAX_PATH_LEN];
	unsigned long int nlink;
	uid_t uid;  // User
	uid_t gid;  // Group
	int mode;

	time_t ctime;  // Created
	time_t mtime;  // Modificated
	time_t atime;  // Accessed

} inode_t;

typedef struct super_block {
	inode_t inodes[MAX_INODES];
	int status_inodes[MAX_INODES];
} super_block_t;

#endif
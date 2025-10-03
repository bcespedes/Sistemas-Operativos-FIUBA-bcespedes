#include <stdbool.h>
#include "stdio.h"
#include "../../def.h"
#include "../../fs_utils.h"


bool equals(super_block_t *s1, super_block_t *s2);


int
main()
{
	char *path = "test009.fisopfs";
	super_block_t expected = {
		.inodes = { [0 ... 14] = { .name = "pepe.txt",
		                           .file_size = 12,
		                           .type = FILE_TYPE_REGULAR,
		                           .path = "/files",
		                           .file_content = "Hola mundo!",
		                           .file_parent = "/",
		                           .nlink = 0,
		                           .uid = 1000,
		                           .gid = 23456,
		                           .mode = 0,
		                           .ctime = 1234567890,
		                           .mtime = 1234567890,
		                           .atime = 1234567890 },
		            [15 ... 28] = {},
		            [29 ... 62] = { .name = "aaaa",
		                            .file_size = 4096,
		                            .type = FILE_TYPE_DIRECTORY,
		                            .path = "/",
		                            .file_content = "",
		                            .file_parent = "",
		                            .nlink = 0,
		                            .uid = 1000,
		                            .gid = 23456,
		                            .mode = 0,
		                            .ctime = 1234567890,
		                            .mtime = 1234567890,
		                            .atime = 1234567890 },
		            [63] = {} },
		.status_inodes = { [0 ... 14] = 1, [15 ... 28] = 0, [29 ... 62] = 1, [63] = 0 }
	};


	int status = write_persistence(path, &expected);

	if (status != 0) {
		printf("PERSISTENCE WRITE ERROR: STATUS %d\n", status);
	} else {
		printf("PERSISTENCE WRITE PASS\n");
	}


	super_block_t *actual = read_persistence(path);

	if (actual == NULL) {
		printf("PERSISTENCE READ ERROR: NULL POINTER\n");
	} else if (!equals(&expected, actual)) {
		printf("PERSISTENCE READ ERROR: NOT EQUALS\n");
	} else {
		printf("PERSISTENCE READ PASS\n");
	}


	return 0;
}


bool
equals(super_block_t *s1, super_block_t *s2)
{
	for (size_t i = 0; i < MAX_INODES; i++) {
		if (s1->status_inodes[i] != s2->status_inodes[i]) {
			return false;
		}
	}

	for (size_t i = 0; i < MAX_INODES; i++) {
		if (strcmp(s1->inodes[i].name, s2->inodes[i].name) != 0) {
			return false;
		}
		if (s1->inodes[i].file_size != s2->inodes[i].file_size) {
			return false;
		}
		if (s1->inodes[i].type != s2->inodes[i].type) {
			return false;
		}
		if (strcmp(s1->inodes[i].path, s2->inodes[i].path) != 0) {
			return false;
		}
		if (strcmp(s1->inodes[i].file_content,
		           s2->inodes[i].file_content) != 0) {
			return false;
		}
		if (strcmp(s1->inodes[i].file_parent,
		           s2->inodes[i].file_parent) != 0) {
			return false;
		}
		if (s1->inodes[i].nlink != s2->inodes[i].nlink) {
			return false;
		}
		if (s1->inodes[i].uid != s2->inodes[i].uid) {
			return false;
		}
		if (s1->inodes[i].gid != s2->inodes[i].gid) {
			return false;
		}
		if (s1->inodes[i].mode != s2->inodes[i].mode) {
			return false;
		}
		if (s1->inodes[i].ctime != s2->inodes[i].ctime) {
			return false;
		}
		if (s1->inodes[i].mtime != s2->inodes[i].mtime) {
			return false;
		}
		if (s1->inodes[i].atime != s2->inodes[i].atime) {
			return false;
		}
	}

	return true;
}
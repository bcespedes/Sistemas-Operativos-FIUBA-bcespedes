#include <stddef.h>
#include "fs_utils.h"

void
get_parent_dir(const char *path, char *output)
{
	strcpy(output, path);
	char *last = strrchr(output, '/');
	if (last && last != output) {
		*last = '\0';
	} else {
		strcpy(output, ROOT);
	}
}

char *
get_file_name(const char *path)
{
	char *last_slash = strrchr(path, '/');
	const char *filename_start = (last_slash != NULL) ? last_slash + 1 : path;
	char *dest = malloc(1024);
	if (!dest)
		return NULL;
	strcpy(dest, filename_start);
	return dest;
}

int
search_inode_by_path(super_block_t superblock, const char *path)
{
	for (int i = 0; i < MAX_INODES; i++) {
		if (superblock.status_inodes[i] == OCCUPIED &&
		    strcmp(superblock.inodes[i].path, path) == 0) {
			return i;
		}
	}
	return -1;
}

int
search_free_inode(super_block_t superblock)
{
	for (int i = 0; i < MAX_INODES; i++) {
		if (superblock.status_inodes[i] == FREE) {
			return i;
		}
	}
	return -1;
}

int
get_inode_index(super_block_t superblock, const char *path)
{
	if (strcmp(path, ROOT) == 0)
		return 0;

	for (int i = 0; i < MAX_INODES; i++) {
		if (superblock.status_inodes[i] == OCCUPIED &&
		    strcmp(superblock.inodes[i].path, path) == 0) {
			return i;
		}
	}
	return -1;
}

int
write_persistence(char *path, super_block_t *superblock)
{
	if (!path || !superblock)
		return -1;

	FILE *fs = fopen(path, "wb");
	if (fs == NULL) {
		perror("write_persistence error fopen");
		return -1;
	}

	if (fwrite(superblock, sizeof(super_block_t), 1, fs) != 1) {
		perror("write_persistence error fwrite");
		fclose(fs);
		return -1;
	}

	fflush(fs);
	fclose(fs);
	return 0;
}


super_block_t *
read_persistence(char *path)
{
	if (!path)
		return NULL;

	FILE *file = fopen(path, "rb");

	if (file == NULL) {
		printf("[debug] read_persistance - No Root such\n");
		return NULL;
	}

	super_block_t *superblock = calloc(1, sizeof(super_block_t));

	if (superblock == NULL) {
		fclose(file);
		return NULL;
	}

	size_t r = fread(superblock, sizeof(super_block_t), 1, file);

	fclose(file);

	if (r != 1) {
		free(superblock);
		return NULL;
	}

	return superblock;
}
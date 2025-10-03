#ifndef FSUTILS_H
#define FSUTILS_H

#include "def.h"

void get_parent_dir(const char *path, char *output);

char *get_file_name(const char *path);

int search_inode_by_path(super_block_t superblock, const char *path);

int search_free_inode(super_block_t superblock);

int get_inode_index(super_block_t superblock, const char *path);

int write_persistence(char *path, super_block_t *superblock);

super_block_t *read_persistence(char *path);


#endif
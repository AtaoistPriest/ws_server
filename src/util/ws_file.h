#ifndef TOOLS_FILE_H
#define TOOLS_FILE_H

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "./ws_common.h"

/*
 * function defined
 * */
int write_file(char *filename, char *data, boolean is_append);
int is_dir_exist(char *filename);
int is_file_exist(char *filename);
int create_dir(char *filename);
int delete_file(char *filename);

#endif

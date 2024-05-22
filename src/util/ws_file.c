#include "ws_file.h"

int write_file(char *filename, char *data, boolean is_append)
{
	if ( filename == NULL || data == NULL || strlen(filename) == 0 )
	{
		return FAILURE;
	}
	FILE *file = NULL;
	if ( is_append )
	{
    	file = fopen(filename, "a");
	}
	else
	{
    	file = fopen(filename, "w");
	}
    if ( !file )
    {
        return FAILURE;
    }
    fprintf(file, "%s\n", data);
    fclose(file);
    return SUCCESS;
}

int is_dir_exist(char *filename)
{
    struct stat st = {0};
    if (stat(filename, &st) == 0 && S_ISDIR(st.st_mode)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int is_file_exist(char *filename)
{
	if ( filename == NULL )
	{
		return FALSE;
	}
    FILE *file = fopen(filename, "r");
    if ( !file )
    {
        return FALSE;
    }
    fclose(file);
    return TRUE;
}



int create_dir(char *filename)
{
	if ( filename == NULL )
	{
		return FAILURE;
	}

    int res = FAILURE;
    if( access(filename, 0) != 0 )
	{
        res = mkdir(filename, 0755);
	}

    return res;
}

int delete_file(char *filename)
{
	if ( filename == NULL )
	{
		return FAILURE;
	}
    int res;
    if( access(filename, 0) == 0 )
	{
        res = remove(filename);
	}
    return res;
}

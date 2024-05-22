#include "./ws_string.h"

// get str line from src_str branch by "\r\n"
int nextline(char *src_str, char *dst_str, int start_idx )
{
	if ( src_str == NULL || dst_str == NULL || start_idx < 0 )
	{
		return ERROR;
	}

	int str_len = strlen(src_str);
	if ( start_idx >= str_len )
	{
		return ERROR;
	}

	int line_len = 0;
	for ( ; start_idx < str_len - 1; start_idx++ )
	{
		if ( src_str[start_idx] == '\r' && src_str[start_idx + 1] == '\n'  )
		{
			dst_str[line_len] = '\0';
			break;
		}
		dst_str[line_len] = src_str[start_idx];
		line_len++;
	}

	if ( start_idx >= str_len - 1 )
	{
		return ERROR;
	}

	return line_len;
}

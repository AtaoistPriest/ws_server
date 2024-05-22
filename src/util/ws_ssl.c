// =====================================================================================
//  Copyright (C) 2024 by 冯飞宇. All rights reserved
//  文 件 名:  src/ws_ssl.c
//  作    者:  冯飞宇
//  创建时间:  2024年04月16日
//  描    述: this is a c/c++ file 
// =====================================================================================
#include "./ws_ssl.h"

void ws_sha1(unsigned char *sec_data, int data_len, unsigned char *sec_accept)
{
	SHA1(sec_data, data_len, sec_accept);
}

int ws_base64_encode(char *in_str, int in_len, char *out_str)
{
	BIO *b64, *bio;    
	BUF_MEM *bptr = NULL;    
	size_t size = 0;    

	if (in_str == NULL || out_str == NULL)        
		return -1;    

	b64 = BIO_new(BIO_f_base64());    
	bio = BIO_new(BIO_s_mem());    
	bio = BIO_push(b64, bio);
	
	BIO_write(bio, in_str, in_len);    
	BIO_flush(bio);    

	BIO_get_mem_ptr(bio, &bptr);    
	memcpy(out_str, bptr->data, bptr->length);    
	out_str[bptr->length-1] = '\0';    
	size = bptr->length;    
	BIO_free_all(bio);    
	return size;
}

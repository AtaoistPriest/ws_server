// =====================================================================================
//  Copyright (C) 2024 by 冯飞宇. All rights reserved
//  文 件 名:  src/ws_ssl.h
//  作    者:  冯飞宇
//  创建时间:  2024年04月16日
//  描    述: this is a c/c++ file 
// =====================================================================================
#ifndef WS_SRC_SSL_H
#define WS_SRC_SSL_H

#include <string.h>
#include <openssl/sha.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

extern void ws_sha1(unsigned char *sec_data, int data_len, unsigned char *sec_accept);
extern int ws_base64_encode(char *in_str, int in_len, char *out_str);

#endif

#ifndef SRC_WS_LOGGER_H
#define SRC_WS_LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/syscall.h>

#include "./ws_common.h"
#include "./ws_os.h"
#include "./ws_file.h"

/*
 * 常量定义
 * */
#define MAX_FILE_PATH_LEN 256
/*
日志级别定义，数值越小，级别越高
*/
#define LOGGER_FATAL_LEVEL			1 	//致命错误，不可恢复
#define LOGGER_ERROR_LEVEL			2	//一般错误，可恢复
#define LOGGER_WARN_LEVEL			3	//警告
#define LOGGER_INFO_LEVEL			4	//信息
#define LOGGER_DEBUG_LEVEL			5	//调试

/*
当前日志级别配置
*/
#define LOGGER_CFG_LEVEL LOGGER_DEBUG_LEVEL
/*
 * 日志文件前缀
 * */
#define LOG_NAME_PREFIX "wslog"
// 日志文件数量
#define DEFAULT_LOG_NUM_SET 5
// 日志文件大小 
#define DEFAULT_LOG_SIZE_SET 5
// 日志每行列数限制
#define LIMIT_LOG_LENGTH 4096

typedef struct _logger_cfg
{
    int level;
    // MB
    long log_size_set;
    int file_num_set;
    char *log_dir;

    long log_size_cur;
    long file_num_cur;
    char *log_name_cur;
    FILE *file_cur;
}logger_cfg;

void logger_init(char *log_path);
void logger_destroy();

void logger_set_level(int level);
void logger_set_log_size(long log_size);
void logger_set_file_num(int file_num);

void _log_debug(const char *file, const char *func, const int line, const char *format, ...);
void _log_info(const char *file, const char *func, const int line, const char *format, ...);
void _log_warn(const char *file, const char *func, const int line, const char *format, ...);
void _log_error(const char *file, const char *func, const int line, const char *format, ...);
void _log_fatal(const char *file, const char *func, const int line, const char *format, ...);

#endif 

#ifndef SRC_LOGGER_H
#define SRC_LOGGER_H

#include "./ws_logger.h"

void logger_init(char *log_path);
void logger_destroy();

void logger_set_level(int level);
void logger_set_log_size(long log_size);
void logger_set_file_num(int file_num);

#define logger_level_printf(level, arg...)\
    do {\
        if (level <= LOGGER_CFG_LEVEL) {\
            if (level == LOGGER_FATAL_LEVEL) {\
                _log_fatal(__FILE__, __func__, __LINE__, ##arg);\
            }\
            if (level == LOGGER_ERROR_LEVEL) {\
                _log_error(__FILE__, __func__, __LINE__, ##arg);\
            }\
            if (level == LOGGER_WARN_LEVEL) {\
                _log_warn(__FILE__, __func__, __LINE__, ##arg);\
            }\
            if (level == LOGGER_INFO_LEVEL) {\
                _log_info(__FILE__, __func__, __LINE__, ##arg);\
            }\
            if (level == LOGGER_DEBUG_LEVEL) {\
                _log_debug(__FILE__, __func__, __LINE__, ##arg);\
            }\
        }\
    } while(0)

#define log_fatal(arg...)	logger_level_printf(LOGGER_FATAL_LEVEL, ##arg)
#define log_error(arg...)	logger_level_printf(LOGGER_ERROR_LEVEL, ##arg)
#define log_warn(arg...)	logger_level_printf(LOGGER_WARN_LEVEL, ##arg)
#define log_info(arg...)	logger_level_printf(LOGGER_INFO_LEVEL, ##arg)
#define log_debug(arg...)	logger_level_printf(LOGGER_DEBUG_LEVEL, ##arg)

#endif 

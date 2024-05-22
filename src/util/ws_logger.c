#include "./ws_logger.h"

static logger_cfg *log_config = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int split_log_file()
{
    int res = FAILURE;
    // check file number
    DIR *dir = opendir(log_config->log_dir);
    if ( dir != NULL )
    {
        char file_date_min[MAX_FILE_PATH_LEN] = "~~";
        boolean is_find = FALSE;
        struct dirent *entry;
        int file_num = 0;
        while ( (entry = readdir(dir)) != NULL )
        {
            if ( strstr(entry->d_name, LOG_NAME_PREFIX) != NULL )
            {
                if ( strcmp(entry->d_name, file_date_min) < 0 )
                {
                    strcpy(file_date_min, entry->d_name);
                    is_find = TRUE;
                }
                file_num++;
            }
        }
        // check current log file number 
        if ( file_num >= log_config->file_num_set && is_find )
        {
            // if delete oldest log file successfully, create new log file
            char file_delete_str[MAX_FILE_PATH_LEN] = {0};
            sprintf(file_delete_str, "%s%s", log_config->log_dir, file_date_min);
            if ( !delete_file(file_delete_str) )
            {
                if ( log_config->file_cur != NULL )
                {
                    fclose(log_config->file_cur);
                }
                memset(log_config->log_name_cur, 0, MAX_FILE_PATH_LEN);
                sprintf(log_config->log_name_cur, "%s%s_%d.log", log_config->log_dir, LOG_NAME_PREFIX, get_ts());
                log_config->file_cur = fopen(log_config->log_name_cur, "a");
                log_config->log_size_cur = 0;
                res = SUCCESS;
            }
            else
            {
                printf("delete log file: %s failed\n", file_delete_str);
            }
        }
        else
        {
            memset(log_config->log_name_cur, 0, MAX_FILE_PATH_LEN);
            sprintf(log_config->log_name_cur, "%s%s_%d.log", log_config->log_dir, LOG_NAME_PREFIX, get_ts());
            if ( log_config->file_cur != NULL )
            {
                fclose(log_config->file_cur);
            }
            log_config->file_cur = fopen(log_config->log_name_cur, "a");
            log_config->log_size_cur = 0;
            res = SUCCESS;
        }
    }
    closedir(dir);
    return res;
}

static void check_log_size()
{
    if ( log_config == NULL )
    {
        return;
    }
    long file_size = log_config->log_size_set * 1024 * 1024;
    if ( log_config->log_size_cur >= file_size )
    {
        split_log_file();
    }
}

void logger_init(char *log_path) 
{
    if ( log_config != NULL )
    {
        return;
    }
    pthread_mutex_lock(&mutex);
    log_config = (logger_cfg *)malloc(sizeof(logger_cfg));

    if ( log_config == NULL )
    {
        pthread_mutex_unlock(&mutex);
        exit(1);
    }

    log_config->level = LOGGER_INFO_LEVEL;
    log_config->log_size_set = DEFAULT_LOG_SIZE_SET;
    log_config->file_num_set = DEFAULT_LOG_NUM_SET;
    log_config->log_dir = (char *)malloc(sizeof(char) * MAX_FILE_PATH_LEN);
    log_config->log_name_cur = (char *)malloc(sizeof(char) * MAX_FILE_PATH_LEN);
    log_config->log_size_cur = 0;
    log_config->file_cur = NULL;

    strcpy(log_config->log_dir, log_path);
    strcat(log_config->log_dir, "/");

    if ( !is_dir_exist(log_config->log_dir) )
    {
        create_dir(log_config->log_dir);
    }

    // reuse
    DIR *dir = opendir(log_config->log_dir);
    char file_date_max[MAX_FILE_PATH_LEN] = "  ";
    boolean log_exist = FALSE;
    if ( dir != NULL )
    {
        struct dirent *entry;
        int file_num = 0;
        while ( (entry = readdir(dir)) != NULL )
        {
            if ( strstr(entry->d_name, LOG_NAME_PREFIX) != NULL )
            {
                if ( strcmp(entry->d_name, file_date_max) > 0 )
                {
                    log_exist = TRUE;
                    strcpy(file_date_max, entry->d_name);
                }
                file_num++;
            }
        }
    }
    if ( log_exist )
    {
        sprintf(log_config->log_name_cur, "%s%s", log_config->log_dir, file_date_max);
    }
    else
    {
        sprintf(log_config->log_name_cur, "%s%s_%d.log", log_config->log_dir, LOG_NAME_PREFIX, get_ts());
    }
    
    log_config->file_cur = fopen(log_config->log_name_cur, "a");

    if ( log_config->file_cur == NULL )
    {
        pthread_mutex_unlock(&mutex);
        exit(1);
    }

    pthread_mutex_unlock(&mutex);
}

void logger_destroy()
{
    pthread_mutex_lock(&mutex);
    if ( log_config != NULL )
    {
        if ( log_config->log_dir != NULL )
        {
            free(log_config->log_dir);
            log_config->log_dir = NULL;
        }
        if ( log_config->log_name_cur != NULL )
        {
            free(log_config->log_name_cur);
            log_config->log_name_cur = NULL;
        }
        if ( log_config->file_cur != NULL )
        {
            fclose(log_config->file_cur);
            log_config->file_cur = NULL;
        }
        free(log_config);
        log_config = NULL;
    }
    pthread_mutex_unlock(&mutex);
}

void logger_set_level(int level)
{
    pthread_mutex_lock(&mutex);

	if ( log_config != NULL )
	{
    	log_config->level = level;
	}

    pthread_mutex_unlock(&mutex);
}

void logger_set_log_size(long log_size)
{
    pthread_mutex_lock(&mutex);

	if ( log_config != NULL )
	{
    	log_config->log_size_set = log_size;
	}

    pthread_mutex_unlock(&mutex);
}

void logger_set_file_num(int file_num)
{
    pthread_mutex_lock(&mutex);

	if ( log_config != NULL )
	{
    	log_config->file_num_set = file_num;
	}

    pthread_mutex_unlock(&mutex);
}


void _log_fatal(const char *file, const char *func, const int line, const char *format, ...)
{
    if ( log_config == NULL )
    {
        return;
    }

    if ( log_config->level < LOGGER_FATAL_LEVEL)
    {
        return;
    }

    va_list args1;
    va_start(args1, format);

    int pid = get_process_id(), tid = get_thread_id();

    char log_str[LIMIT_LOG_LENGTH], log_content[LIMIT_LOG_LENGTH - 200];

	vsprintf(log_content, format, args1);   
    sprintf(log_str, "%s %s [%d][%d] [%s %s:%d] [FATAL] %s\n", __DATE__, __TIME__, pid, tid, file, func, line, log_content);

    pthread_mutex_lock(&mutex);
    fprintf(log_config->file_cur, "%s", log_str);
    fflush(log_config->file_cur);

    log_config->log_size_cur += strlen(log_str);
    check_log_size();
    pthread_mutex_unlock(&mutex);

    va_end(args1);
    exit(-1);
}

void _log_error(const char *file, const char *func, const int line, const char *format, ...)
{
    if ( log_config == NULL )
    {
        return;
    }

    if ( log_config->level < LOGGER_ERROR_LEVEL)
    {
        return;
    }

    va_list args1;
    va_start(args1, format);

    int pid = get_process_id(), tid = get_thread_id();

    char log_str[LIMIT_LOG_LENGTH], log_content[LIMIT_LOG_LENGTH - 200];

	vsprintf(log_content, format, args1);   
    sprintf(log_str, "%s %s [%d][%d] [%s %s:%d] [ERROR] %s\n", __DATE__, __TIME__, pid, tid, file, func, line, log_content);

    pthread_mutex_lock(&mutex);

    fprintf(log_config->file_cur, "%s", log_str);
    fflush(log_config->file_cur);

    log_config->log_size_cur += strlen(log_str);
    check_log_size();

    pthread_mutex_unlock(&mutex);

    va_end(args1);
}


void _log_warn(const char *file, const char *func, const int line, const char *format, ...)
{
    if ( log_config == NULL )
    {
        return;
    }

    if ( log_config->level < LOGGER_WARN_LEVEL)
    {
        return;
    }

    va_list args1;
    va_start(args1, format);

    int pid = get_process_id(), tid = get_thread_id();

    char log_str[LIMIT_LOG_LENGTH], log_content[LIMIT_LOG_LENGTH - 200];

	vsprintf(log_content, format, args1);   
    sprintf(log_str, "%s %s [%d][%d] [%s %s:%d] [WARN] %s\n", __DATE__, __TIME__, pid, tid, file, func, line, log_content);

    pthread_mutex_lock(&mutex);

    fprintf(log_config->file_cur, "%s", log_str);
    fflush(log_config->file_cur);

    log_config->log_size_cur += strlen(log_str);
    check_log_size();

    pthread_mutex_unlock(&mutex);

    va_end(args1);
}


void _log_info(const char *file, const char *func, const int line, const char *format, ...)
{
    if ( log_config == NULL || log_config->file_cur == NULL )
    {
        return;
    }

    if ( log_config->level < LOGGER_INFO_LEVEL)
    {
        return;
    }

    // handler args
    va_list args1;
    va_start(args1, format);

    int pid = get_process_id(), tid = get_thread_id();

    char log_str[LIMIT_LOG_LENGTH], log_content[LIMIT_LOG_LENGTH - 200];

	vsprintf(log_content, format, args1);   
    sprintf(log_str, "%s %s [%d][%d] [%s %s:%d] [INFO] %s\n", __DATE__, __TIME__, pid, tid, file, func, line, log_content);

    pthread_mutex_lock(&mutex);

    fprintf(log_config->file_cur, "%s", log_str);
    fflush(log_config->file_cur);
    log_config->log_size_cur += strlen(log_str);
    check_log_size();

    pthread_mutex_unlock(&mutex);

    va_end(args1);
}


void _log_debug(const char *file, const char *func, const int line, const char *format, ...)
{
    if ( log_config == NULL )
    {
        return;
    }

    if ( log_config->level < LOGGER_DEBUG_LEVEL)
    {
        return;
    }

    va_list args1;
    va_start(args1, format);

    int pid = get_process_id(), tid = get_thread_id();

    char log_str[LIMIT_LOG_LENGTH], log_content[LIMIT_LOG_LENGTH - 200];

	vsprintf(log_content, format, args1);   
    sprintf(log_str, "%s %s [%d][%d] [%s %s:%d] [DEBUG] %s\n", __DATE__, __TIME__, pid, tid, file, func, line, log_content);

    pthread_mutex_lock(&mutex);

    fprintf(log_config->file_cur, "%s", log_str);
    fflush(log_config->file_cur);

    log_config->log_size_cur += strlen(log_str);
    check_log_size();

    pthread_mutex_unlock(&mutex);

    va_end(args1);
}

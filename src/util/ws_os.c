#include "ws_os.h"

int get_process_id()
{
    int pid = getpid();
    return pid;
}

int get_thread_id()
{
    int tid = syscall(SYS_gettid);
    return tid;
}

int get_ts()
{
    time_t ts = time(NULL);
    return time(&ts);
}

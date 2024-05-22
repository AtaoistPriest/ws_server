#ifndef SRC_TOOLS_OS_H
#define SRC_TOOLS_OS_H

#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>

extern long syscall(long pid, ...);

int get_process_id();
int get_thread_id();
int get_ts();

#endif

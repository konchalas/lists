#ifndef _THREAD_H_
#define _THREAD_H_

#include <stdio.h>
#include <pthread.h>
#include <stdint.h>

#if defined (__sun)
#    include <sys/pset.h>  /* P_PID, processor_bind() */
#    include <sys/types.h>
#    include <sys/processor.h>
#    include <sys/procset.h>
#    include <unistd.h>    /* getpid() */
#elif (defined (__gnu_linux__) || defined (__gnu__linux) || defined (__linux__))
#    include <sched.h>     /* CPU_SET, CPU_ZERO, cpu_set_t, sched_setaffinity() */
#endif

#include <config.h>
#include <system.h>

/*inline*/ void setThreadId(int32_t id);
/*inline*/ int32_t getThreadId(void);
/*inline*/ int threadPin(int32_t cpu_id);
/*inline*/ int StartThreadsN(int nthreads, void *(*func)(void *), int mode);
/*inline*/ void JoinThreadsN(int nthreads);
/*inline*/ void resched(void);

#endif


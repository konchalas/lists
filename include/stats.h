#ifndef _STATS_H_
#define _STATS_H_

#include <config.h>

void init_cpu_counters(void);
void start_cpu_counters(int id);
void stop_cpu_counters(int id);
void printStats(void);

#endif

#include "bench_utils.h"

#ifndef NB_NUMA_NODES
#define NB_NUMA_NODES 1
#define NUMA_NODE_SIZE USE_CPUS
#endif


int compute_cpu_id(int thread_id)
{
    int numa_id = thread_id % NB_NUMA_NODES;
    int cpu_id = thread_id / NB_NUMA_NODES;

    return numa_id * NUMA_NODE_SIZE + cpu_id;
}

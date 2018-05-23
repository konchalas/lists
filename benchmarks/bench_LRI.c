#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sched.h>
#include <pthread.h>
#include <stdlib.h>

#include "fastrand.h"
#include "config.h"
#include "debug.h"

#include "bench_utils.h"

#include "bench_interface.h"
#include "mem_interface.h"


#include "liu.hpp"
#include "liumod.hpp"
#include "liumod2.hpp"

#include "tinymt64.h"

#ifndef DURATION
#define DURATION 1
#endif

#ifndef RANDOM
#define RANDOM 0
#endif

/* used by debug.h*/
__thread FILE *debug_output;

__thread int __thread_id;

volatile int main_work = 1;
static pthread_barrier_t init_barr;

void *table;

int64_t* ops;
int64_t totops, minops, maxops;

void ALRMhandler (int sig)
{
    main_work = 0;
}

void* my_thread(void* threadid)
{
    cpu_set_t cpuset;

    int64_t round=0;
    int64_t value=0;
    int64_t key;
    int op;

    int op_min,op_range;

    int64_t max_key_val= 1 << MAX_KEY;
    int i =0;

    tinymt64_t randomvar;
    
    if(NLOOKUP==0){
        op_min=0;
        op_range=100;
    }
    else if(NLOOKUP>=1){
        op_min=100;
        op_range=100;
    }
    else{
        op_min=0;
        op_range=(1/(1-NLOOKUP))*100;
    }

    
    __thread_id = (int)(int64_t)threadid;

    THREAD_INIT(__thread_id);

    pool_thread_init();
    
    table_thread_init(&table);

    PRINT_DEBUG_INIT();

    CPU_ZERO(&cpuset);
    CPU_SET(compute_cpu_id(__thread_id), &cpuset);

#if (RANDOM==0)
    fastRandomSetSeed(time(NULL)+__thread_id*100);
#else
    tinymt64_init(&randomvar, time(NULL)+__thread_id*100);
#endif
    
    /* pin the thread to a core */
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset))
    {
        fprintf(stderr, "Thread pinning failed!\n");
        exit(1);
    }


    /* in this test, we start from a half full table */
    /* check if thread should participate in loading the table */
    if(__thread_id< NB_NUMA_NODES){
    
        printf("starting loading the table for thread %d\n",__thread_id);

        int subset = (max_key_val)/NB_NUMA_NODES;
        
        struct timespec before,after;

        clock_gettime(CLOCK_REALTIME, &before);
    
        for(i=0; i < subset/2;i++){
#if (RANDOM==0)
            key = fastRandomRange32(0, max_key_val);
#else
            key = max_key_val * tinymt64_generate_double(&randomvar);
#endif
            table_insert(table,key,key);
        }
    
        clock_gettime(CLOCK_REALTIME, &after);
    
        printf("thread %d: time %.2lf\n",__thread_id, ((double)after.tv_sec - before.tv_sec)+((double)after.tv_nsec - before.tv_nsec)/(1000*1000*1000));
    }
    
    /* wait for the others to initialize */
    int ret = pthread_barrier_wait(&init_barr);
    if (ret != 0 && ret != PTHREAD_BARRIER_SERIAL_THREAD)
    {
        fprintf(stderr, "Waiting on the barrier failed!\n");
        exit(1);
    }
    

    for (round = 0; main_work; round++)
    {
        op = fastRandomRange(op_min, op_range);
#if (RANDOM==0)
        key = fastRandomRange32(0, max_key_val);
#else
        key = max_key_val * tinymt64_generate_double(&randomvar);
#endif
        if(op < 50){
            /*printf("******** INSERT %ld\n",key);*/
            table_insert(table,key,key);
        }
        else if(op<100){
            table_remove(table, key);
        }
        else{
            /*printf("******** LOOKUP %ld\n",key);*/
            table_lookup(table, key, &value);
        }
    }

    /* store the result */
    ops[__thread_id] = round;

    PRINT_DEBUG_FINALIZE();


    return NULL;
}


int main(void)
{

    pthread_t* thr;
    int testtime = DURATION;

    signal (SIGALRM, ALRMhandler);

    printf("########## exp setup: \t %d threads \t %d cores\n",N_THREADS,USE_CPUS);


    thr = (pthread_t*) malloc(N_THREADS * sizeof(pthread_t));
    ops = (int64_t*) malloc(N_THREADS * sizeof(int64_t));
    memset(ops, 0, N_THREADS * sizeof(int64_t));

    
    if(pthread_barrier_init(&init_barr, NULL, N_THREADS+1))
    {
        printf("Could not create a barrier\n");
        exit(1);
    }

    pool_thread_init();
    
    table_init(&table, DEPTH, BUCKET_DEPTH);


    /* create the threads */
    int i = 0;
    
    for(i=0; i < N_THREADS; i++){
        if(pthread_create(&thr[i], NULL, &my_thread, (void*)(int64_t)i)){
            printf("Could not create thread %d\n", i);
            exit(1);
        }
    }
    
    /* wait for everyone to initialize, then set the alarm */
    int ret = pthread_barrier_wait(&init_barr);
    if (ret != 0 && ret != PTHREAD_BARRIER_SERIAL_THREAD){
        fprintf(stderr, "Waiting on the barrier failed!\n");
        exit(1);
    }

    alarm (testtime);
    totops = 0; minops = 1 << 30; maxops = 0;
    for(i = 0; i < N_THREADS; i++){
        if(pthread_join(thr[i], NULL)){
            fprintf(stderr, "Could not join thread %d\n", i);
            exit(1);
        }        
        printf("[%d]: %lld\n" , i, (long long int)ops[i]);
        
        totops += ops[i];
        if (ops[i] > maxops) maxops = ops[i];
        if (ops[i] < minops) minops = ops[i];
    }

    table_finalize(table);

    printf("throughput: %lld\t\t", (long long int)totops/testtime);
    printf("fairness: %f\n", ((float)maxops)/minops);

    printf("maxops: %lld \t minops: %lld\n", (long long int)maxops, (long long int)minops);

    return 0;
}

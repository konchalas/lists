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

#ifndef RANDOM
#define RANDOM 0
#endif

#define INSERT_FACT 0.8


/* used by debug.h*/
__thread FILE *debug_output;

__thread int __thread_id;

static pthread_barrier_t init_barr;

void *table;

double* duration;
double totdur, mindur, maxdur;

/* void ALRMhandler (int sig) */
/* { */
/*     main_work = 0; */
/* } */

void* my_thread(void* threadid)
{
    cpu_set_t cpuset;

    int64_t round=0;
    int64_t value=0;
    int64_t key;
    int op;

    int op_min,op_range;

    int64_t max_key_val= 1 << MAX_KEY;

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

    /* wait for the others to initialize */
    int ret = pthread_barrier_wait(&init_barr);
    if (ret != 0 && ret != PTHREAD_BARRIER_SERIAL_THREAD)
    {
        fprintf(stderr, "Waiting on the barrier failed!\n");
        exit(1);
    }

    /* compute the number of insert operations to be executed on the
     * table */
    round = (int)(((float)max_key_val/N_THREADS) * INSERT_FACT);

    struct timespec start_tt, end_tt;
    clock_gettime(CLOCK_MONOTONIC, &start_tt);

    
    while(round > 0){

        op = fastRandomRange(op_min, op_range);
#if (RANDOM==0)
        key = fastRandomRange32(0, max_key_val);
#else
        key = max_key_val * tinymt64_generate_double(&randomvar);
#endif

        if(op < 100){
            /*printf("******** INSERT %ld\n",key);*/
            table_insert(table,key,key);
            round--;
        }
        else{
            /*printf("******** LOOKUP %ld\n",key);*/
            table_lookup(table, key, &value);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_tt);


    //table_print(table, 1);
#ifdef VERBOSE
    table_print(table, 1);
#endif
    
    PRINT_DEBUG_FINALIZE();

    double total_time_ms= (double) (((double)end_tt.tv_sec - start_tt.tv_sec)*1000) + (((double)end_tt.tv_nsec - start_tt.tv_nsec)/(1000*1000));
    

    //printf("Thread %d: %lf\n", __thread_id, total_time_ms);

    duration[__thread_id]=total_time_ms;

    
    return NULL;
}


int main(void)
{

    pthread_t* thr;

    printf("########## exp setup: \t %d threads \t %d cores\n",N_THREADS,USE_CPUS);


    thr = (pthread_t*) malloc(N_THREADS * sizeof(pthread_t));
    duration = (double*) malloc(N_THREADS * sizeof(double));
    memset(duration, 0, N_THREADS * sizeof(double));

    
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

    totdur = 0; mindur = 1 << 30; maxdur = 0;
    for(i = 0; i < N_THREADS; i++){
        if(pthread_join(thr[i], NULL)){
            fprintf(stderr, "Could not join thread %d\n", i);
            exit(1);
        }        
        printf("[%d]: %lf\n" , i, duration[i]);
        
        totdur += duration[i];
        if (duration[i] > maxdur) maxdur = duration[i];
        if (duration[i] < mindur) mindur = duration[i];
    }
    
    table_finalize(table);


    printf("avg_time: %lf\n", totdur/N_THREADS);
    printf("mintime: %lf \t maxtime: %lf\n", mindur, maxdur);

    
    /* printf("throughput: %lld\t\t", (long long int)totops/testtime); */
    /* printf("fairness: %f\n", ((float)maxops)/minops); */

    /* printf("maxops: %lld \t minops: %lld\n", (long long int)maxops, (long long int)minops); */

    return 0;
}

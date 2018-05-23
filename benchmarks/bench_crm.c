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


typedef struct action{
    int thread_id; /* the thread that should apply the op */
    int op; /* 1 -> key to be inserted during first step; 2 -> key to
             * be inserted during second step */
} action_t;

/* used by debug.h*/
__thread FILE *debug_output;

__thread int __thread_id;


static pthread_barrier_t init_barr;
static pthread_barrier_t internal_barr; /* synch of working threads */


void *table;


volatile action_t *key_status;

volatile int id_shift;

int nb_loops=4;

void* my_thread(void* threadid)
{
    cpu_set_t cpuset;
    
    int64_t max_key_val= 1 << MAX_KEY;
    int64_t value=0;

    int loop=0;
    
    __thread_id = (int)(int64_t)threadid;

    THREAD_INIT(__thread_id);

    pool_thread_init();
    
    table_thread_init(&table);

    PRINT_DEBUG_INIT();

    CPU_ZERO(&cpuset);
    CPU_SET(compute_cpu_id(__thread_id), &cpuset);
    
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


    int64_t k=0;
    int found=0;
    for(loop=0; loop<nb_loops; loop++){
        
        /* run first step where only inserts are applied */

        for(k=0; k<max_key_val; k++){
            if(key_status[k].thread_id == __thread_id){
                if(key_status[k].op == loop%2){
                    table_insert(table,k,k);
                }
                else{
                    table_remove(table,k);
                }
            }
        }        

    
        /* wait for the other working threads */
        ret = pthread_barrier_wait(&internal_barr);
        if (ret != 0 && ret != PTHREAD_BARRIER_SERIAL_THREAD)
        {
            fprintf(stderr, "Waiting on the barrier failed!\n");
            exit(1);
        }
        
        /* time to check that all modifications were successful */
        for(k=0; k<max_key_val; k++){
            if((key_status[k].thread_id+id_shift)%N_THREADS == __thread_id){
                found = table_lookup(table,k,&value);

                if(key_status[k].op == loop%2){
                    if(!found){
                        printf("ERROR(tid:%d): could not find inserted key %ld (by %d)\n",__thread_id, k, key_status[k].thread_id);
                        exit(1);
                    }
                    if(value != k){
                        printf("ERROR(tid:%d): returned value %ld for key %ld (by %d)\n",__thread_id, value, k, key_status[k].thread_id);
                        exit(1);
                    }
                }
                else{
                    if(found){
                        printf("ERROR(tid:%d): found key %ld although it was removedby %d\n",__thread_id, k, key_status[k].thread_id);
                    }
                }
            }
        }


        /* wait for the other working threads */
        ret = pthread_barrier_wait(&internal_barr);
        if (ret != 0 && ret != PTHREAD_BARRIER_SERIAL_THREAD)
        {
            fprintf(stderr, "Waiting on the barrier failed!\n");
            exit(1);
        }
    }


    /* now run all threads for a few iterations */
    for(loop=0; loop<nb_loops; loop++){
        
        /* run first step where only inserts are applied */

        for(k=0; k<max_key_val; k++){
            if(key_status[k].thread_id == __thread_id){
                if(key_status[k].op == loop%2){
                    table_insert(table,k,k);
                }
                else{
                    table_remove(table,k);
                }
            }
        }
    }

    loop--;
    
    /* wait for the other working threads */
    ret = pthread_barrier_wait(&internal_barr);
    if (ret != 0 && ret != PTHREAD_BARRIER_SERIAL_THREAD)
    {
        fprintf(stderr, "Waiting on the barrier failed!\n");
        exit(1);
    }


    /* time to check that all modifications were successful */
    for(k=0; k<max_key_val; k++){
        if((key_status[k].thread_id+id_shift)%N_THREADS == __thread_id){
            found = table_lookup(table,k,&value);

            if(key_status[k].op == loop%2){
                if(!found){
                    printf("ERROR(tid:%d): could not find inserted key %ld (by %d)\n",__thread_id, k, key_status[k].thread_id);
                    exit(1);
                }
                if(value != k){
                    printf("ERROR(tid:%d): returned value %ld for key %ld (by %d)\n",__thread_id, value, k, key_status[k].thread_id);
                    exit(1);
                }
            }
            else{
                if(found){
                    printf("ERROR(tid:%d): found key %ld although it was removedby %d\n",__thread_id, k, key_status[k].thread_id);
                }
            }
        }
    }
    
    PRINT_DEBUG_FINALIZE();


    return NULL;
}


int main(void)
{

    pthread_t* thr;

    printf("########## exp setup: \t %d threads \t %d cores\n",N_THREADS,USE_CPUS);


    thr = (pthread_t*) malloc(N_THREADS * sizeof(pthread_t));

    key_status= (action_t*) malloc((1<<MAX_KEY) * sizeof(action_t));

    
    if(pthread_barrier_init(&init_barr, NULL, N_THREADS+1))
    {
        printf("Could not create a barrier\n");
        exit(1);
    }

    if(pthread_barrier_init(&internal_barr, NULL, N_THREADS))
    {
        printf("Could not create a barrier\n");
        exit(1);
    }


    pool_thread_init();
    table_init(&table, DEPTH, BUCKET_DEPTH);

    /* init the status of each key */
    fastRandomSetSeed(time(NULL));

    id_shift=fastRandomRange32(0, N_THREADS);

    int64_t k=0;
    for(k=0; k < (1<<MAX_KEY); k++){
        key_status[k].thread_id=fastRandomRange32(0, N_THREADS);
        key_status[k].op=fastRandomRange32(0, 100)%2;
    }
    
    
    memset((void*)key_status,0,MAX_KEY*sizeof(int));

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

    
    for(i = 0; i < N_THREADS; i++){
        if(pthread_join(thr[i], NULL)){
            fprintf(stderr, "Could not join thread %d\n", i);
            exit(1);
        }
        printf("Thread %d terminated correctly\n",i);
    }

    table_finalize(table);
    
    printf("TEST PASSED\n");
    
    free((void*)key_status);

    return 0;
}

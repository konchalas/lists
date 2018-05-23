#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sched.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>

#include "fastrand.h"
#include "config.h"
#include "debug.h"

#include "bench_utils.h"

#include "bench_interface.h"
#include "mem_interface.h"
#include "system.h"


#define N_THREADS 2
#define MAX_KEY 1024
#define DEPTH 10
#define NLOOKUP 0.9
#define BUCKET_DEPTH 10


extern void table_thread_init(void **table);
extern int table_insert(void *table, int64_t key, int64_t value);
extern int table_remove(void *table, int64_t key);
extern int table_lookup(void *table, int64_t key, int64_t *value);
extern int table_lookup_safe(void *table, int64_t key, int64_t *value);
extern int table_print(void *table, int print_level);


/* used by debug.h*/
__thread FILE *debug_output;

__thread int __thread_id;

volatile int main_work = 1;
static pthread_barrier_t init_barr;

void *table;

int64_t* ops;
int64_t totops, minops, maxops;

volatile int *key_status;


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

    int64_t max_key_val= MAX_KEY;

    int insert_return_value=0;

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

    //pool_thread_init();

    table_thread_init(&table);

    PRINT_DEBUG_INIT();

    CPU_ZERO(&cpuset);
    //CPU_SET(compute_cpu_id(__thread_id), &cpuset);

    fastRandomSetSeed(time(NULL)+__thread_id*100);

    /* pin the thread to a core */
    /*if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset))
    {
        fprintf(stderr, "Thread pinning failed!\n");
        exit(1);
        }*/

    /* wait for the others to initialize */
    int ret = pthread_barrier_wait(&init_barr);
    if (ret != 0 && ret != PTHREAD_BARRIER_SERIAL_THREAD)
    {
        fprintf(stderr, "Waiting on the barrier failed!\n");
        exit(1);
    }


    for (round = 0; round < 1000000 || main_work; round++)
    {
        op = fastRandomRange(op_min, op_range);
        key = fastRandomRange32(0, max_key_val);

        /*key = fastRandomRange32((__thread_id*(max_key_val/N_THREADS)), max_key_val/N_THREADS);

          assert(key < max_key_val);*/

        if(op < 100){
            /*printf("******** INSERT %ld\n",key);*/
            if(key_status[key]==0){
                key_status[key]=1;
            }

            insert_return_value=table_insert(table,key,key);

            //table_lookup_safe(table, key, &value);
            table_lookup(table, key, &value);

            if(value != key){
                /*pthread_mutex_lock(&mymutex);*/
                PRINT_ERROR("ERROR %d: value %lld returned for just inserted key %lld (returned val %d)\n", __thread_id,(long long int)value, (long long int)key,insert_return_value);
                round=1000000;
                sleep(1);
                table_print(table, 1);
                value=MAX_KEY+1;
                table_lookup(table,key,&value);
                PRINT_DEBUG(1,"a second lookup return %lld - %lld\n",(long long int) key, (long long int) value);
                if(key==value){
                    table_print(table, 1);
                }
                /*pthread_mutex_unlock(&mymutex);*/
                /*exit(1);*/
                return NULL;
            }

            if(key_status[key]!=2){
                key_status[key]=2;
            }
        }
        else{
            /*printf("******** LOOKUP %ld\n",key);*/
            value=MAX_KEY+1;
            if(key_status[key]==0){
                if(table_lookup(table,key,&value)){
                    if(key_status[key]==0){
                        /*pthread_mutex_lock(&mymutex);*/
                        PRINT_ERROR("ERROR %d: key %lld was found but not inserted\n", __thread_id, (long long int)key);
                        round=1000000;
                        sleep(1);

                        table_print(table, 1);
                        /*pthread_mutex_unlock(&mymutex);*/
                        /*exit(1);*/
                        return NULL;
                    }
                }
            }
            else if(key_status[key]==2){
                if(!table_lookup(table,key,&value)){
                    /*pthread_mutex_lock(&mymutex);*/
                    PRINT_ERROR("ERROR %d: key %lld should have been found\n", __thread_id, (long long int)key);
                    round=1000000;
                    sleep(1);

                    table_print(table, 1);
                    /*pthread_mutex_unlock(&mymutex);*/
                    /*exit(1);*/
                    return NULL;
                }
                else if(key != value){
                    /*pthread_mutex_lock(&mymutex);*/
                    PRINT_ERROR("ERROR %d: value %lld returned for key %lld\n", __thread_id, (long long int)value, (long long int)key);

                    round=1000000;
                    sleep(1);

                    table_print(table, 1);
                    /*pthread_mutex_unlock(&mymutex);*/
                    /*exit(1);*/
                    return NULL;
                }
            }
            else{
                if(table_lookup(table,key,&value)){
                    if(key != value){
                        /*pthread_mutex_lock(&mymutex);*/
                        PRINT_ERROR("ERROR %d: value %lld returned for key %lld\n", __thread_id, (long long int)value, (long long int)key);
                        round=1000000;
                        sleep(1);


                        table_print(table, 1);
                        /*pthread_mutex_unlock(&mymutex);*/
                        /*exit(1);*/
                        return NULL;
                    }
                }
            }
        }
    }

    /* store the result */
    ops[__thread_id] = round;

    PRINT_DEBUG_FINALIZE();

    //printf("thread terminating: %d\n",__thread_id);


    return NULL;
}


int main(void)
{

    pthread_t* thr;
    int testtime = 1;

    signal (SIGALRM, ALRMhandler);

    printf("########## exp setup: \t %d threads \t %d cores\n",N_THREADS,USE_CPUS);


    thr = (pthread_t*) malloc(N_THREADS * sizeof(pthread_t));
    ops = (int64_t*) malloc(N_THREADS * sizeof(int64_t));
    memset(ops, 0, N_THREADS * sizeof(int64_t));

    key_status= (int*) malloc((MAX_KEY) * sizeof(int));


    if(pthread_barrier_init(&init_barr, NULL, N_THREADS+1))
    {
        printf("Could not create a barrier\n");
        exit(1);
    }


    //pool_thread_init();
    //table_init(&table, DEPTH, BUCKET_DEPTH);

    memset((void*)key_status,0,(MAX_KEY)*sizeof(int));

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

#ifdef ADEBUG
    main_work = 0;
#else
    alarm (testtime);
#endif

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

    //table_finalize(table);

    printf("throughput: %lld\t\t", (long long int)totops/testtime);
    printf("fairness: %f\n", ((float)maxops)/minops);

    printf("maxops: %lld \t minops: %lld\n", (long long int)maxops, (long long int)minops);

    free((void*)key_status);

    return 0;
}

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sched.h>
#include <pthread.h>
#include <stdlib.h>
#include <argp.h>

#include "fastrand.h"
#include "config.h"
#include "debug.h"
#include "system.h"

#include "bench_utils.h"

#include "bench_interface.h"
#include "mem_interface.h"

#include "liu.h"
#include "liumod.h"
#include "liumod2.h"

#include "tinymt64.h"


#ifndef RANDOM
#define RANDOM 0
#endif

#define DEPTH 10
#define BUCKET_DEPTH 10
#define NB_NUMA_NODES 2 

int N_THREADS;
int N_CORES_PER_CPU;
int MAX_KEY;
int DURATION;
float NLOOKUP;
unsigned long add;
unsigned long added;
unsigned long remove__;
unsigned long removed;
unsigned long contains_;
unsigned long found;




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

int thread_affinity[16];


void* my_thread(void* threadid)
{
    cpu_set_t cpuset;

    int64_t round=0;
    int64_t value=0;
    int64_t key;
    int op;

    int op_min,op_range;

    int64_t max_key_val = MAX_KEY;

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

    //pool_thread_init();

    table_thread_init(&table);

    PRINT_DEBUG_INIT();


    CPU_ZERO(&cpuset);
    CPU_SET(compute_cpu_id(thread_affinity[__thread_id]), &cpuset);

#if (RANDOM==0)
    fastRandomSetSeed(time(NULL)+__thread_id*100);
#else
    tinymt64_init(&randomvar, time(NULL)+__thread_id*100);
#endif
    
    /////* pin the thread to a core */
    //if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset))
    //{
    //    fprintf(stderr, "Thread pinning failed!\n");
    //    exit(1);
    //}


    printf("starting loading the table for thread %d\n",__thread_id);

    /* in this test, we start from a half full table */
    /* check if thread should participate in loading the table */
    if(__thread_id < NB_NUMA_NODES){
    
      printf("starting loading the table for thread %d\n",__thread_id);

      int subset = (max_key_val)/NB_NUMA_NODES;
        
      struct timespec before,after;

      clock_gettime(CLOCK_REALTIME, &before);
    
      for(int i=0; i < subset/2;i++){
#if (RANDOM==0)
        key = fastRandomRange32(0, max_key_val);
#else
        key = max_key_val * tinymt64_generate_double(&randomvar);
#endif
	printf("Inserting %d\n", key);
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
          if (table_insert(table,key,key)) {
            added++;
          }
//	printf("Inserting %d\n", key);
          add++;
        }
        else if(op<100){
          if (table_remove(table, key)) {
            removed++;
          }
//	printf("Removing %d\n", key);
          remove__++;
        }
        else{
//	printf("Searching %d\n", key);
            /*printf("******** LOOKUP %ld\n",key);*/
          if (table_lookup(table, key, &value)) {

 //           printf("******** found%ld\n",key);
            found++;
          }
          contains_++;
        }
    }

    /* store the result */
    ops[__thread_id] = round;

    PRINT_DEBUG_FINALIZE();


    return NULL;
}

struct arguments
{
  int nr_threads;
  int nr_cores_per_cpu;
  int max_key;
  float nlookup;
  float duration;
};


/* Parse a single option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;

  switch (key)
    {
    case 'n':
      arguments->nr_threads = atoi(arg);
      break;
    case 'c':
      arguments->nr_cores_per_cpu = atoi(arg);
      break;
    case 'k':
      arguments->max_key = atoi(arg);
      break;
    case 'l':
      arguments->nlookup = atof(arg);
      break;
    case 'd':
      arguments->duration = atoi(arg);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

const char *argp_program_version = "Bench LI";
const char *argp_program_bug_address = "konstantinos.chalas@grenoble-inp.org";
static char doc[] = "Benchmark using lookup and insert operations";
static char args_doc[] = "[FILENAME]...";


static struct argp_option options[] = { 
  { "N_THREADS", 'n', "COUNT", 0, "The number of threads to run the benchmark."},
  { "N_CORES_PER_CPU", 'c', "COUNT", 0, "The number of cores per cpu."},
  { "MAX_KEY", 'k', "COUNT", 0, "The maximum number of elements we can have in our list."},
  { "NLOOKUP", 'l', "COUNT", 0, "The percentage of lookups to perform. e.g 0.9 for 90% lookups"},
  { "DURATION", 'd', "COUNT", 0, "The maximum number of elements we can have in our list."},
  { 0 } 
};

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };



int main(int argc, char *argv[])
{
    pthread_t* thr;
    int testtime;
    struct arguments args;

    if(N_THREADS == 2) {
      thread_affinity[0] = 0;
      thread_affinity[1] = 8;
    } else if (N_THREADS == 4) {
      thread_affinity[0] = 0;
      thread_affinity[1] = 8;
      thread_affinity[2] = 1;
      thread_affinity[3] = 9;
    } else if (N_THREADS == 8) {
      thread_affinity[0] = 0;
      thread_affinity[1] = 8;
      thread_affinity[2] = 1;
      thread_affinity[3] = 9;
      thread_affinity[4] = 2;
      thread_affinity[5] = 10;
      thread_affinity[6] = 3;
      thread_affinity[7] = 11;
    } else if (N_THREADS == 16) {
      thread_affinity[0] = 0;
      thread_affinity[1] = 8;
      thread_affinity[2] = 1;
      thread_affinity[3] = 9;
      thread_affinity[4] = 2;
      thread_affinity[5] = 10;
      thread_affinity[6] = 3;
      thread_affinity[7] = 11;
      thread_affinity[8] = 4;
      thread_affinity[9] = 12;
      thread_affinity[10] = 5;
      thread_affinity[11] = 13;
      thread_affinity[12] = 6;
      thread_affinity[13] = 14;
      thread_affinity[14] = 7;
      thread_affinity[15] = 15;
    }

    argp_parse (&argp, argc, argv, 0, 0, &args);

    N_THREADS = args.nr_threads;
    N_CORES_PER_CPU = args.nr_cores_per_cpu;
    MAX_KEY = args.max_key;
    NLOOKUP = args.nlookup;
    DURATION = args.duration;

    signal (SIGALRM, ALRMhandler);

    printf("########## exp setup: \t %d threads \t %d cores \t %d cores per cpu \t %f nlookup \t %d duration\n",N_THREADS, USE_CPUS, N_CORES_PER_CPU, NLOOKUP, DURATION);


    testtime = DURATION;
    thr = (pthread_t*) malloc(N_THREADS * sizeof(pthread_t));
    ops = (int64_t*) malloc(N_THREADS * sizeof(int64_t));
    memset(ops, 0, N_THREADS * sizeof(int64_t));

    add = 0;
    added = 0;
    remove__ = 0;
    removed = 0;
    contains_ = 0;
    found = 0;



    if(pthread_barrier_init(&init_barr, NULL, N_THREADS+1))
    {
        printf("Could not create a barrier\n");
        exit(1);
    }

    //pool_thread_init();

    //table_init(&table, DEPTH, BUCKET_DEPTH);

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

    //table_finalize(table);

    printf("throughput: %lld\t\t", (long long int)totops/testtime);
    printf("fairness: %f\n", ((float)maxops)/minops);

    printf("maxops: %lld \t minops: %lld\n", (long long int)maxops, (long long int)minops);


    printf("Add: %ld, added: %ld\n", add, added);
    printf("Remove: %ld, removed: %ld\n", remove__, removed);
    printf("Contains: %ld, found: %ld\n", contains_, found);

    return 0;
}






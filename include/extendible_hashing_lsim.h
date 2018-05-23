#ifndef _EXTENDIBLE_HASHING_LSIM_H_
#define _EXTENDIBLE_HASHING_LSIM_H_

/* l stands for legacy: implementation of a hash table with as few
 * modifications as possible compared to the psim algo*/

#include <stdint.h>
#include "extendible_hashing_seq.h"
#include "tvec.h"
#include "pool.h"
#include "sim.h"

#include "extendible_hashing_sim.h"

typedef struct request_lsim{
    /*extendible_hashing_op_t opcode;
      unsigned int hkey;*/
    uint32_t opcode;
    /*uint32_t hkey;*/
    int64_t key;
    int64_t val;
    int64_t pad[13]; /* T: padding has a positive impact on throughput
                      * at least for "small" HT*/
} request_lsim_t;

typedef struct result_lsim{
    int64_t rVal;
    /*uint64_t status;*/
} result_lsim_t;

typedef struct half_lstate{
    uint64_t nb_items;
    item_t items[MAX_ITEMS];
    ToggleVector applied;
    result_lsim_t ret[N_THREADS];
} half_lstate_t;

typedef struct lstate {
    uint64_t nb_items;
    item_t items[MAX_ITEMS];
    ToggleVector applied;
    result_lsim_t ret[N_THREADS];
} lstate_t;

typedef struct extendible_hashing_lsim_local_ThreadState {
    PoolStruct pool;
    ToggleVector mask CACHE_ALIGN;
    ToggleVector toggle CACHE_ALIGN;
    ToggleVector my_bit CACHE_ALIGN;
    int local_index;
} extendible_hashing_lsim_local_ThreadState_t;


typedef struct extendible_hashing_lsim_instance {
    volatile pointer_t sp CACHE_ALIGN;
    volatile ToggleVector a_toggles CACHE_ALIGN;
#ifndef GLOBAL_ANNOUNCE
    volatile request_lsim_t announce[N_THREADS] CACHE_ALIGN;
#endif
    volatile lstate_t pool[N_THREADS * _SIM_LOCAL_POOL_SIZE_ + 1] CACHE_ALIGN;
    /*those don't change over time, so they do not need to be part of
      the state*/
    unsigned int prefix;
    int depth;
    /* easiest way to manage thread state when bsim_instance are
     * created dynamically*/
    extendible_hashing_lsim_local_ThreadState_t* threadState[N_THREADS] CACHE_ALIGN;
} extendible_hashing_lsim_instance_t;

typedef struct extendible_hash_table_lsim{
    int depth;
    extendible_hashing_lsim_instance_t **directory;
} extendible_hash_table_lsim_t;


extendible_hashing_lsim_local_ThreadState_t* extendible_hashing_lsim_local_ThreadState_init(int pid, int obj_size);


void extendible_hashing_lsim_init(extendible_hash_table_lsim_t **table, int depth);
void extendible_hashing_lsim_finalize(extendible_hash_table_lsim_t *table);
void extendible_hashing_lsim_insert(extendible_hash_table_lsim_t *table, int64_t key, int64_t value, int thread_id);
void extendible_hashing_lsim_remove(extendible_hash_table_lsim_t *table, int64_t key, int thread_id);
int extendible_hashing_lsim_lookup(extendible_hash_table_lsim_t *table, int64_t key, int64_t *value, int thread_id);
int extendible_hashing_lsim_lookup_through_lsim(extendible_hash_table_lsim_t *table, int64_t key, int64_t *value, int thread_id);

#ifdef GLOBAL_ANNOUNCE
volatile request_lsim_t lsim_announce[N_THREADS] CACHE_ALIGN;
#endif



#endif

#ifndef _EXTENDIBLE_HASHING_SIM2_H_
#define _EXTENDIBLE_HASHING_SIM2_H_

#include <extendible_hashing_sim.h>

/****** version including resizing ******/
#define _SIM_LOCAL_POOL_SIZE_         2

typedef union pointer_t {
    struct StructData2 {
        volatile int64_t seq;
        volatile void *index;
    } struct_data;
    volatile __int128 raw_data;
} pointer_t;

typedef struct request_dsim {
    /*extendible_hashing_op_t opcode;
      unsigned int hkey;*/
    uint64_t opcode;
    int64_t key;
    int64_t val;
    uint64_t seqnum1;
    uint64_t seqnum2;
    int64_t pad[11];
} request_dsim_t;


typedef struct half_dstate {
    int depth;
    extendible_hashing_bsim_instance_t **directory;
    /*ToggleVector applied;*/
#ifdef DEBUG
    int counter;
#endif
} half_dstate_t;

typedef struct dstate {
    int depth;
    extendible_hashing_bsim_instance_t **directory;
#ifdef DEBUG
    int counter;
#endif
#ifdef CHECK_CORRECTNESS
    int64_t tt_seqnum;
#endif
    int32_t pad[PAD_CACHE(sizeof(half_dstate_t))];
} dstate_t;


typedef struct extendible_hashing_dsim_instance {
#ifdef CHECK_CORRECTNESS
    int64_t tt_seqnum;
#endif
    volatile pointer_t sp CACHE_ALIGN;
    /*volatile ToggleVector a_toggles CACHE_ALIGN;*/
    volatile request_dsim_t help[N_THREADS] CACHE_ALIGN;
    //volatile dstate_t pool[N_THREADS * _SIM_LOCAL_POOL_SIZE_ + 1] CACHE_ALIGN;
    extendible_hashing_sim_local_ThreadState_t* threadState[N_THREADS] CACHE_ALIGN;
    /*PoolStruct thread_state_pools[N_THREADS] CACHE_ALIGN;*/
} extendible_hashing_dsim_instance_t;




void extendible_hashing_sim2_init(extendible_hashing_dsim_instance_t **table, int depth_table, int depth_buckets);
void extendible_hashing_sim2_finalize(extendible_hashing_dsim_instance_t *table);
int extendible_hashing_sim2_insert(extendible_hashing_dsim_instance_t *table, int64_t key, int64_t value);
int extendible_hashing_sim2_remove(extendible_hashing_dsim_instance_t *table, int64_t key);

int extendible_hashing_sim2_lookup_through_bsim(extendible_hashing_dsim_instance_t *table, int64_t key, int64_t *value);
int extendible_hashing_sim2_lookup(extendible_hashing_dsim_instance_t *table, int64_t key, int64_t *value);

int extendible_hashing_sim2_lookup_for_debug(extendible_hashing_dsim_instance_t *table, int64_t key, int64_t *value);


void extendible_hashing_print_dsim(int print_level, extendible_hashing_dsim_instance_t *table);

#endif

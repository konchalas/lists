#ifndef _EXTENDIBLE_HASHING_SIM_H_
#define _EXTENDIBLE_HASHING_SIM_H_

#include <stdint.h>
#include "extendible_hashing_seq.h"
#include "tvec.h"
#include "pool.h"


typedef enum request_status{
    BLOCKED = 0,
    SUCCESS,
    FAILED
} request_status_t;

typedef enum extendible_hashing_op{
    INSERT = 0,
    LOOKUP,
    REMOVE
} extendible_hashing_op_t;

typedef struct request_bsim{
    uint64_t opcode;
    int64_t key;
    int64_t val;
    uint64_t seqnum;
    int64_t pad[12]; /* T: padding has a positive impact on throughput
                      * at least for "small" HT*/
} request_bsim_t;

typedef struct result_bsim{
#if defined(VERBOSE) || defined(ADEBUG)
    uint64_t seqnum;
    uint64_t opcode;
    int64_t key;
#endif    
    int64_t rVal;
    uint64_t status;
} result_bsim_t;

typedef struct result_reference{
    uint64_t seqnum : 48;
    uint64_t index : 16;
} result_reference_t;

typedef struct half_bstate{
    uint64_t nb_items;
    ToggleVector applied;
    //struct half_bstate *next;
    item_t items[MAX_ITEMS];
    result_reference_t ret[N_THREADS];
} half_bstate_t;

typedef struct bstate_without_ret_t{
    uint64_t nb_items;
    ToggleVector applied;
    //struct half_bstate *next;
    item_t items[MAX_ITEMS];
} bstate_without_ret_t;

typedef struct bstate {
    uint64_t nb_items;
    ToggleVector applied;
    //struct bstate *next;
    item_t items[MAX_ITEMS];
    result_reference_t ret[N_THREADS];
    int32_t pad[PAD_CACHE(sizeof(half_bstate_t))];
} bstate_t;



typedef struct extendible_hashing_sim_local_ThreadState {
    //int local_index;
#ifdef WITH_TOGGLE_VECTOR
    ToggleVector mask;
    ToggleVector toggle;
    ToggleVector my_bit;
#endif
} extendible_hashing_sim_local_ThreadState_t;

typedef struct extendible_hashing_bsim_instance {
    volatile bstate_t *sp CACHE_ALIGN;
#ifdef WITH_TOGGLE_VECTOR
    volatile ToggleVector a_toggles CACHE_ALIGN;
#endif
#ifndef GLOBAL_ANNOUNCE
    volatile request_bsim_t announce[N_THREADS] CACHE_ALIGN;
#endif
    unsigned int prefix;
    int depth;
    /* easiest way to manage thread state when bsim_instance are
     * created dynamically*/
    /* TR: To be moved out of the struct ?? */
    extendible_hashing_sim_local_ThreadState_t* threadState[N_THREADS] CACHE_ALIGN;
} extendible_hashing_bsim_instance_t;

typedef struct extendible_hash_table_sim{
    int depth;
    extendible_hashing_bsim_instance_t **directory;
} extendible_hash_table_sim_t;


extern __thread uint64_t sequence_number;

#if defined(VERBOSE) || defined(ADEBUG)    
extern __thread uint64_t op_seqnum;
extern __thread int running_op;
#endif


extendible_hashing_sim_local_ThreadState_t* extendible_hashing_sim_local_ThreadState_init(int pid, int obj_size);

void extendible_hashing_sim_init(extendible_hash_table_sim_t **table, int depth);
void extendible_hashing_sim_finalize(extendible_hash_table_sim_t *table);
void extendible_hashing_sim_insert(extendible_hash_table_sim_t *table, int64_t key, int64_t value);
int extendible_hashing_sim_lookup(extendible_hash_table_sim_t *table, int64_t key, int64_t *value);
int extendible_hashing_sim_lookup_through_bsim(extendible_hash_table_sim_t *table, int64_t key, int64_t *value);
void extendible_hashing_sim_remove(extendible_hash_table_sim_t *table, int64_t key);


/*inline*/ int64_t extendible_hashing_bsim_ApplyOp(extendible_hashing_bsim_instance_t *bsim_instance, extendible_hashing_sim_local_ThreadState_t *th_state, extendible_hashing_op_t operation, int64_t key, int64_t value, uint64_t seqnum);

request_status_t extendible_hashing_sim_insert_in_bstate(bstate_t *bstate, extendible_hashing_sim_local_ThreadState_t *th_state, int64_t key, int64_t value);

request_status_t extendible_hashing_sim_remove_from_bstate(bstate_t *bstate, extendible_hashing_sim_local_ThreadState_t *th_state, int64_t key);


int extendible_hashing_sim_lookup_in_bstate(bstate_t *bstate, int64_t key, int64_t *value);

int extendible_hashing_sim_lookup_in_bstate_noblock(bstate_t *bstate, int64_t key, int64_t *value);


void extendible_hashing_print_bsim(int print_level,extendible_hashing_bsim_instance_t *bucket);

#ifdef CHECK_CORRECTNESS
int extendible_hashing_check_bstate(int print_level, bstate_t *bstate);
#endif

#ifdef GLOBAL_ANNOUNCE
volatile request_bsim_t announce[N_THREADS] CACHE_ALIGN;
#endif

volatile result_bsim_t return_values[N_THREADS][N_THREADS] CACHE_ALIGN;

#endif

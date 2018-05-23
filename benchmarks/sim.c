#include "bench_interface.h"
#include "../include/extendible_hashing_seq.h"
#include "../include/extendible_hashing_sim.h"

#include "epoch.h"

void table_init(void **table, int depth, int bucket_depth){
    extendible_hashing_sim_init((extendible_hash_table_sim_t **)table,depth);
}


void table_finalize(void *table){
    extendible_hashing_sim_finalize((extendible_hash_table_sim_t *)table);
}

inline int table_insert(void *table, int64_t key, int64_t value){
    epoch_start();
    extendible_hashing_sim_insert(table, key, value);
    epoch_done();
    return 0;
}


inline int table_remove(void *table, int64_t key){
    epoch_start();
    extendible_hashing_sim_remove(table, key);
    epoch_done();
    return 0;
}


inline int table_lookup(void *table, int64_t key, int64_t *value){
    epoch_start();
    int ret = extendible_hashing_sim_lookup(table, key, value);
    epoch_done();
    return (ret==SUCCESS);
}


int table_lookup_safe(void *table, int64_t key, int64_t *value){
    epoch_start();
    int ret = extendible_hashing_sim_lookup_through_bsim(table, key, value);
    epoch_done();
    return (ret==SUCCESS);
}

int table_print(void *table, int print_level){
    printf("No print function for SIM\n");
    return -1;
}

    
void table_thread_init(void **table){
    return;
}


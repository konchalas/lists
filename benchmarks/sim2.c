#include "bench_interface.h"
#include "../include/extendible_hashing_seq.h"
#include "../include/extendible_hashing_sim2.h"

#include "epoch.h"


void table_init(void **table, int depth, int bucket_depth){
    extendible_hashing_sim2_init((extendible_hashing_dsim_instance_t **)table,depth, bucket_depth);
}


void table_finalize(void *table){
    extendible_hashing_sim2_finalize((extendible_hashing_dsim_instance_t *)table);
}

inline int table_insert(void *table, int64_t key, int64_t value){
    epoch_start();
    int ret = extendible_hashing_sim2_insert(table, key, value);
    epoch_done();
    return ret;
}


inline int table_remove(void *table, int64_t key){
    epoch_start();
    int ret = extendible_hashing_sim2_remove(table, key);
    epoch_done();
    return ret;
}


inline int table_lookup(void *table, int64_t key, int64_t *value){
    epoch_start();
    int ret = extendible_hashing_sim2_lookup(table, key, value);
    epoch_done();
    return (ret==SUCCESS);
}


int table_lookup_safe(void *table, int64_t key, int64_t *value){
    epoch_start();
    int ret = extendible_hashing_sim2_lookup_through_bsim(table, key, value);
    epoch_done();

    return (ret==SUCCESS);
}

int table_print(void *table, int print_level){
    epoch_start();
    extendible_hashing_print_dsim(print_level,table);
    epoch_done();
    return 0;
}

void table_thread_init(void **table){
    return;
}    

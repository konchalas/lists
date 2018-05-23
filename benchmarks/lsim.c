#include "bench_interface.h"
#include "../include/extendible_hashing_seq.h"
#include "../include/extendible_hashing_lsim.h"

void table_init(void **table, int depth, int bucket_depth){
    extendible_hashing_lsim_init((extendible_hash_table_lsim_t **)table,depth);
}


void table_finalize(void *table){
    extendible_hashing_lsim_finalize((extendible_hash_table_lsim_t *)table);
}

inline int table_insert(void *table, int64_t key, int64_t value, int tid){
    extendible_hashing_lsim_insert(table, key, value, tid);
    return 0;
}


inline int table_remove(void *table, int64_t key, int tid){
    extendible_hashing_lsim_remove(table, key, tid);
    return 0;
}


inline int table_lookup(void *table, int64_t key, int64_t *value, int tid){
    return extendible_hashing_lsim_lookup(table, key, value, tid);
}


int table_lookup_safe(void *table, int64_t key, int64_t *value, int tid){
    extendible_hashing_lsim_lookup_through_lsim(table, key, value, tid);
    return 0;
}

int table_print(void *table, int print_level){
    printf("No print function for LSIM\n");
    return -1;
}
    
void table_thread_init(void **table, int tid){
    return;
}

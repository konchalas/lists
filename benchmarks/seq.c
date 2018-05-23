#include "bench_interface.h"
#include "../include/extendible_hashing_seq.h"


void table_init(void **table, int depth, int bucket_depth){
    if(N_THREADS > 1){
        fprintf(stderr,"SEQ implemenation does not support multithreading\n");
        exit(1);
    }
    extendible_hashing_init((extendible_hash_table_t **)table,depth, depth);
}


void table_finalize(void *table){
    extendible_hashing_finalize((extendible_hash_table_t *)table);
}

inline int table_insert(void *table, int64_t key, int64_t value){
    extendible_hashing_insert(table, key, value);
    return 0;
}


inline int table_remove(void *table, int64_t key){
    return extendible_hashing_remove(table, key);
}


inline int table_lookup(void *table, int64_t key, int64_t *value){
    return extendible_hashing_lookup(table, key, value);
}


int table_lookup_safe(void *table, int64_t key, int64_t *value){
    return extendible_hashing_lookup(table, key, value);
}

int table_print(void *table, int print_level){
    printf("No print function for SEQ\n");
    return -1;
}

    
void table_thread_init(void **table){
    return;
}

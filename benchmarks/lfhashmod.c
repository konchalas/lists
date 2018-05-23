#include "bench_interface.h"
#include "../include/lfhash.h"

void table_init(void **table, int depth, int bucket_depth){
    *table = qt_hash_create(depth, 0);
}


void table_finalize(void *table){
    /* commented out because it generates a segfault */
    /*qt_hash_destroy(table);*/
}

inline int table_insert(void *table, int64_t key, int64_t value){
    return qt_hash_put(table,(void*)key,(void*)key);
}


inline int table_remove(void *table, int64_t key){
    return qt_hash_remove(table, key);
}


inline int table_lookup(void *table, int64_t key, int64_t *value){
    return qt_hash_get2(table, key, (void**)value);
}


int table_lookup_safe(void *table, int64_t key, int64_t *value){
    return qt_hash_get2(table, key,(void**)value);
}

int table_print(void *table, int print_level){
    qt_hash_print_table(table);
    return 0;
}

void table_thread_init(void **table){
    return;
}

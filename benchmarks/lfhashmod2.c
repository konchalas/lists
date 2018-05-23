#include "bench_interface.h"
#include "../include/lfhash.h"

#include "epoch.h"

void table_init(void **table, int depth, int bucket_depth){
    *table = qt_hash_create(depth, 0);
}


void table_finalize(void *table){
    /* commented out because it generates a segfault */
    /*qt_hash_destroy(table);*/
}

inline int table_insert(void *table, int64_t key, int64_t value){
    epoch_start();
    int ret =qt_hash_put(table,(void*)key,(void*)key);
    epoch_done();
    return ret;
}


inline int table_remove(void *table, int64_t key){
    epoch_start();
    int ret= qt_hash_remove(table, key);
    epoch_done();
    return ret;
}


inline int table_lookup(void *table, int64_t key, int64_t *value){
    epoch_start();
    int ret= qt_hash_get2(table, key, (void**)value);
    epoch_done();
    return ret;
}


int table_lookup_safe(void *table, int64_t key, int64_t *value){
    epoch_start();
    int ret = qt_hash_get2(table, key,(void**)value);
    epoch_done();
    return ret;
}

int table_print(void *table, int print_level){
    epoch_start();
    qt_hash_print_table(table);
    epoch_done();
    return 0;
}

void table_thread_init(void **table){
    return;
}

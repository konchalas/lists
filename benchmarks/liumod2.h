#ifndef __LIUMOD2_HPP__
#define __LIUMOD2_HPP__


#ifdef FLAG_liumod2

#include "bench_interface.h"
#include "../code_Liu_modified2/hash.hpp"
//#include "../code_Liu_modified/mm.hpp"

#include "epoch.h"


void table_init(void **table, int depth, int bucket_depth){
    //wbmm_init(N_THREADS+2);
    *table = new hashset_t();
}

void table_finalize(void *table){
    delete (hashset_t*)table;
}

inline int table_insert(void *table, int64_t key, int64_t value){
    epoch_start();
    int ret = ((hashset_t*)table)->insert(key,value);
    epoch_done();
    return ret;
}


inline int table_remove(void *table, int64_t key){
    epoch_start();
    int ret = ((hashset_t*)table)->remove(key);
    epoch_done();
    return ret;
}


inline int table_lookup(void *table, int64_t key, int64_t *value){
    epoch_start();
    int ret =  ((hashset_t*)table)->lookup(key, value);
    epoch_done();
    return ret;
}


int table_lookup_safe(void *table, int64_t key, int64_t *value){
    epoch_start();
    int ret = ((hashset_t*)table)->lookup(key, value);
    epoch_done();
    return ret;
}

int table_print(void *table, int print_level){
    epoch_start();
    std::cout << table;
    epoch_done();
    return 0;
}
    
void table_thread_init(void **table){
    //wbmm_thread_init(__thread_id__);
}

#endif /* FLAG_liumod2 */

#endif /* __LIUMOD2_HPP__ */

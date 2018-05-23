#ifndef __LIU_HPP__
#define __LIU_HPP__


#ifdef FLAG_liu

#include "bench_interface.h"
#include "../code_Liu/hash.hpp"
#include "../code_Liu/mm.hpp"


void table_init(void **table, int depth, int bucket_depth){
    wbmm_init(N_THREADS+2);
    *table = new hashset_t();
}


void table_finalize(void *table){
    delete (hashset_t*)table;
}

inline int table_insert(void *table, int64_t key, int64_t value){
    return ((hashset_t*)table)->insert(key);
}


inline int table_remove(void *table, int64_t key){
    return ((hashset_t*)table)->remove(key);
}


inline int table_lookup(void *table, int64_t key, int64_t *value){
    return ((hashset_t*)table)->contains(key);
}


int table_lookup_safe(void *table, int64_t key, int64_t *value){
    return ((hashset_t*)table)->contains(key);
}

int table_print(void *table, int print_level){
    std::cout << table;
    return 0;
}
    
void table_thread_init(void **table){
    wbmm_thread_init(__thread_id__);
}

#endif /* FLAG_liu */

#endif /* __LIU_HPP__ */

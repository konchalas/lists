#ifndef __BENCH_INTERFACE_H__
#define __BENCH_INTERFACE_H__

#include <stdint.h>

#define THREAD_INIT(_tid) (__thread_id__ = _tid)

void table_init(void **table, int depth, int bucket_depth);

void table_finalize(void *table);

int table_insert(void *table, int64_t key, int64_t value);

int table_remove(void *table, int64_t key);

int table_lookup(void *table, int64_t key, int64_t *value);

int table_lookup_safe(void *table, int64_t key, int64_t *value);

int table_print(void *table, int print_level);
    
void table_thread_init(void **table);


#endif /* __BENCH_INTERFACE_H__ */

#ifndef _EXTENDIBLE_HASHING_CLH_H_
#define _EXTENDIBLE_HASHING_CLH_H_

#include <stdint.h>
#include "extendible_hashing_seq.h"
#include "clh.h"

typedef struct bucket_clh{
    unsigned int prefix;
    int depth;
    int nb_items;
    item_t items[MAX_ITEMS];
    CLHLockStruct *lock;
} bucket_clh_t;

typedef struct extendible_hash_table_clh{
    int depth;
    bucket_clh_t **directory;
} extendible_hash_table_clh_t;


void extendible_hashing_clh_init(extendible_hash_table_clh_t **table, int depth);
void extendible_hashing_clh_finalize(extendible_hash_table_clh_t *table);
void extendible_hashing_clh_insert(extendible_hash_table_clh_t *table, int64_t key, int64_t value);
int extendible_hashing_clh_lookup(extendible_hash_table_clh_t *table, int64_t key, int64_t *value);

int extendible_hashing_clh_remove(extendible_hash_table_clh_t *table, int64_t key);


#endif

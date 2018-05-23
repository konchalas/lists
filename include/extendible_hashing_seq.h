#ifndef _EXTENDIBLE_HASHING_SEQ_H_
#define _EXTENDIBLE_HASHING_SEQ_H_

#include <stdint.h>
#include "primitives.h"

#ifndef MAX_ITEMS
#define MAX_ITEMS 8
#endif

#define JUNK -3637

typedef struct item{
    /*int64_t hkey;*/
    int64_t key;
    int64_t val;
#if defined(VERBOSE) || defined(ADEBUG)    
    int64_t seqnum;
    int tid;
    int op;
#endif
} item_t;

/*typedef struct item_pointer{
    int64_t hkey;      // It is important, this field to be 64 bits! 
    int64_t key;
    int64_t val;
    } item_pointer_t;*/

typedef struct bucket{
    item_t items[MAX_ITEMS];
    uint32_t prefix;
    int32_t depth;
    int32_t nb_items;
    int32_t pad;
} bucket_t;

typedef struct extendible_hash_table{
    int depth;
    bucket_t **directory;
} extendible_hash_table_t;


void extendible_hashing_init(extendible_hash_table_t **table, int depth, int buck_depth);
void extendible_hashing_finalize(extendible_hash_table_t *table);
void extendible_hashing_insert(extendible_hash_table_t *table, int64_t key, int64_t value);
int extendible_hashing_lookup(extendible_hash_table_t *table, int64_t key, int64_t *value);
int extendible_hashing_remove(extendible_hash_table_t *table, int64_t key);


#define HASH(key_) ((unsigned int)(key_))
#define PREFIX(key_, size_) ((key_) & (~((~0U)<<(size_))))




#endif

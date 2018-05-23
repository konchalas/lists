#ifndef _LFHASH_H_
#define _LFHASH_H_

#include <stdlib.h> /* for malloc/free/etc */
#include <stdio.h>
#include <unistd.h> /* for getpagesize() */
#include <assert.h>
#include <inttypes.h> /* for PRI___ macros (printf) */
#include <limits.h>
#include <pthread.h>
#include "pool.h"

#ifdef MAX_ITEMS
#define MAX_LOAD     MAX_ITEMS
#else
#define MAX_LOAD     16
#endif



//#define USE_HASHWORD 1


#define _MULTI_THREADED

/* external types */
typedef const void *qt__key_t;
typedef struct qt_hash_s *qt_hash;
typedef void (*qt_hash_callback_fn)(const qt__key_t, void *, void *);
typedef void (*qt_hash_deallocator_fn)(void *);

/* internal types */
typedef uint64_t _key_t;
typedef uint64_t so_key_t;
typedef uintptr_t marked_ptr_t;

#define MARK_OF(x)           ((x) & 1)
#define PTR_MASK(x)          ((x) & ~(marked_ptr_t)1)
#define PTR_OF(x)            ((hash_entry *)PTR_MASK(x))
#define CONSTRUCT(mark, ptr) (PTR_MASK((uintptr_t)ptr) | (mark))
#define UNINITIALIZED ((marked_ptr_t)0)

/* These are GCC builtins; other compilers may require inline assembly */
#define CAS(ADDR, OLDV, NEWV) __sync_val_compare_and_swap((ADDR), (OLDV), (NEWV))
#define INCR(ADDR, INCVAL) __sync_fetch_and_add((ADDR), (INCVAL))

//size_t hard_max_buckets = (1 << 16);
#define hard_max_buckets ((size_t) 1 << 28)     // This is the maximum hash-table size

typedef struct hash_entry_s {
    so_key_t     key;
    void        *value;
    marked_ptr_t next;
} hash_entry;

struct qt_hash_s {
    marked_ptr_t   *B;  // Buckets
    volatile size_t count;
    volatile size_t size;
#ifdef LOCAL_POOL
    PoolStruct lfhash_item_pool[N_THREADS] CACHE_ALIGN;
#endif
};

qt_hash H;

int qt_hash_put(qt_hash  h, qt__key_t key, void *value);
void *qt_hash_get(qt_hash h, const qt__key_t key);
int qt_hash_remove(qt_hash h, const qt__key_t key);
qt_hash qt_hash_create(int depth, int needSync);
void qt_hash_destroy(qt_hash h);
void qt_hash_destroy_deallocate(qt_hash h, qt_hash_deallocator_fn f);
size_t qt_hash_count(qt_hash h);
void qt_hash_callback(qt_hash h, qt_hash_callback_fn f, void *arg);
void print_ent(const qt__key_t k, void *v, void *a);

void qt_hash_print_table(qt_hash h);


int qt_hash_get2(qt_hash h, const qt__key_t key, void **vvv);

#endif

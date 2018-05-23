#ifndef __MEM_INTERFACE_H__
#define __MEM_INTERFACE_H__

#include <stdlib.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


#ifdef HEAPSIZE
#define THREAD_CACHE     HEAPSIZE
#else
#define THREAD_CACHE   2
#endif
    
typedef union mem_reference{
    struct refecence{
    uint64_t addr : 48;
    uint64_t seq : 15;
    } ref;
    void *ptr;
} mem_reference_t;


typedef struct mem_reference_pool{
    mem_reference_t *pool;
    int first;
    int last;
    int size;   /* size of the pool */
    int count;  /* nb of available items */
    int64_t pad[13];
} mem_reference_pool_t;
    
typedef struct mem_alloc_tracker{
    void **set;
    int count;
    int size;
    int active;
    int64_t pad[13];
}mem_alloc_tracker_t;

    

#define LAST_BIT (1ULL<<63)

#define REF_PTR(_x) ((void*)((long long unsigned)(_x) & (((~0ULL)>>16)|LAST_BIT)))

void pool_init(size_t obj_size);
void pool_thread_init(void);

void* pool_alloc(void);
void* pool_alloc_zeroed(void);
void pool_free(void *ptr);
void pool_recycle(void *ptr);

void pool_free_cautious(void *ptr);

void pool_resize(void);

/* common to all implementations */
void* system_alloc(size_t obj_size);
void system_free(void *ptr);
void system_free_with_gc(void* ptr);
    
void mem_alloc_tracking_init(void);
void mem_alloc_tracking_start(void);
void mem_alloc_tracking_stop(void);
void mem_alloc_tracking_free(int doit);


#ifdef __cplusplus
}
#endif

    
#endif

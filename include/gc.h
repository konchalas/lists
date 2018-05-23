#ifndef __GC_H__
#define __GC_H__

#include "mem_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

void gc_init(void);
void gc_thread_init(void);
    
/* 
 * pool is where to insert the reclaimed reference
 * to_free tells whether the references should be freed in case pool
 * == NULL 
*/
void gc_collect(void* addr, mem_reference_pool_t* pool);

//void gc_free(void* addr, int limit);

/* same as collect except that addr is always to be released to the
 * system */
void gc_collect_basic(void* addr);

/* same as collect_basic but for mem that might be added to pool */
void gc_collect_basic_pool(void* addr, mem_reference_pool_t* pool);
    
#ifdef __cplusplus
}
#endif

    
#endif /* __GC_H__ */

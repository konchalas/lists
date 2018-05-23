#ifndef _POOL_H_
#define _POOL_H_

#include <primitives.h>

typedef struct HalfPoolStruct {
    char *heap;
    int index;
    int obj_size;
} HalfPoolStruct;


typedef struct PoolStruct {
    char *heap;
    int index;
    int obj_size;
    char pad[PAD_CACHE(sizeof(HalfPoolStruct))];
} PoolStruct;

/*inline*/ void init_pool(PoolStruct *pool, int obj_size);
/*inline*/ void *alloc_obj(PoolStruct *pool);
/*inline*/ void free_last_obj(PoolStruct *pool, void *obj);
/*inline*/ void rollback(PoolStruct *pool, int num_objs);

void free_obj(PoolStruct *pool, void *obj);


#endif

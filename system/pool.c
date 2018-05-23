#include <pool.h>

static const int PSIZE = 2048;

inline void init_pool(PoolStruct *pool, int obj_size) {
    pool->heap = getAlignedMemory(CACHE_LINE_SIZE, PSIZE * obj_size);
    pool->obj_size = obj_size;
    pool->index = 0;
}

inline void *alloc_obj(PoolStruct *pool) {
    int offset;

    if (pool->index == PSIZE-1) {
        int size = pool->obj_size;
        init_pool(pool, size);
    }

    offset = pool->index;
    pool->index += 1;
    return (void *)(pool->heap + (offset * pool->obj_size));
}

inline void free_last_obj(PoolStruct *pool, void *obj) {
    if (pool->index > 0) {
        pool->index -= 1;
    }
}

inline void free_obj(PoolStruct *pool, void *obj)
{
}



inline void rollback(PoolStruct *pool, int num_objs) {
    if (pool->index - num_objs >= 0)
        pool->index -= num_objs;
    else
        pool->index = 0;
}

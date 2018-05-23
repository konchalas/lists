#ifndef __EPOCH_H__
#define __EPOCH_H__

#include "types.h"


#ifdef __cplusplus
extern "C" {
#endif

#define N_THREADS 64

    extern volatile int_aligned64_t epoch[N_THREADS];

    
    void epoch_init(void);
    void epoch_start(void);
    void epoch_done(void);
    void epoch_collect(int64_t* vect);

    


#ifdef __cplusplus
}
#endif



#endif

#ifndef _FASTRAND_H_
#define _FASTRAND_H_

#include <math.h>
#include <stdint.h>
#include <limits.h>

#define SIM_RAND_MAX         32768



// This random generators are implementing 
// by following POSIX.1-2001 directives.
// ---------------------------------------


#ifdef __cplusplus
extern "C" {
#endif

/*inline*/ long fastRandom(void);
/*inline*/ uint32_t fastRandom32(void);
/*inline*/ void fastRandomSetSeed(uint32_t seed);
/*inline*/ long fastRandomRange(long low, long high);
/*inline*/ uint32_t fastRandomRange32(uint32_t low, uint32_t high);

#ifdef __cplusplus
}
#endif

    
#endif

#ifndef MYFLPT_H
#define MYFLPT_H

#include <stdint.h>

#define MANT_BITS 23
#define MANT_MASK ((1<<MANT_BITS)-1)
#define TO_MANT_FIELD(x) ((x) << EXP_BITS)

#define EXP_BITS (32 - 1 - MANT_BITS)
#define EXP_MASK ((1 << EXP_BITS)-1)
#define EXP_OFFSET ((1 << (EXP_BITS-1))-1)

#define INFINITY EXP_MASK

#define SIGN(x) (x & 0x80000000)

typedef uint32_t myflp_t;

//#define FLOAT2MYFLP(x) (*((myflp_t*)&x))
//#define MYFLP2FLOAT(x) (*((float*)&x))

myflp_t fp_add(myflp_t a, myflp_t b);
myflp_t fp_mul(myflp_t a, myflp_t b);
myflp_t fp_div(myflp_t a, myflp_t b);
myflp_t int2myflp(int32_t x);

static inline float myflp2float(myflp_t f) {
    uint32_t x = *((uint32_t*)&f);
    uint32_t y = (SIGN(x)) | ((x >> EXP_BITS) & MANT_MASK) | ((x & EXP_MASK) << MANT_BITS);
    return *((float*)&y);
}

static inline myflp_t float2myflp(float f) {
    uint32_t x = *((uint32_t*)&f);
    return (SIGN(x)) | ((x & MANT_MASK) << EXP_BITS) | ((x >> MANT_BITS) & EXP_MASK);
}

#define fp_sub(a,b) (fp_add(a,(b^0x80000000)))

#endif // MYFLPT_H
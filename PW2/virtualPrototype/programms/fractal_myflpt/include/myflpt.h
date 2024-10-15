#ifndef MYFLPT_H
#define MYFLPT_H

#include <stdint.h>

#define MANT_BITS 23
#define EXP_BITS (32 - 1 - MANT_BITS)
#define EXP_OFFSET ((1 << EXP_BITS - 1))

#define INFINITY (((1 << EXP_BITS)-1) << MANT_BITS)

typedef uint32_t myflp_t;

#define FLOAT2MYFLP(x) (*((myflp_t*)&x))
#define MYFLP2FLOAT(x) (*((float*)&x))

myflp_t fp_add(myflp_t a, myflp_t b);
myflp_t fp_mul(myflp_t a, myflp_t b);
myflp_t fp_div(myflp_t a, myflp_t b);
myflp_t int2myflp(int32_t x);

#define fp_sub(a,b) (fp_add(a,(b^0x80000000)))

#endif // MYFLPT_H
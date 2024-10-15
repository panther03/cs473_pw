#include "myflpt.h"
#include "cfg.h"

//#define FP_DEBUG

#ifdef FP_DEBUG
#include <stdio.h>
#define FP_DBG_PRINT printf
#else
#define FP_DBG_PRINT
#endif

#define MAX(a,b) ((a<b)?b:a)

static inline void renormalize(uint32_t *mant_res, uint32_t *exp) {
    // Overflow case
    if (*mant_res >> (MANT_BITS+1)) {
        *exp += (1 << MANT_BITS);
        *mant_res >>= 1;
        FP_DBG_PRINT("(Corrected Overflow) S = %x\n", *mant_res);
    } else {
        // Count leading zeros
        // Should always terminate because:
        // either the whole mantissa was 0, in which case we returned before
        // or there is a 1 at or after bit 22
        // can't be one before because we truncated
        while ((*mant_res & (1 << (MANT_BITS))) == 0) {
            *exp -= (1 << MANT_BITS);
            *mant_res <<= 1;
        }
    }
    
    *mant_res = *mant_res & ((1 << MANT_BITS)-1);
}

myflp_t int2myflp(int32_t x) {
    if (x == 0) return 0;

    uint32_t sign = x & 0x80000000;
    if (sign) {
        x = -x;
    }
    // replace magic number 127
    int exp = 127 + 30;
    while ((x & 0x40000000) == 0) {
        x <<= 1;
        exp -= 1;
    }
    // todo replace magic number 7
    uint32_t mant = (((uint32_t)x) >> 7) & ((1 << MANT_BITS)-1);

    return sign | (exp << MANT_BITS) | mant;
}

myflp_t fp_add(myflp_t a, myflp_t b) {
    // Form mantissas
    const uint32_t mant_ghost = (1 << MANT_BITS);
    int32_t mant_a = mant_ghost | (a & ((1<<MANT_BITS)-1));
    int32_t mant_b = mant_ghost | (b & ((1<<MANT_BITS)-1));
    FP_DBG_PRINT("(Start Mantissas) A: %x | B: %x\n", mant_a, mant_b);
    // Extract exponents 
    uint32_t exp_a = a & (((1<<EXP_BITS)-1)<<MANT_BITS);
    uint32_t exp_b = b & (((1<<EXP_BITS)-1)<<MANT_BITS);
    FP_DBG_PRINT("(Exponents) A: %x | B: %x\n", exp_a, exp_b);

    // Pick larger exponent and de-normalize mantissa to larger exponent
    uint32_t exp;
    if (exp_a < exp_b) {
        exp = exp_b;
        int shamt = ((exp_b-exp_a) >> MANT_BITS);
        if (shamt < 31) {
            mant_a >>= shamt;
        } else {
            mant_a = 0;
        }
    } else {
        exp = exp_a;
        int shamt = ((exp_a-exp_b) >> MANT_BITS);
        if (shamt < 31) {
            mant_b >>= shamt;
        } else {
            mant_b = 0;
        }
    }
    FP_DBG_PRINT("(Denorm Mantissas) A: %x | B: %x\n", mant_a, mant_b);

    // Grab sign of A,B and adjust mantissas accordingly
    uint32_t a_sign = ((~a) >> 31);
    uint32_t b_sign = ((~b) >> 31);
    mant_a ^= (a_sign - 1);
    mant_a += (a >> 31);
    mant_b ^= (b_sign - 1);
    mant_b += (b >> 31);
    FP_DBG_PRINT("(Signed Mantissas) A: %x | B: %x\n", mant_a, mant_b);

    uint32_t mant_res = mant_a + mant_b;
    FP_DBG_PRINT("(Raw Result) S = %x\n", mant_res);

    // Convert to unsigned, store result in sign bit
    uint32_t sign_res = mant_res & 0x80000000;
    if (sign_res) {
        mant_res = 0 - mant_res;
    }

    mant_res = mant_res & ((1 << ((MANT_BITS+2)))-1);
    FP_DBG_PRINT("(Truncated Result) S = %x\n", mant_res);

    // 0 mantissa? quit early
    // we also return for 0 exponent as a shortcut
    if (mant_res == 0 || exp == 0) { return 0;}

    renormalize(&mant_res, &exp);

    return sign_res | exp | mant_res;
}

myflp_t fp_mul(myflp_t a, myflp_t b) {
    if (a == 0 || b == 0) { return 0; }
    // Form mantissas
    const uint32_t mant_ghost = (1 << MANT_BITS);
    int32_t mant_a = mant_ghost | (a & ((1<<MANT_BITS)-1));
    int32_t mant_b = mant_ghost | (b & ((1<<MANT_BITS)-1));
    FP_DBG_PRINT("(Start Mantissas) A: %x | B: %x\n", mant_a, mant_b);

    // Grab sign of A,B and adjust mantissas accordingly
    uint32_t a_sign = ((~a) >> 31);
    uint32_t b_sign = ((~b) >> 31);
    mant_a ^= (a_sign - 1);
    mant_a += (a >> 31);
    mant_b ^= (b_sign - 1);
    mant_b += (b >> 31);
    FP_DBG_PRINT("(Signed Mantissas) A: %x | B: %x\n", mant_a, mant_b);

    // Extract exponents 
    uint32_t exp_a = a & (((1<<EXP_BITS)-1)<<MANT_BITS);
    uint32_t exp_b = b & (((1<<EXP_BITS)-1)<<MANT_BITS);
    FP_DBG_PRINT("(Exponents) A: %x | B: %x\n", exp_a, exp_b);

    // Resulting exponent is sum, accounting for the default offset
    const int exp_offset = ((1<<(EXP_BITS-1))-1);
    int32_t exp = exp_a + exp_b - (exp_offset << MANT_BITS);
    FP_DBG_PRINT("(Exponent Sum) S: %x\n", exp);

    // Multiply mantissas    
#ifdef FP_MUL64
    int64_t mant_res_temp  = ((int64_t)(mant_a) * (int64_t)(mant_b)) >> MANT_BITS;
#else
    // Mantissas are Q1.23 fixed point values
    // We can do a 32-bit multiplication if we first convert them to 16-bit values,
    // to get a signed 32-bit result.
    // We have to make sure to shift one extra because we need to preserve the sign bit.
    // So the operands have the range of a 15-bit unsigned value.
    uint32_t mant_a_trunc = (mant_a >> ((MANT_BITS+1)-15));
    uint32_t mant_b_trunc = (mant_b >> ((MANT_BITS+1)-15));
    FP_DBG_PRINT("(Operands) A: %x | B: %x\n", mant_a_trunc, mant_b_trunc);
    // First whole part (1) of fixed point in operand is at bit 14, should be at bit (MANT_BITS)
    // Make sure to convert to signed first because we want arithmetic shift!
    int32_t mant_res_temp = (int32_t)((mant_a_trunc * mant_b_trunc)) >> (14*2-MANT_BITS);
#endif
    uint32_t mant_res = (uint32_t)mant_res_temp;
    FP_DBG_PRINT("(Raw Result) P = %x\n", mant_res);
    
    // Convert to unsigned, store result in sign bit
    uint32_t sign_res = mant_res & 0x80000000;
    if (sign_res) {
        mant_res = 0 - mant_res;
    }

    mant_res = mant_res & ((1 << ((MANT_BITS+2)))-1);
    FP_DBG_PRINT("(Truncated Result) S = %x\n", mant_res);

    // 0 mantissa? quit early
    // we also return for 0 exponent as a shortcut
    if (mant_res == 0 || exp == 0) { return 0;}

    renormalize(&mant_res, &exp);

    return sign_res | exp | mant_res;
}

myflp_t fp_div(myflp_t a, myflp_t b) {
    if (a == 0) { return 0; }
    if (b == 0) { return INFINITY; }
    // Form mantissas
    const uint32_t mant_ghost = (1 << MANT_BITS);
    int32_t mant_a = mant_ghost | (a & ((1<<MANT_BITS)-1));
    int32_t mant_b = mant_ghost | (b & ((1<<MANT_BITS)-1));
    FP_DBG_PRINT("(Start Mantissas) A: %x | B: %x\n", mant_a, mant_b);

    // Grab sign of A,B and adjust mantissas accordingly
    uint32_t a_sign = ((~a) >> 31);
    uint32_t b_sign = ((~b) >> 31);
    mant_a ^= (a_sign - 1);
    mant_a += (a >> 31);
    mant_b ^= (b_sign - 1);
    mant_b += (b >> 31);
    FP_DBG_PRINT("(Signed Mantissas) A: %x | B: %x\n", mant_a, mant_b);

    // Extract exponents 
    uint32_t exp_a = a & (((1<<EXP_BITS)-1)<<MANT_BITS);
    uint32_t exp_b = b & (((1<<EXP_BITS)-1)<<MANT_BITS);
    FP_DBG_PRINT("(Exponents) A: %x | B: %x\n", exp_a, exp_b);

    // Resulting exponent is diff, accounting for the default offset
    const int exp_offset = ((1<<(EXP_BITS-1))-1);
    int32_t exp = exp_a - exp_b + (exp_offset << MANT_BITS);
    FP_DBG_PRINT("(Exponent Diff) S: %x\n", exp);

    // Divide mantissas    
    int64_t mant_res_temp  = (((int64_t)(mant_a)) << MANT_BITS) / (int64_t)(mant_b);
    uint32_t mant_res = (uint32_t)mant_res_temp;
    FP_DBG_PRINT("(Raw Result) P = %x\n", mant_res);
    
    // Convert to unsigned, store result in sign bit
    uint32_t sign_res = mant_res & 0x80000000;
    if (sign_res) {
        mant_res = 0 - mant_res;
    }

    mant_res = mant_res & ((1 << ((MANT_BITS+2)))-1);
    FP_DBG_PRINT("(Truncated Result) S = %x\n", mant_res);

    // 0 mantissa? quit early
    // we also return for 0 exponent as a shortcut
    if (mant_res == 0 || exp == 0) { return 0;}

    renormalize(&mant_res, &exp);

    return sign_res | exp | mant_res;
}
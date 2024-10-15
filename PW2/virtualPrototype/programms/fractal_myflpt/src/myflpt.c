#include "../include/myflpt.h"

#define MAX(a,b) ((a<b)?b:a)

myflp_t fp_add(myflp_t a, myflp_t b) {
    const uint32_t mant_ghost = (1 << MANT_BITS);
    int32_t mant_a = mant_ghost | (a & ((1<<MANT_BITS)-1));
    int32_t mant_b = mant_ghost | (b & ((1<<MANT_BITS)-1));
    //printf("%x\n", mant_a);
    //printf("%x\n", mant_b);
    uint32_t exp_a = a & (((1<<EXP_BITS)-1)<<MANT_BITS);
    uint32_t exp_b = b & (((1<<EXP_BITS)-1)<<MANT_BITS);
    //printf("%x,%x\n",exp_a,exp_b);
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
    //printf("%x\n", mant_a);
    //printf("%x\n", mant_b);
    uint32_t a_sign = ((~a) >> 31);
    uint32_t b_sign = ((~b) >> 31);
    mant_a ^= (a_sign - 1);
    mant_a += (a >> 31);
    mant_b ^= (b_sign - 1);
    mant_b += (b >> 31);
    //printf("%x\n", mant_a);
    //printf("%x\n", mant_b);
    uint32_t mant_res = mant_a + mant_b;
    //printf("%x\n", mant_res);
    uint32_t sign_res = mant_res & 0x80000000;
    uint32_t mant_sign = (mant_res >> (MANT_BITS+1)) & 0x1;
    if (exp == 0 || mant_res == 0) { return 0;}
    if ((a_sign == b_sign) && (mant_sign == a_sign)) {
        // overflow
        // todo probably shouldnt do this if input is 0
        //printf("ofl\n");
        exp += (1 << MANT_BITS);
        mant_res >>= 1;
    }
    if (sign_res) {
        mant_res = 0 - mant_res;
    }
    //printf("%x\n",exp);
    
    // count leading zeros
    //
    while ((mant_res & (1 << (MANT_BITS))) == 0) {
        exp -= (1 << MANT_BITS);
        mant_res <<= 1;
        if ((mant_res & ((1 << MANT_BITS)-1)) == 0) { break;}
    }
    mant_res = mant_res & ((1 << MANT_BITS)-1);
    return sign_res | exp | mant_res;
}

myflp_t fp_mul(myflp_t a, myflp_t b) {
    if (a == 0 || b == 0) { return 0; }
    const uint32_t mant_ghost = (1 << MANT_BITS);
    int32_t mant_a = mant_ghost | (a & ((1<<MANT_BITS)-1));
    int32_t mant_b = mant_ghost | (b & ((1<<MANT_BITS)-1));
    uint32_t a_sign = ((~a) >> 31);
    uint32_t b_sign = ((~b) >> 31);
    //printf("%x\n", mant_a);
    //printf("%x\n", mant_b);
    mant_a ^= (a_sign - 1);
    mant_a += (a >> 31);
    mant_b ^= (b_sign - 1);
    mant_b += (b >> 31);
    //printf("%x\n", mant_a);
    //printf("%x\n", mant_b);
    //uint32_t op_a = (mant_a >> ((MANT_BITS>>1)-2));
    //uint32_t op_b = (mant_b >> ((MANT_BITS>>1)-2));
    //int32_t res = (int32_t)((op_a * op_b)) >> 5;
    int64_t res = ((int64_t)(mant_a) * (int64_t)(mant_b)) >> 23;
    //printf("%x\n", op_a);
    //printf("%x\n", op_b);
    //printf("%x\n", res);
    uint32_t mant_res = (uint32_t)res;
    //printf("%x;%x;%x\n", mant_a,mant_b,mant_res);
    uint32_t sign_res = mant_res & 0x80000000;
    uint32_t exp_a = ((a >> MANT_BITS) & ((1<<EXP_BITS)-1)) - ((1<<(EXP_BITS-1))-1);
    uint32_t exp_b = ((b >> MANT_BITS) & ((1<<EXP_BITS)-1)) - ((1<<(EXP_BITS-1))-1);
    int32_t exp = exp_a + exp_b;
    if (sign_res) {
        mant_res = 0 - mant_res;
    }
    uint32_t ofl = (mant_res >> (MANT_BITS+1)) & 0x1;
    if (ofl) {
        exp += 1;
        mant_res >>= 1;
    }
    
    //printf("%x;%x;%x;%x;%x\n", mant_a,mant_b,mant_res, exp_a,exp_b);
    // count leading zeros
    //
    while (((mant_res & (1 << (MANT_BITS))) == 0)) {
        exp -= 1;
        mant_res <<= 1;
        if ((mant_res & ((1 << MANT_BITS)-1)) == 0) {break; }
    }
    mant_res = mant_res & ((1 << MANT_BITS)-1);
    return sign_res | ((exp + ((1<<(EXP_BITS-1))-1)) << MANT_BITS) | mant_res;
}
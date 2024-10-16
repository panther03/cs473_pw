#include <fractal_conv.h>

// Implement a basic Q7.25 (32bit) Fixed Point Operator
// Here, scaling factor (S) = 2^28
// Inspiration: https://en.wikipedia.org/wiki/Fixed-point_arithmetic#Binary_fixed-point_multiplication

// Float to FXP
fxp32_t floatToFXP(float in) {
  float out = in * 0x2000000; // Same as * 2^27
  return (fxp32_t)(out); // Casting rounds down + adding 0.5 results in correct rounding
}
// (unsigned) int to FXP
fxp32_t intToFXP(int in) {
  return (fxp32_t)(in << 25); 
}

// Assuming both have same Qm.f format, the addition is straightforward
fxp32_t fxpAdd(fxp32_t op1, fxp32_t op2) {
  return op1 + op2;
}
fxp32_t fxpSub(fxp32_t op1, fxp32_t op2) {
  return op1 - op2;
}

// Casting to 64bit in order to avoid overflow
fxp32_t fxpMul(fxp32_t op1, fxp32_t op2) {
  int64_t res = ((int64_t) op1) * ((int64_t) op2);
  return (fxp32_t)( res >> 25);  // Rounding and shifting back
}

// fxp32_t fxpMul(fxp32_t op1, fxp32_t op2) {
//   int32_t res = ((int32_t) (op1>>13)) * ((int32_t) (op2>>13));
//   return (fxp32_t)(( res << 1));  // Rounding and shifting back
// }



// Converting back to floating point
float fxpToFloat(fxp32_t in) {
  return ((float)in) / 0x2000000;
}
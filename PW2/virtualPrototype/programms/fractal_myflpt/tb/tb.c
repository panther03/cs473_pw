#include "../include/myflpt.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define PANIC_THRESHOLD 1000
// #define MUTE

int err_cnt = 0;

float fp_verif(float x, float y, float exp, uint32_t act, char op) {
    uint32_t exp_d = *((uint32_t*)&exp);
    uint32_t x_d = *((uint32_t*)&x);
    uint32_t y_d = *((uint32_t*)&y);
    float act_f = *((float*)&act);
    double err = (double)(act_f) - (double)(exp);
    err = err / ((double)(exp)) * 100.0;
    if (exp_d != act) {
        err_cnt++;
#ifndef MUTE
          printf("Disagree: %f %c %f = %f, expected %f (%f%% err)\n"
                "     Hex: %08x %c %08x = %08x, expected %08x\n\n", x, op, y, act_f, exp, err, x_d, op, y_d, act, exp_d);
          if (err_cnt > PANIC_THRESHOLD) exit(1);
#endif
    }
    return act_f;
}

#define FP_DIV_V(x,y) fp_verif(x,y,x/y, fp_div(FLOAT2MYFLP(x),FLOAT2MYFLP(y)), '/');
#define FP_MUL_V(x,y) fp_verif(x,y,x*y, fp_mul(FLOAT2MYFLP(x),FLOAT2MYFLP(y)), '*');
#define FP_SUB_V(x,y) fp_verif(x,y,x-y, fp_sub(FLOAT2MYFLP(x),FLOAT2MYFLP(y)), '-');
#define FP_ADD_V(x,y) fp_verif(x,y,x+y, fp_add(FLOAT2MYFLP(x),FLOAT2MYFLP(y)), '+');

uint16_t myflp_mandlebrot_test(float cx, float cy, uint16_t n_max) {
  uint16_t n = 0;
  float x = cx;
  float y = cy;
  float two = 2.0;
  float xx, yy, two_xy;
  float xxyy;
  do {
    xx = FP_MUL_V(x,x);
    yy = FP_MUL_V(y,y);
    float xy = FP_MUL_V(x,y);
    two_xy = FP_MUL_V(xy,two);

    float xxmyy = FP_SUB_V(xx,yy);

    x = FP_ADD_V(xxmyy, cx);
    y = FP_ADD_V(two_xy, cy);
    xxyy = FP_ADD_V(xx,yy);
    ++n;
  } while ((xxyy < 4) && (n < n_max));
  return n;
}

uint16_t calc_mandelbrot_point_soft(myflp_t cx, myflp_t cy, uint16_t n_max);

uint16_t mandlebrot_point_ref(float cx, float cy, uint16_t n_max) {
  uint16_t n = 0;
  float x = cx;
  float y = cy;
  float xx, yy, two_xy;
  float xxyy;
  do {
    xx = x*x;
    yy = y*y;
    two_xy = x * y * 2.0;

    x = xx-yy + cx;
    y = two_xy + cy;
    xxyy = xx + yy;
    ++n;
  } while ((xxyy < 4) && (n < n_max));
  return n;
}

void test_mandlebrot() {
    float cx_0 = -2.0;
    float cy = -1.5;
    int DIM = 512;
    float delta = 3.0 / DIM;
    int m_err = 0;
    int worst = 0;
    for (int k = 0; k < DIM; ++k) {
        float cx = cx_0;
        for(int i = 0; i < DIM; ++i) {
            uint16_t n1 = calc_mandelbrot_point_soft(FLOAT2MYFLP(cx), FLOAT2MYFLP(cy), 64);
            uint16_t n2 = mandlebrot_point_ref(cx, cy, 64);
            if (abs(n1 - n2) > 5) {
                printf("(%f,%f) Diff: %d\n", cx, cy, n1-n2);
                m_err++;
                if (abs(n1-n2) > worst) {
                    worst = abs(n1-n2);
                }
            }
            cx += delta;
        }
        cy += delta;
    }
    printf("Total error pixels: %d (out of %d) (worst %d)\n", m_err, DIM*DIM, worst);
}

int main() {
    // Test simple multiplication
    float x = 3;
    float y = -5;
    float z = FP_MUL_V(x,y);
    if (err_cnt) {return 1;}

    // Test simple addition
    x = 1.0;
    y = 0.00025;
    z = FP_ADD_V(x,y);
    if (err_cnt) {return 1;}

    // Test conversion
    int c = 512;
    myflp_t cf = int2myflp(c);
    float expected = (float)c;
    myflp_t expected_d = FLOAT2MYFLP(expected);
    if (cf != expected_d) {
      float cf_f = MYFLP2FLOAT(cf);
      printf("Disagree: convert %d => %f, expected %f \n"
             "     Hex: convert %d => %08x, expected %08x\n\n",
            c,cf_f,expected,c,cf,expected_d);
      return 1;
    }

    // Test simple division
    x = 3.0;
    y = 512;
    z = FP_DIV_V(x,y);
    if (err_cnt) {return 1;}

    // Test single mandlebrot point
    x = 0.039062;
    y = -0.814453;    
    myflp_mandlebrot_test(x,y,64);

    // Compare full mandlebrot
    test_mandlebrot();
    return 0;
}

#include "../include/myflpt.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ERR_THRESHOLD 0.01
#define PANIC_THRESHOLD 1000
#define MUTE 1

int err = 0;

float fp_verif(float x, float y, float exp, uint32_t act, char op) {
    uint32_t exp_d = *((uint32_t*)&exp);
    uint32_t x_d = *((uint32_t*)&x);
    uint32_t y_d = *((uint32_t*)&y);
    float act_f = *((float*)&act);
    double err = (double)(act_f) - (double)(exp);
    err = err / ((double)(exp)) * 100.0;
    if (exp_d != act && !MUTE) {
        printf("Disagree: %f %c %f = %f, expected %f (%f%% err)\n"
               "     Hex: %08x %c %08x = %08x, expected %08x\n\n", x, op, y, act_f, exp, err, x_d, op, y_d, act, exp_d);
        err++;
        if (err > PANIC_THRESHOLD) exit(1);
    }
    return act_f;
}

#define FP_MUL_V(x,y) fp_verif(x,y,x*y, fp_mul(*((myflp_t*)&x),*((myflp_t*)&y)), '*');
#define FP_SUB_V(x,y) fp_verif(x,y,x-y, fp_sub(*((myflp_t*)&x),*((myflp_t*)&y)), '-');
#define FP_ADD_V(x,y) fp_verif(x,y,x+y, fp_add(*((myflp_t*)&x),*((myflp_t*)&y)), '+');

uint16_t mandlebrot_point_myflp(float cx, float cy, uint16_t n_max) {
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

uint16_t calc_mandelbrot_point_soft(float cx, float cy, uint16_t n_max) {
  myflp_t x = *((myflp_t*)&cx);
  myflp_t y = *((myflp_t*)&cy);
  float two_f = 2.0;
  myflp_t two = *((myflp_t*)&two_f);
  uint16_t n = 0;
  myflp_t xx, yy, two_xy;
  float xxyy;
  do {
    xx = fp_mul(x,x);
    yy = fp_mul(y,y);
    two_xy = fp_mul(fp_mul(x,y),two);

    myflp_t xxmyy = fp_sub(xx,yy);
    x = fp_add(xxmyy,*((myflp_t*)&cx));
    y = fp_add(two_xy,*((myflp_t*)&cy));
    myflp_t xxyy_m = fp_add(xx,yy);
    xxyy = *((float*)&xxyy_m);
    ++n;
  } while ((xxyy < 4) && (n < n_max));
  return n;
}

uint16_t mandlebrot_point(float cx, float cy, uint16_t n_max) {
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
            uint16_t n1 = calc_mandelbrot_point_soft(cx, cy, 64);
            uint16_t n2 = mandlebrot_point(cx, cy, 64);
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
    printf("Total error: %d (out of %d) (worst %d)\n", m_err, DIM*DIM, worst);
}

int main() {
    //float x = -0.500000;
    //float y = -0.591797;
    //mandlebrot_point_myflp(x,y,64);
    //return 0;
    //float zero = 0.0;
    //float x = FP_ADD_V(zero, zero);
    //return 0;
    test_mandlebrot();
    return 0;
}

#ifndef FRACTAL_FXPT_H
#define FRACTAL_FXPT_H

#include <stdint.h>

//! Colour type (5-bit red, 6-bit green, 5-bit blue)
typedef uint16_t rgb565;

#define FXP_FRAC 28
typedef int32_t qxpy_t;

#define FXP_MUL(a,b) ((qxpy_t)((a >> (FXP_FRAC/2)) * (b >> (FXP_FRAC/2))))
#define FLOAT_TO_FXP(x) ((qxpy_t)(x * ((float)(1<<FXP_FRAC))))

//! \brief Pointer to fractal point calculation function
typedef uint16_t (*calc_frac_point_p)(qxpy_t cx, qxpy_t cy, uint16_t n_max);

uint16_t calc_mandelbrot_point_soft(qxpy_t cx, qxpy_t cy, uint16_t n_max);

//! Pointer to function mapping iteration to colour value
typedef rgb565 (*iter_to_colour_p)(uint16_t iter, uint16_t n_max);

rgb565 iter_to_bw(uint16_t iter, uint16_t n_max);
rgb565 iter_to_grayscale(uint16_t iter, uint16_t n_max);
rgb565 iter_to_colour(uint16_t iter, uint16_t n_max);

void draw_fractal(rgb565 *fbuf, int width, int height,
                  calc_frac_point_p cfp_p, iter_to_colour_p i2c_p,
                  qxpy_t cx_0, qxpy_t cy_0, qxpy_t delta, uint16_t n_max);

#endif // FRACTAL_FXPT_H

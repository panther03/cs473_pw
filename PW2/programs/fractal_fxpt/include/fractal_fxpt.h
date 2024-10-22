#ifndef FRACTAL_FXPT_H
#define FRACTAL_FXPT_H

#include <stdint.h>
#include "fractal_conv.h"

//! Colour type (5-bit red, 6-bit green, 5-bit blue)
typedef uint16_t rgb565;

//! \brief Pointer to fractal point calculation function
typedef uint16_t (*calc_frac_point_p)(fxp32_t cx, fxp32_t cy, uint16_t n_max);

uint16_t calc_mandelbrot_point_soft(fxp32_t cx, fxp32_t cy, uint16_t n_max);

//! Pointer to function mapping iteration to colour value
typedef rgb565 (*iter_to_colour_p)(uint16_t iter, uint16_t n_max);

rgb565 iter_to_bw(uint16_t iter, uint16_t n_max);
rgb565 iter_to_grayscale(uint16_t iter, uint16_t n_max);
rgb565 iter_to_colour(uint16_t iter, uint16_t n_max);

void draw_fractal(rgb565 *fbuf, int width, int height,
                  calc_frac_point_p cfp_p, iter_to_colour_p i2c_p,
                  fxp32_t cx_0, fxp32_t cy_0, fxp32_t delta, uint16_t n_max);

#endif // FRACTAL_FXPT_H

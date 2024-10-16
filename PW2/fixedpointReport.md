# Fixed Point Integers

## Format

In our fixed-point representation, we will use 32 bits to store the values.

From the theory behind the Mandelbrot algorithm, it is clear that the algorithm requires precise real numbers in order to function properly.

However, there are some restrictions on the range that we operate in. First of all, there is an escape condition for deciding if a coordinate is in the Mandelbrot set, namely:

$X_n^2 + Y_n^2 \geq 4$

Also, inside the coordinate system, we assume that we operate within the range:

$x = [-2, 1] \quad \text{and} \quad y = [-1.5, 1.5]$

as configured in the $\texttt{main\_fxpt.c}$ file. We can use this to derive a limit to the range of possible values.

First of all, we note that values can be negative, meaning we need a sign bit.

Next, we observe that the largest values that can be computed are the squares used in the escape condition. In the worst-case scenario, we have that, for instance, $X_n^2 + Y_n^2$ is close to $4$ but still below it. In the case they are equal, the next value of $Y_{n+1}$ will be $2 \cdot X_n \cdot Y_n$, which is almost equal to $8$. In the next escape condition, $Y_{n+1}^2$ results in a value just below $64$. After this, the escape condition stops computation and moves on to the next point. We consider this the worst-case incident, meaning we need 6 bits to represent values up to $64$, plus one more for the sign.

Considering this, we settled on the Q7.25 fixed-point format.

## Updated Code

We made all the fixed-point functionality in a file named $\texttt{fractal\_fxpt.c}$, with the corresponding header file:

```c
typedef int32_t fxp32_t;

fxp32_t floatToFXP(float in);
fxp32_t intToFXP(int in);
float fxpToFloat(fxp32_t in);

fxp32_t fxpAdd(fxp32_t op1, fxp32_t op2);
fxp32_t fxpSub(fxp32_t op1, fxp32_t op2);
fxp32_t fxpMul(fxp32_t op1, fxp32_t op2);
```

As shown, we define a new type $\texttt{fxp32\_t}$, which represents Q7.25 fixed-point values. Also shown are all the conversion and arithmetic functions.

By virtue of using fixed-point values, the arithmetic operations are rather simple. Only the $\textit{fxpMul}$ function has some particularities, arising from having to deal with overflow issues. This will be discussed further below.

In the $\texttt{fractal\_fxpt.h}$ and $\texttt{fractal\_fxpt.c}$ files, we changed the function definitions accordingly to allow the use of the fixed-point type.

```c
// Snippet from fractal_fxpt.h
typedef uint16_t (*calc_frac_point_p)(fxp32_t cx, fxp32_t cy, uint16_t n_max);

uint16_t calc_mandelbrot_point_soft(fxp32_t cx, fxp32_t cy, uint16_t n_max);

void draw_fractal(rgb565 *fbuf, int width, int height,
                  calc_frac_point_p cfp_p, iter_to_colour_p i2c_p,
                  fxp32_t cx_0, fxp32_t cy_0, fxp32_t delta, uint16_t n_max);
```

In $\texttt{src/main\_fxpt.c}$, we simply used the conversions to initialize the fixed-point values and called the $\textit{draw\_fractal}$ function with these values.

```c
fxp32_t CX_0_FXP = floatToFXP(CX_0);
fxp32_t CY_0_FXP = floatToFXP(CY_0);
fxp32_t DELTA_0_FXP = floatToFXP(delta);

draw_fractal(frameBuffer, SCREEN_WIDTH, SCREEN_HEIGHT, &calc_mandelbrot_point_soft, &iter_to_colour, CX_0_FXP, CY_0_FXP, DELTA_0_FXP, N_MAX);
```

## Handling Overflow

As described above, the format should allow for all relevant values to be represented inside the Q7.25 fixed-point format. However, during multiplication, two 32-bit integers are multiplied, meaning we risk overflowing. There are three ways of handling this:

1. Casting to 64-bit values during multiplication and truncating afterward.
2. Truncating beforehand, followed by a 32-bit multiplication.
3. Not doing anything and hoping for the best.

We will not consider the third option. For the first option, the following approach can be used:

```c
// Casting to 64-bit in order to avoid overflow
fxp32_t fxpMul(fxp32_t op1, fxp32_t op2) {
  int64_t res = ((int64_t)op1) * ((int64_t)op2);
  return (fxp32_t)(res >> 25);  // Shifting back
}
```

Here, casting is used before doing 64-bit multiplication. Afterward, the value is right-shifted 25 bits since multiplication causes the fraction part of the fixed-point number to be shifted 25 bits left.

For the second option, truncating can be done first, in the following way:

```c
// Avoiding 64-bit multiplication
fxp32_t fxpMul(fxp32_t op1, fxp32_t op2) {
  int32_t res = ((int32_t)(op1 >> 13)) * ((int32_t)(op2 >> 13));
  return (fxp32_t)(res << 1);  // Shifting back
}
```

Here, we remove the lowest 13 bits, which loses some precision. However, 12 bits are still available for the fraction, which might be sufficiently accurate for our purposes. Afterward, the 32-bit values are multiplied. As before, multiplying the two 12-bit fraction parts left-shifts it by 12 bits. The result needs to be shifted once more to align the fraction part at the 25th bit correctly.

## Results

Running the program, a major improvement in speed is seen. With caches enabled, the floating-point implementation takes about 85 seconds. For the 64-bit multiplication version, it takes about 15 seconds. For the 32-bit multiplication version, it takes just about 5.64 seconds.

There is little loss of detail in the image, as shown in the figure below.

<div style="display: flex; justify-content: space-around;">
  <figure>
    <img src="imgs/fxp_32bit_mult_w_cache.png" alt="Image A" style="width: auto;">
    <figcaption>FXP 32-bit multiplication</figcaption>
  </figure>
  <figure>
    <img src="imgs/fxp_64bit_mult_w_cache.png" alt="Image B" style="width: auto;">
    <figcaption>FXP 64-bit multiplication</figcaption>
  </figure>
  <figure>
    <img src="imgs/fxp_32bit_mult_w_cache.png" alt="Image C" style="width: auto;">
    <figcaption>FLP version</figcaption>
  </figure>
</div>

Based on this, we believe the 32-bit multiplication fixed-point version is the best, as it achieved an almost identical Mandelbrot while only taking about 5.6 seconds to complete.

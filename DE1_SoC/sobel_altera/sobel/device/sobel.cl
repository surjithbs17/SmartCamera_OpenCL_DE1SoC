/*
Credits: https://www.altera.com/support/support-resources/design-examples/design-software/opencl/sobel-filter.html
*/
#include "../host/inc/defines.h"

// Sobel filter kernel
// frame_in and frame_out are different buffers. Specify restrict on
// them so that the compiler knows they do not alias each other.
__kernel
void sobel(global unsigned int * restrict frame_in, global unsigned int * restrict frame_out,
           const int iterations, const unsigned int threshold)
{
    // Filter coefficients
    int Gx[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}};
    int Gy[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};

    // Pixel buffer of 2 rows and 3 extra pixels
    int rows[2 * COLS + 3];

    // The initial iterations are used to initialize the pixel buffer.
    int count = -(2 * COLS + 3);
    while (count != iterations) {
        // Each cycle, shift a new pixel into the buffer.
        // Unrolling this loop allows the compile to infer a shift register.
        #pragma unroll
        for (int i = COLS * 2 + 2; i > 0; --i) {
            rows[i] = rows[i - 1];
        }
        rows[0] = count >= 0 ? frame_in[count] : 0;

        int x_dir = 0;
        int y_dir = 0;

        // With these loops unrolled, one convolution can be computed every
        // cycle.
        #pragma unroll
        for (int i = 0; i < 3; ++i) {
            #pragma unroll
            for (int j = 0; j < 3; ++j) {
                unsigned int pixel = rows[i * COLS + j];
                unsigned int b = pixel & 0xff;
                unsigned int g = (pixel >> 8) & 0xff;
                unsigned int r = (pixel >> 16) & 0xff;

                // RGB -> Luma conversion approximation
                // Avoiding floating point math operators greatly reduces
                // resource usage.
                unsigned int luma = r * 66 + g * 129 + b * 25;
                luma = (luma + 128) >> 8;
                luma += 16;

                x_dir += luma * Gx[i][j];
                y_dir += luma * Gy[i][j];
            }
        }

        int temp = abs(x_dir) + abs(y_dir);
        unsigned int clamped;
        if (temp > threshold) {
            clamped = 0xffffff;
        } else {
            clamped = 0;
        }

        if (count >= 0) {
            frame_out[count] = clamped;
        }
        count++;
    }
}

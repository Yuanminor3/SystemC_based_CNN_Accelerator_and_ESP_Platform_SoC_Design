// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <stdio.h>
#include <stdlib.h>

#ifdef __riscv
#define MB_ADDR 0xB0000000
#else
#define MB_ADDR 0x50000000
#endif

typedef int32_t token_t;

//Conversion Functions
static inline int float2fixed(float value, int tot_bits, int n_int_bits)
{
        unsigned shift_int = 0x3f800000 + 0x800000 * (tot_bits - n_int_bits);
        float *shift = (float *) &shift_int;

        return (int)(value * (*shift));
}

static inline float fixed2float(int value, int tot_bits,int n_int_bits)
{
        unsigned shift_int = 0x3f800000 - 0x800000 * (tot_bits - n_int_bits);
        float *shift = (float *) (&shift_int);

        return (*shift) * (float)value;
}

//Programmer view for Convolutional Layer
void convolution_compute(float *dst, float *bias, float *src, float *w, int cols, int rows, int src_chans,
                        int dst_chans, int pool_size, int pool_stride, int do_pool, int do_pad)
{
    const int chan_stride = cols * rows;
    const int kernel_size = 9;

    // Convolution
    for (int k = 0; k < src_chans; k++) {
        int last = 0;
        if (k == src_chans - 1)
            last = 1;

        const float *img = &src[k * chan_stride];

        for (int c = 0; c < dst_chans; c++) {
            const float *filter = &w[k * dst_chans * kernel_size + c * kernel_size];
            float *out = &dst[chan_stride * c];
            float b = bias[c];

            float *out_pool = &dst[pool_stride * c];

            // dotsum_3x3
            for (int j = 1; j < cols - 1; j++)     // intput h
                for (int i = 1; i < cols - 1; i++) { // intput w
                    const int index_out_p0 = (j - 1) * cols + i - 1;
                    const int index_out_p1 = j * cols + i - 1;
                    const int index_out_p2 = (j - 1) * cols + i;
                    const int index_out_p3 = j * cols + i;
                    const int index_out = index_out_p3;
                    int index_out_pool;
                    if (do_pad)
                        index_out_pool = j / 2 * pool_size + i / 2;
                    else
                        index_out_pool = (j / 2 - 1) * pool_size + i / 2 - 1;

                    out[index_out] +=
                        img[index_out - 1 - 1 * cols] * filter[0] + img[index_out + 0 - 1 * cols] * filter[1] +
                        img[index_out + 1 - 1 * cols] * filter[2] + img[index_out - 1] * filter[3] +
                        img[index_out + 0] * filter[4] + img[index_out + 1] * filter[5] +
                        img[index_out - 1 + 1 * cols] * filter[6] + img[index_out + 0 + 1 * cols] * filter[7] +
                        img[index_out + 1 + 1 * cols] * filter[8];

                        // Activation
                    if (last) {
                        if (out[index_out] + b < 0)
                            out[index_out] = 0;
                        else
                            out[index_out] = out[index_out] + b;

                        // Max Pool 2x2
                        if ((j % 2 == 0) && (i % 2 == 0) && do_pool) {
                            const float p0 = out[index_out_p0];
                            const float p1 = out[index_out_p1];
                            const float p2 = out[index_out_p2];
                            const float p3 = out[index_out_p3];

                            float max = p0;
                            if (max < p1)
                                max = p1;
                            if (max < p2)
                                max = p2;
                            if (max < p3)
                                max = p3;
                            out_pool[index_out_pool] = max;
                        }
                    }
                }

            // Pad (resize)
            if (last && do_pad) {
                for (int i = 0; i < pool_size; i++)
                    out_pool[i] = 0.0;

                for (int i = 0; i < pool_size; i++)
                    out_pool[pool_size * (pool_size - 1) + i] = 0.0;

                for (int i = 0; i < pool_size; i++)
                    out_pool[pool_size * i] = 0.0;

                for (int i = 0; i < pool_size; i++)
                    out_pool[pool_size * i + pool_size - 1] = 0.0;
            }
        }
    }
}

//Programmer view for Fully Connected Layer
void fc_compute(float *dst, float *src, float *w, float *b, int w_cols, int w_rows, int relu)
{
    for (int j = 0; j < w_rows; j++) {
        const int w_index = j * w_cols;
        dst[j] = 0;
        for (int i = 0; i < w_cols; i++) {
            dst[j] += src[i] * w[w_index + i];
        }
        if (relu) {
            if (dst[j] + b[j] < 0)
                dst[j] = 0;
            else
                dst[j] += b[j];
        }
    }
}

//Function to store the model into the allocated buffers.
//Do not touch
static void load_model(float** test_1, float** w_conv, float** bias_conv, float** w_fc, float** bias_fc)
{
    float* data=(float*) MB_ADDR;

    for (int j = 0; j < 10; j++)
        for (int i = 0; i < 3468; i++)
                test_1[j][i] = (float) data[3468 * j + i];

    for (int i = 0; i < 864; i++)
        w_conv[0][i] = (float) data[10 * 3468 + i];

    for (int i = 0; i < 9216; i++)
        w_conv[1][i] = (float) data[864 + 10 * 3468 + i];

    for (int i = 0; i < 18432; i++)
        w_conv[2][i] = (float) data[9216 + 864 + 10 * 3468 + i];

    for (int i = 0; i < 73728; i++)
        w_conv[3][i] = (float) data[18432 + 9216 + 864 + 10 * 3468 +i];

    for (int i = 0; i < 32; i++)
        bias_conv[0][i] = (float) data[73728 + 18432 + 9216 + 864 + 10 * 3468 + i];

    for (int i = 0; i < 32; i++)
        bias_conv[1][i] = (float) data[32 + 73728 + 18432 + 9216 + 864 + 10 * 3468 + i];

    for (int i = 0; i < 64; i++)
        bias_conv[2][i] = (float) data[32 + 32 + 73728 + 18432 + 9216 + 864 + 10 * 3468 + i];

    for (int i = 0; i < 128; i++)
        bias_conv[3][i] = (float) data[64 + 32 + 32 + 73728 + 18432 + 9216 + 864 + 10 * 3468 + i];

    for (int i = 0; i < 64; i++)
        bias_fc[0][i] = (float) data[128 + 64 + 32 + 32 + 73728 + 18432 + 9216 + 864 + 10 * 3468 + i];

    for (int i = 0; i < 10; i++)
        bias_fc[1][i] = (float) data[64 + 128 + 64 + 32 + 32 + 73728 + 18432 + 9216 + 864 + 10 * 3468 + i];

    for (int i = 0; i < 131072; i++)
        w_fc[0][i] = (float) data[10 + 64 + 128 + 64 + 32 + 32 + 73728 + 18432 + 9216 + 864 + 10 * 3468 + i];

    for (int i = 0; i < 640; i++)
        w_fc[1][i] = (float) data[131072 + 10 + 64 + 128 + 64 + 32 + 32 + 73728 + 18432 + 9216 + 864 + 10 * 3468 + i];
}


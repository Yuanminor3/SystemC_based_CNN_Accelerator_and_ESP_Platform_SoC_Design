/* Copyright 2022 Columbia University SLD Group */

#ifndef __CONV_LAYER_FUNCTIONS_HPP__
#define __CONV_LAYER_FUNCTIONS_HPP__

#include "conv_layer.hpp"



void conv_layer_sysc_catapult::convolution_compute(uint32_t num_cols, uint32_t num_rows,
                                     uint32_t src_chans, uint32_t dst_chans,
                                     int src_chan, int dst_chan,
                                     bool do_pool, bool do_pad,
                                     uint32_t pool_size, uint32_t pool_stride,
                                     bool pingpong, bool out_pingpong)
{

    const int chan_stride = num_cols * num_rows;
    const int input_ptr = 0;
    const int input_w_ptr = 0;
    const int output_bias_ptr = dst_chan;
    const int output_ptr = 0;
    const int output_pool_ptr = 0;

    const bool last = (src_chan == src_chans - 1);

    FPDATA_WORD filter_word[9];
    FPDATA filter[9];

	#pragma hls_unroll factor=2
	#pragma hls_logic_opt
    for (int i = 0 ; i < 9; i++)
    {
#if defined(SMALL)
            //filter_word[i] = input_w_ping[0][input_w_ptr + i];
		if (pingpong)
            filter_word[i] = input_w_ping[0][input_w_ptr + i];
        else
            filter_word[i] = input_w_pong[0][input_w_ptr + i];
#endif        
#if defined(MEDIUM)
            //filter_word[i] = input_w_ping[0][input_w_ptr + i];
		if (pingpong)
            filter_word[i] = input_w_ping[0][input_w_ptr + i];
        else
            filter_word[i] = input_w_pong[0][input_w_ptr + i];
#endif
#if defined(FAST)
        if (pingpong)
            filter_word[i] = input_w_ping[0][input_w_ptr + i];
        else
            filter_word[i] = input_w_pong[0][input_w_ptr + i];

#endif
        int2f(filter_word[i], filter[i]);

    }

    FPDATA_WORD b_word =output_b[0][output_bias_ptr];
    FPDATA b;

    int2f(b_word, b);

    //
    // Convolution
    //

    const int cols = num_cols;
	#pragma hls_unroll factor=2
	#pragma hls_logic_opt
	#pragma hls_resource core=add limit=1
    // dotsum_3x3
    for (int j = 1; j < cols - 1; j++)
		#pragma hls_unroll factor=2
		#pragma hls_logic_opt
		#pragma hls_resource core=add limit=1
        for (int i = 1; i < cols - 1; i++)
        {
            //const int index_out_p0 = (j - 1) * cols + i - 1;
            //const int index_out_p1 = j * cols + i - 1;
            //const int index_out_p2 = (j - 1) * cols + i;
            const int index_out_p3 = j * cols + i;
            const int index_out = index_out_p3;

            int index_out_pool;

            if(do_pad)
			#pragma hls_logic_opt
            { index_out_pool = j / 2 * pool_size + i / 2; }
            else
			#pragma hls_logic_opt
            { index_out_pool = (j / 2 - 1) * pool_size + i / 2 - 1; }

            FPDATA_WORD img_word[9];
            FPDATA img[9];
			#pragma hls_logic_opt
            int index_k = (j*i)/3;
            int index_i;
#if defined(FAST)
			if(pingpong)
			{
                img_word[0]=input_ping[0][input_ptr + index_out - 1 - 1 * cols];
                img_word[1]=input_ping[0][input_ptr + index_out + 0 - 1 * cols];
                img_word[2]=input_ping[0][input_ptr + index_out + 1 - 1 * cols];

                img_word[3]=input_ping[0][input_ptr + index_out - 1 ];
                img_word[4]=input_ping[0][input_ptr + index_out + 0 ];
                img_word[5]=input_ping[0][input_ptr + index_out + 1 ];

                img_word[6]=input_ping[0][input_ptr + index_out - 1 + 1 * cols];
                img_word[7]=input_ping[0][input_ptr + index_out + 0 + 1 * cols];
                img_word[8]=input_ping[0][input_ptr + index_out + 1 + 1 * cols];


            }
            else
            {


                img_word[0]=input_pong[0][input_ptr + index_out - 1 - 1 * cols];
                img_word[1]=input_pong[0][input_ptr + index_out + 0 - 1 * cols];
                img_word[2]=input_pong[0][input_ptr + index_out + 1 - 1 * cols];

                img_word[3]=input_pong[0][input_ptr + index_out - 1 ];
                img_word[4]=input_pong[0][input_ptr + index_out + 0 ];
                img_word[5]=input_pong[0][input_ptr + index_out + 1 ];

                img_word[6]=input_pong[0][input_ptr + index_out - 1 + 1 * cols];
                img_word[7]=input_pong[0][input_ptr + index_out + 0 + 1 * cols];
                img_word[8]=input_pong[0][input_ptr + index_out + 1 + 1 * cols];


            }
#endif
            
#if defined(SMALL)
			if(pingpong)
			{
                img_word[0]=input_ping[0][input_ptr + index_out - 1 - 1 * cols];
                img_word[1]=input_ping[0][input_ptr + index_out + 0 - 1 * cols];
                img_word[2]=input_ping[0][input_ptr + index_out + 1 - 1 * cols];

                img_word[3]=input_ping[0][input_ptr + index_out - 1 ];
                img_word[4]=input_ping[0][input_ptr + index_out + 0 ];
                img_word[5]=input_ping[0][input_ptr + index_out + 1 ];

                img_word[6]=input_ping[0][input_ptr + index_out - 1 + 1 * cols];
                img_word[7]=input_ping[0][input_ptr + index_out + 0 + 1 * cols];
                img_word[8]=input_ping[0][input_ptr + index_out + 1 + 1 * cols];


            }
            else
            {


                img_word[0]=input_pong[0][input_ptr + index_out - 1 - 1 * cols];
                img_word[1]=input_pong[0][input_ptr + index_out + 0 - 1 * cols];
                img_word[2]=input_pong[0][input_ptr + index_out + 1 - 1 * cols];

                img_word[3]=input_pong[0][input_ptr + index_out - 1 ];
                img_word[4]=input_pong[0][input_ptr + index_out + 0 ];
                img_word[5]=input_pong[0][input_ptr + index_out + 1 ];

                img_word[6]=input_pong[0][input_ptr + index_out - 1 + 1 * cols];
                img_word[7]=input_pong[0][input_ptr + index_out + 0 + 1 * cols];
                img_word[8]=input_pong[0][input_ptr + index_out + 1 + 1 * cols];


            }
                /*img_word[0]=input_ping[0][input_ptr + index_out - 1 - 1 * cols];
                img_word[1]=input_ping[0][input_ptr + index_out + 0 - 1 * cols];
                img_word[2]=input_ping[0][input_ptr + index_out + 1 - 1 * cols];

                img_word[3]=input_ping[0][input_ptr + index_out - 1 ];
                img_word[4]=input_ping[0][input_ptr + index_out + 0 ];
                img_word[5]=input_ping[0][input_ptr + index_out + 1 ];

                img_word[6]=input_ping[0][input_ptr + index_out - 1 + 1 * cols];
                img_word[7]=input_ping[0][input_ptr + index_out + 0 + 1 * cols];
                img_word[8]=input_ping[0][input_ptr + index_out + 1 + 1 * cols];*/
			
#endif
#if defined(MEDIUM)
                /*img_word[0]=input_ping[0][input_ptr + index_out - 1 - 1 * cols];
                img_word[1]=input_ping[0][input_ptr + index_out + 0 - 1 * cols];
                img_word[2]=input_ping[0][input_ptr + index_out + 1 - 1 * cols];

                img_word[3]=input_ping[0][input_ptr + index_out - 1 ];
                img_word[4]=input_ping[0][input_ptr + index_out + 0 ];
                img_word[5]=input_ping[0][input_ptr + index_out + 1 ];

                img_word[6]=input_ping[0][input_ptr + index_out - 1 + 1 * cols];
                img_word[7]=input_ping[0][input_ptr + index_out + 0 + 1 * cols];
                img_word[8]=input_ping[0][input_ptr + index_out + 1 + 1 * cols];*/
			if(pingpong)
			{
                img_word[0]=input_ping[0][input_ptr + index_out - 1 - 1 * cols];
                img_word[1]=input_ping[0][input_ptr + index_out + 0 - 1 * cols];
                img_word[2]=input_ping[0][input_ptr + index_out + 1 - 1 * cols];

                img_word[3]=input_ping[0][input_ptr + index_out - 1 ];
                img_word[4]=input_ping[0][input_ptr + index_out + 0 ];
                img_word[5]=input_ping[0][input_ptr + index_out + 1 ];

                img_word[6]=input_ping[0][input_ptr + index_out - 1 + 1 * cols];
                img_word[7]=input_ping[0][input_ptr + index_out + 0 + 1 * cols];
                img_word[8]=input_ping[0][input_ptr + index_out + 1 + 1 * cols];


            }
            else
            {


                img_word[0]=input_pong[0][input_ptr + index_out - 1 - 1 * cols];
                img_word[1]=input_pong[0][input_ptr + index_out + 0 - 1 * cols];
                img_word[2]=input_pong[0][input_ptr + index_out + 1 - 1 * cols];

                img_word[3]=input_pong[0][input_ptr + index_out - 1 ];
                img_word[4]=input_pong[0][input_ptr + index_out + 0 ];
                img_word[5]=input_pong[0][input_ptr + index_out + 1 ];

                img_word[6]=input_pong[0][input_ptr + index_out - 1 + 1 * cols];
                img_word[7]=input_pong[0][input_ptr + index_out + 0 + 1 * cols];
                img_word[8]=input_pong[0][input_ptr + index_out + 1 + 1 * cols];


            }
#endif

            


                

			#pragma hls_unroll factor=2
			#pragma hls_logic_opt
			#pragma hls_resource core=add limit=1
            for (int k = 0; k < 9; k++)
			
                int2f(img_word[k], img[k]);

            FPDATA_WORD out_word;
            FPDATA out;

            if (src_chan == 0)
            { out = 0; }
            else
            {
#if defined(FAST)
                if (out_pingpong)
                    out_word = output_ping[0][output_ptr + index_out];
                else
                    out_word = output_pong[0][output_ptr + index_out];
#endif
#if defined(SMALL)
				if (out_pingpong)
                    out_word = output_ping[0][output_ptr + index_out];
                else
                    out_word = output_pong[0][output_ptr + index_out];
                    //out_word = output_ping[0][output_ptr + index_out];
#endif                
#if defined(MEDIUM)
				if (out_pingpong)
                    out_word = output_ping[0][output_ptr + index_out];
                else
                    out_word = output_pong[0][output_ptr + index_out];
                    //out_word = output_ping[0][output_ptr + index_out];
#endif 
                int2f(out_word, out);
            }
            #if defined(FAST)
                #pragma unroll yes
            #else
                #pragma hls_unroll no
            #endif 
			#pragma hls_logic_opt
			#pragma hls_resource core=add limit=1
            for (uint16_t k = 0; k < 9; k++)
				#pragma hls_logic_opt
                out+=img[k] * filter[k];

            // Activation
            if (last)
            {
                if (out + b < 0)
                { out = 0; }
                else
				#pragma hls_logic_opt
                { out = out + b; }
            }

            f2int(out, out_word);
#if defined(FAST)
            if (out_pingpong)
                output_ping[0][output_ptr + index_out] = out_word;
            else
                output_pong[0][output_ptr + index_out] = out_word;
#endif
#if defined(SMALL)
                //output_ping[0][output_ptr + index_out] = out_word;
			if (out_pingpong)
                output_ping[0][output_ptr + index_out] = out_word;
            else
                output_pong[0][output_ptr + index_out] = out_word;

#endif
#if defined(MEDIUM)
				//output_ping[0][output_ptr + index_out] = out_word;
			if (out_pingpong)
                output_ping[0][output_ptr + index_out] = out_word;
            else
                output_pong[0][output_ptr + index_out] = out_word;

#endif           
            wait();

        }

}

void conv_layer_sysc_catapult::poolpad_compute(uint32_t num_cols, uint32_t num_rows,
                                 uint32_t src_chans, uint32_t dst_chans,
                                 int src_chan, int dst_chan,
                                 bool do_pool, bool do_pad,
                                 uint32_t pool_size, uint32_t pool_stride,
                                 bool pingpong, bool out_pingpong)
{


    const int chan_stride = num_cols * num_rows;
    const int input_ptr = 0;
    const int input_w_ptr = 0;
    const int output_bias_ptr = dst_chan;
    const int output_ptr = 0;
    const int output_pool_ptr = 0;

    const int cols = num_cols;
	#pragma hls_unroll factor=2
	#pragma hls_logic_opt
    // dotsum_3x3
	#pragma hls_resource core=add limit=1
    for (int j = 1; j < cols - 1; j++) // intput h
		#pragma hls_logic_opt
		#pragma hls_unroll factor=2
		#pragma hls_resource core=add limit=1
        for (int i = 1; i < cols - 1; i++) // intput w
        {
			#pragma hls_logic_opt
            const int index_out_p0 = (j - 1) * cols + i - 1;
			#pragma hls_logic_opt
            const int index_out_p1 = j * cols + i - 1;
			#pragma hls_logic_opt
            const int index_out_p2 = (j - 1) * cols + i;
			#pragma hls_logic_opt
            const int index_out_p3 = j * cols + i;
			#pragma hls_logic_opt
            const int index_out = index_out_p3;
			

            int index_out_pool;

            if (do_pad)
			#pragma hls_logic_opt
            { index_out_pool = j / 2 * pool_size + i / 2; }
            else
			#pragma hls_logic_opt
            { index_out_pool = (j / 2 - 1) * pool_size + i / 2 - 1; }


            // Max Pool 2x2
            if ((j % 2 == 0) && (i % 2 == 0) && do_pool)
            {

                FPDATA_WORD p0_word, p1_word, p2_word, p3_word;
                FPDATA p0, p1, p2, p3;
#if defined(FAST)
                if (out_pingpong)
                {
                    p0_word = output_ping[0][output_ptr + index_out_p0];
                    p1_word = output_ping[0][output_ptr + index_out_p1];
                    p2_word = output_ping[0][output_ptr + index_out_p2];
                    p3_word = output_ping[0][output_ptr + index_out_p3];
                }
                else
                {
                    p0_word = output_pong[0][output_ptr + index_out_p0];
                    p1_word = output_pong[0][output_ptr + index_out_p1];
                    p2_word = output_pong[0][output_ptr + index_out_p2];
                    p3_word = output_pong[0][output_ptr + index_out_p3];
                }
#endif
#if defined(SMALL)
                    /*p0_word = output_ping[0][output_ptr + index_out_p0];
                    p1_word = output_ping[0][output_ptr + index_out_p1];
                    p2_word = output_ping[0][output_ptr + index_out_p2];
                    p3_word = output_ping[0][output_ptr + index_out_p3];*/
				if (out_pingpong)
                {
                    p0_word = output_ping[0][output_ptr + index_out_p0];
                    p1_word = output_ping[0][output_ptr + index_out_p1];
                    p2_word = output_ping[0][output_ptr + index_out_p2];
                    p3_word = output_ping[0][output_ptr + index_out_p3];
                }
                else
                {
                    p0_word = output_pong[0][output_ptr + index_out_p0];
                    p1_word = output_pong[0][output_ptr + index_out_p1];
                    p2_word = output_pong[0][output_ptr + index_out_p2];
                    p3_word = output_pong[0][output_ptr + index_out_p3];
                }
#endif                
#if defined(MEDIUM)
                    /*p0_word = output_ping[0][output_ptr + index_out_p0];
                    p1_word = output_ping[0][output_ptr + index_out_p1];
                    p2_word = output_ping[0][output_ptr + index_out_p2];
                    p3_word = output_ping[0][output_ptr + index_out_p3];*/
					if (out_pingpong)
                {
                    p0_word = output_ping[0][output_ptr + index_out_p0];
                    p1_word = output_ping[0][output_ptr + index_out_p1];
                    p2_word = output_ping[0][output_ptr + index_out_p2];
                    p3_word = output_ping[0][output_ptr + index_out_p3];
                }
                else
                {
                    p0_word = output_pong[0][output_ptr + index_out_p0];
                    p1_word = output_pong[0][output_ptr + index_out_p1];
                    p2_word = output_pong[0][output_ptr + index_out_p2];
                    p3_word = output_pong[0][output_ptr + index_out_p3];
                }
#endif          

                int2f(p0_word, p0);
                int2f(p1_word, p1);
                int2f(p2_word, p2);
                int2f(p3_word, p3);

                FPDATA max = p0;

                if (max < p1)
                { max = p1; }

                if (max < p2)
                { max = p2; }

                if (max < p3)
                { max = p3; }

                FPDATA_WORD out_word;

                f2int(max, out_word);
#if defined(FAST)
                if (out_pingpong)
                    output_ping[0][output_pool_ptr + index_out_pool] = out_word;
                else
                    output_pong[0][output_pool_ptr + index_out_pool] = out_word;
#endif
#if defined(SMALL)
				if (out_pingpong)
                    output_ping[0][output_pool_ptr + index_out_pool] = out_word;
                else
                    output_pong[0][output_pool_ptr + index_out_pool] = out_word;
                    //output_ping[0][output_pool_ptr + index_out_pool] = out_word;
#endif
#if defined(MEDIUM)
				if (out_pingpong)
                    output_ping[0][output_pool_ptr + index_out_pool] = out_word;
                else
                    output_pong[0][output_pool_ptr + index_out_pool] = out_word;
                    //output_ping[0][output_pool_ptr + index_out_pool] = out_word;
#endif
            }
        }
	
    // Pad (resize)*/
    /*if (do_pad)
    {
		#pragma hls_unroll factor=1
		#pragma hls_logic_opt
        for (int i = 0; i < pool_size; i ++)
        {
            // first row
            if (out_pingpong)
                output_ping[0][output_pool_ptr + i]=0;
            else
                output_pong[0][output_pool_ptr + i]=0;
        }
		#pragma hls_unroll factor=1
		#pragma hls_logic_opt
        for (int i = 0; i < pool_size; i ++)
        {
            // last row
            if (out_pingpong)
				#pragma hls_logic_opt
                output_ping[0][output_pool_ptr + pool_size * (pool_size - 1) + i]=0;
				#pragma hls_logic_opt
            else
                output_pong[0][output_pool_ptr + pool_size * (pool_size - 1) + i]=0;

        }
		#pragma hls_unroll factor=1
		#pragma hls_logic_opt
        for (int i = 0; i < pool_size; i ++)
        {
            // first column
            if (out_pingpong)
                output_ping[0][output_pool_ptr + pool_size * i]=0;
            else
                output_pong[0][output_pool_ptr + pool_size * i]=0;

        }
		#pragma hls_unroll factor=1
		#pragma hls_logic_opt
        for (int i = 0; i < pool_size; i ++)
        {
            // last column
            if (out_pingpong)
                output_ping[0][output_pool_ptr + pool_size * i + pool_size - 1]=0;
            else
                output_pong[0][output_pool_ptr + pool_size * i + pool_size - 1]=0;
        }
    }*/
}




#endif /* __CONV_LAYER_FUNCTIONS_HPP__ */
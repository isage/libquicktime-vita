/*******************************************************************************
 cmodel_permutation.h

 libquicktime - A library for reading and writing quicktime/avi/mp4 files.
 http://libquicktime.sourceforge.net

 Copyright (C) 2002 Heroine Virtual Ltd.
 Copyright (C) 2002-2011 Members of the libquicktime project.

 This library is free software; you can redistribute it and/or modify it under
 the terms of the GNU Lesser General Public License as published by the Free
 Software Foundation; either version 2.1 of the License, or (at your option)
 any later version.

 This library is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 details.

 You should have received a copy of the GNU Lesser General Public License along
 with this library; if not, write to the Free Software Foundation, Inc., 51
 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*******************************************************************************/

#include "colorspace_tables.h"
#include "colorspace_macros.h"
#include <quicktime/colormodels.h>
#include <inttypes.h>

// ****************************** Pixel transfers *****************************

// *************************** RGB565 -> ************************************

static inline void transfer_RGB565_to_RGB565(uint16_t *(*output), uint16_t *input)
{
        *(*output) = *input;
	(*output) ++;
}

static inline void transfer_RGB565_to_RGB888(unsigned char *(*output), uint16_t *input)
{
        (*output)[0] = RGB16_TO_R_8(*input);
        (*output)[1] = RGB16_TO_G_8(*input);
        (*output)[2] = RGB16_TO_B_8(*input);
	(*output) +=3;
}

static inline void transfer_RGB565_to_RGBA8888(unsigned char *(*output), uint16_t *input)
{
        (*output)[0] = RGB16_TO_R_8(*input);
        (*output)[1] = RGB16_TO_G_8(*input);
        (*output)[2] = RGB16_TO_B_8(*input);
        (*output)[3] = 0xff;
	(*output) +=4;
}

// *************************** BGR565 -> ************************************

static inline void transfer_BGR565_to_BGR565(uint16_t *(*output), uint16_t *input)
{
        *(*output) = *input;
	(*output) ++;
}

static inline void transfer_BGR565_to_RGB888(unsigned char *(*output), uint16_t *input)
{
        (*output)[0] = BGR16_TO_R_8(*input);
        (*output)[1] = BGR16_TO_G_8(*input);
        (*output)[2] = BGR16_TO_B_8(*input);
	(*output) +=3;
}

static inline void transfer_BGR565_to_RGBA8888(unsigned char *(*output), uint16_t *input)
{
        (*output)[0] = BGR16_TO_R_8(*input);
        (*output)[1] = BGR16_TO_G_8(*input);
        (*output)[2] = BGR16_TO_B_8(*input);
        (*output)[3] = 0xff;
	(*output) +=4;
}

// *************************** RGB888 -> ************************************

static inline void transfer_RGB888_to_BGR565(uint16_t *(*output), unsigned char *input)
{
        PACK_8_TO_BGR16(input[0], input[1], input[2], *(*output));
	(*output) ++;
}


static inline void transfer_RGB888_to_RGB565(uint16_t *(*output), unsigned char *input)
{
        PACK_8_TO_RGB16(input[0], input[1], input[2], *(*output));
	(*output) ++;
}

static inline void transfer_RGB888_to_BGR888(unsigned char *(*output), unsigned char *input)
{
	(*output)[0] = input[2];
	(*output)[1] = input[1];
	(*output)[2] = input[0];
	(*output) += 3;
}

static inline void transfer_RGB888_to_RGB888(unsigned char *(*output), unsigned char *input)
{
	(*output)[0] = input[0];
	(*output)[1] = input[1];
	(*output)[2] = input[2];
	(*output) += 3;
}

static inline void transfer_RGB888_to_RGBA8888(unsigned char *(*output), unsigned char *input)
{
	(*output)[0] = input[0];
	(*output)[1] = input[1];
	(*output)[2] = input[2];
	(*output)[3] = 0xff;
	(*output) += 4;
}

static inline void transfer_RGB888_to_RGB161616(uint16_t *(*output), unsigned char *input)
{
	(*output)[0] = (input[0] << 8) | input[0];
	(*output)[1] = (input[1] << 8) | input[1];
	(*output)[2] = (input[2] << 8) | input[2];
	(*output) += 3;
}

static inline void transfer_RGB888_to_RGBA16161616(uint16_t *(*output), unsigned char *input)
{
	(*output)[0] = (input[0] << 8) | input[0];
	(*output)[1] = (input[1] << 8) | input[1];
	(*output)[2] = (input[2] << 8) | input[2];
	(*output)[3] = 0xffff;
	(*output) += 4;
}


static inline void transfer_RGB888_to_BGR8888(unsigned char *(*output), unsigned char *input)
{
	(*output)[0] = input[2];
	(*output)[1] = input[1];
	(*output)[2] = input[0];
	(*output) += 4;
}


static inline void transfer_RGB888_to_YUVA8888(unsigned char *(*output), unsigned char *input)
{
	int y, u, v;

	RGB_24_TO_YUV_8(input[0], input[1], input[2], y, u, v);

	(*output)[0] = y;
	(*output)[1] = u;
	(*output)[2] = v;
	(*output)[3] = 255;
	(*output) += 4;
}


static inline void transfer_RGB888_to_YUVA16161616(uint16_t *(*output), unsigned char *input)
{
	RGB_24_TO_YUV_16(input[0], input[1], input[2], (*output)[0], (*output)[1], (*output)[2]);
	(*output)[3] = 0xffff;
	(*output) += 4;
}

static inline void transfer_RGB888_to_YUV420P_YUV422P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	unsigned char *input,
	int output_column)
{
	int y, u, v;

	RGB_24_TO_YUV_8(input[0], input[1], input[2], y, u, v);

	output_y[output_column] = y;
	output_u[output_column / 2] = u;
	output_v[output_column / 2] = v;
}

static inline void transfer_RGB888_to_YUVJ420P_YUVJ422P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	unsigned char *input,
	int output_column)
{
	int y, u, v;

	RGB_24_TO_YUVJ_8(input[0], input[1], input[2], y, u, v);

	output_y[output_column] = y;
	output_u[output_column / 2] = u;
	output_v[output_column / 2] = v;
}

static inline void transfer_RGB888_to_YUV422P16(uint16_t *output_y, 
	uint16_t *output_u, 
	uint16_t *output_v, 
	unsigned char *input,
	int output_column)
{
	RGB_24_TO_YUV_16(input[0], input[1], input[2],
                         output_y[output_column], output_u[output_column/2], output_v[output_column/2]);

}

static inline void transfer_RGB888_to_YUV422P10(uint16_t *output_y,
    uint16_t *output_u,
    uint16_t *output_v,
    unsigned char *input,
    int output_column)
{
    RGB_24_TO_YUV_10(input[0], input[1], input[2],
                         output_y[output_column], output_u[output_column/2], output_v[output_column/2]);

}

static inline void transfer_RGB888_to_YUVJ422P10(uint16_t *output_y,
    uint16_t *output_u,
    uint16_t *output_v,
    unsigned char *input,
    int output_column)
{
    RGB_24_TO_YUVJ_10(input[0], input[1], input[2],
                         output_y[output_column], output_u[output_column/2], output_v[output_column/2]);

}

static inline void transfer_RGB888_to_YUV411P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	unsigned char *input,
	int output_column)
{
	int y, u, v;

	RGB_24_TO_YUV_8(input[0], input[1], input[2], y, u, v);

	output_y[output_column] = y;
	output_u[output_column / 4] = u;
	output_v[output_column / 4] = v;
}


static inline void transfer_RGB888_to_YUV444P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	unsigned char *input,
	int output_column)
{
	int y, u, v;

	RGB_24_TO_YUV_8(input[0], input[1], input[2], y, u, v);

	output_y[output_column] = y;
	output_u[output_column] = u;
	output_v[output_column] = v;
}

static inline void transfer_RGB888_to_YUVJ444P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	unsigned char *input,
	int output_column)
{
	int y, u, v;

	RGB_24_TO_YUVJ_8(input[0], input[1], input[2], y, u, v);

	output_y[output_column] = y;
	output_u[output_column] = u;
	output_v[output_column] = v;
}

static inline void transfer_RGB888_to_YUV444P16(uint16_t *output_y, 
                                                uint16_t *output_u, 
                                                uint16_t *output_v, 
                                                unsigned char *input,
                                                int output_column)
{
	int y, u, v;

	RGB_24_TO_YUV_16(input[0], input[1], input[2], y, u, v);

	output_y[output_column] = y;
	output_u[output_column] = u;
	output_v[output_column] = v;
}

static inline void transfer_RGB888_to_YUV422(unsigned char *(*output), 
	unsigned char *input,
	int j)
{
	int y, u, v;

	RGB_24_TO_YUV_8(input[0], input[1], input[2], y, u, v);

	if(!(j & 1))
	{ 
// Store U and V for even pixels only
		 (*output)[1] = u;
		 (*output)[3] = v;
		 (*output)[0] = y;
	}
	else
	{ 
// Store Y and advance output for odd pixels only
		 (*output)[2] = y;
		 (*output) += 4;
	}

}







// *************************** RGBA8888 -> ************************************

// These routines blend in a black background

static inline void transfer_RGBA8888_to_BGR565(uint16_t *(*output), unsigned char *input)
{
	unsigned int r, g, b, a;
	a = input[3];
	r = ((unsigned int)input[0] * a) >> 8;
	g = ((unsigned int)input[1] * a) >> 8;
	b = ((unsigned int)input[2] * a) >> 8;
        PACK_8_TO_BGR16(r, g, b, *(*output));
	(*output) ++;
}

static inline void transfer_RGBA8888_to_RGB565(uint16_t *(*output), unsigned char *input)
{
	unsigned int r, g, b, a;
	a = input[3];
	r = ((unsigned int)input[0] * a)>> 8;
	g = ((unsigned int)input[1] * a)>> 8;
	b = ((unsigned int)input[2] * a)>> 8;
        PACK_8_TO_RGB16(r, g, b, *(*output));
	(*output) ++;
}

static inline void transfer_RGBA8888_to_BGR888(unsigned char *(*output), unsigned char *input)
{
	unsigned int r, g, b, a;
	a = input[3];
	r = ((unsigned int)input[0] * a)>> 8;
	g = ((unsigned int)input[1] * a)>> 8;
	b = ((unsigned int)input[2] * a)>> 8;
	(*output)[0] = b;
	(*output)[1] = g;
	(*output)[2] = r;
	(*output) += 3;
}

static inline void transfer_RGBA8888_to_RGB888(unsigned char *(*output), unsigned char *input)
{
	unsigned int r, g, b, a;
	a = input[3];
	r = ((unsigned int)input[0] * a)>> 8;
	g = ((unsigned int)input[1] * a)>> 8;
	b = ((unsigned int)input[2] * a)>> 8;
	(*output)[0] = r;
	(*output)[1] = g;
	(*output)[2] = b;
	(*output) += 3;
}

static inline void transfer_RGBA8888_to_RGBA8888(unsigned char *(*output), unsigned char *input)
{
	(*output)[0] = input[0];
	(*output)[1] = input[1];
	(*output)[2] = input[2];
	(*output)[3] = input[3];
	(*output) += 4;
}

static inline void transfer_RGBA8888_to_RGB161616(uint16_t *(*output), unsigned char *input)
{
	int opacity;
	
	opacity = input[3];
	(*output)[0] = (((int)input[0] << 8) | input[0]) * opacity / 0xff;
	(*output)[1] = (((int)input[1] << 8) | input[1]) * opacity / 0xff;
	(*output)[2] = (((int)input[2] << 8) | input[2]) * opacity / 0xff;
	(*output) += 3;
}

static inline void transfer_RGBA8888_to_RGBA16161616(uint16_t *(*output), unsigned char *input)
{

	(*output)[0] = (((int)input[0]) << 8) | input[0];
	(*output)[1] = (((int)input[1]) << 8) | input[1];
	(*output)[2] = (((int)input[2]) << 8) | input[2];
	(*output)[3] = (((int)input[3]) << 8) | input[3];
	(*output) += 4;
}

static inline void transfer_RGBA8888_to_BGR8888(unsigned char *(*output), unsigned char *input)
{
	unsigned int r, g, b, a;
	a = input[3];
	r = ((unsigned int)input[0] * a)>> 8;
	g = ((unsigned int)input[1] * a)>> 8;
	b = ((unsigned int)input[2] * a)>> 8;
	(*output)[0] = b;
	(*output)[1] = g;
	(*output)[2] = r;
	(*output) += 4;
}


static inline void transfer_RGBA8888_to_YUVA8888(unsigned char *(*output), unsigned char *input)
{
	int y, u, v;

	RGB_24_TO_YUV_8(input[0], input[1], input[2], y, u, v);

	(*output)[0] = y;
	(*output)[1] = u;
	(*output)[2] = v;
	(*output)[3] = input[3];
	(*output) += 4;
}


static inline void transfer_RGBA8888_to_YUVA16161616(uint16_t *(*output), unsigned char *input)
{
	int r, g, b;

	r = (((int)input[0]) << 8) | input[0];
	g = (((int)input[1]) << 8) | input[1];
	b = (((int)input[2]) << 8) | input[2];
	RGB_24_TO_YUV_16(input[0], input[1], input[2], (*output)[0], (*output)[1], (*output)[2]);

	(*output)[3] =  RGB_8_TO_16(input[3]);
	(*output) += 4;
}




static inline void transfer_RGBA8888_to_YUV420P_YUV422P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	unsigned char *input,
	int output_column)
        {
	int y, u, v, a, r, g, b;
	
	a = input[3];
	r = (input[0] * a)>> 8;
	g = (input[1] * a)>> 8;
	b = (input[2] * a)>> 8;

	RGB_24_TO_YUV_8(r, g, b, y, u, v);

	output_y[output_column] = y;
	output_u[output_column / 2] = u;
	output_v[output_column / 2] = v;
}

static inline void transfer_RGBA8888_to_YUVJ420P_YUVJ422P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	unsigned char *input,
	int output_column)
        {
	int y, u, v, a, r, g, b;
	
	a = input[3];
	r = (input[0] * a)>> 8;
	g = (input[1] * a)>> 8;
	b = (input[2] * a)>> 8;

	RGB_24_TO_YUVJ_8(r, g, b, y, u, v);

	output_y[output_column] = y;
	output_u[output_column / 2] = u;
	output_v[output_column / 2] = v;
}

static inline void transfer_RGBA8888_to_YUV444P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	unsigned char *input,
        int output_column)
{
	int y, u, v, a, r, g, b;
	
	a = input[3];
	r = (input[0] * a)>> 8;
	g = (input[1] * a)>> 8;
	b = (input[2] * a)>> 8;

	RGB_24_TO_YUV_8(r, g, b, y, u, v);

	output_y[output_column] = y;
	output_u[output_column] = u;
	output_v[output_column] = v;
  
}




static inline void transfer_RGBA8888_to_YUV422(unsigned char *(*output), 
	unsigned char *input,
	int j)
{
	int y, u, v, a, r, g, b;
	
	a = input[3];
	r = (input[0] * a)>> 8;
	g = (input[1] * a)>> 8;
	b = (input[2] * a)>> 8;

	RGB_24_TO_YUV_8(r, g, b, y, u, v);

	if(!(j & 1))
	{ 
// Store U and V for even pixels only
		 (*output)[1] = u;
		 (*output)[3] = v;
		 (*output)[0] = y;
	}
	else
	{ 
// Store Y and advance output for odd pixels only
		 (*output)[2] = y;
		 (*output) += 4;
	}


}

static inline void transfer_RGBA8888_to_YUV422P16(uint16_t *output_y, 
	uint16_t *output_u, 
	uint16_t *output_v, 
	unsigned char *input,
	int output_column)
{
	int a, r, g, b;
	
	a = input[3];
	r = (input[0] * a)>> 8;
	g = (input[1] * a)>> 8;
	b = (input[2] * a)>> 8;

	RGB_24_TO_YUV_16(r, g, b, output_y[output_column], output_u[output_column/2], output_v[output_column/2]);

}

static inline void transfer_RGBA8888_to_YUV411P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	unsigned char *input,
	int output_column)
{
        int y, u, v, a, r, g, b;
	
	a = input[3];
	r = (input[0] * a)>> 8;
	g = (input[1] * a)>> 8;
	b = (input[2] * a)>> 8;

	RGB_24_TO_YUV_8(r, g, b, y, u, v);

	output_y[output_column] = y;
	output_u[output_column / 4] = u;
	output_v[output_column / 4] = v;
}


static inline void transfer_RGBA8888_to_YUVJ444P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	unsigned char *input,
	int output_column)
{
        int y, u, v, a, r, g, b;
	
	a = input[3];
	r = (input[0] * a)>> 8;
	g = (input[1] * a)>> 8;
	b = (input[2] * a)>> 8;

	RGB_24_TO_YUVJ_8(r, g, b, y, u, v);

	output_y[output_column] = y;
	output_u[output_column] = u;
	output_v[output_column] = v;
}

static inline void transfer_RGBA8888_to_YUV444P16(uint16_t *output_y, 
                                                uint16_t *output_u, 
                                                uint16_t *output_v, 
                                                unsigned char *input,
                                                int output_column)
{
        int y, u, v, a, r, g, b;
	
	a = input[3];
	r = (input[0] * a)>> 8;
	g = (input[1] * a)>> 8;
	b = (input[2] * a)>> 8;

	RGB_24_TO_YUV_16(r, g, b, y, u, v);

	output_y[output_column] = y;
	output_u[output_column] = u;
	output_v[output_column] = v;
}







// ******************************** RGB161616 -> *********************************


static inline void transfer_RGB161616_to_BGR565(uint16_t *(*output), uint16_t *input)
{
        PACK_16_TO_BGR16(input[0], input[1], input[2], *(*output))
	(*output) ++;
}

static inline void transfer_RGB161616_to_RGB565(uint16_t *(*output), uint16_t *input)
{
        PACK_16_TO_RGB16(input[0], input[1], input[2], *(*output))
	(*output) ++;
}

static inline void transfer_RGB161616_to_BGR888(unsigned char *(*output), uint16_t *input)
{
	(*output)[0] = input[2] >> 8;
	(*output)[1] = input[1] >> 8;
	(*output)[2] = input[0] >> 8;
	(*output) += 3;
}

static inline void transfer_RGB161616_to_RGB888(unsigned char *(*output), uint16_t *input)
{
	(*output)[0] = input[0] >> 8;
	(*output)[1] = input[1] >> 8;
	(*output)[2] = input[2] >> 8;
	(*output) += 3;
}

static inline void transfer_RGB161616_to_RGBA8888(unsigned char *(*output), uint16_t *input)
{
	(*output)[0] = input[0] >> 8;
	(*output)[1] = input[1] >> 8;
	(*output)[2] = input[2] >> 8;
	(*output)[3] = 0xff;
	(*output) += 4;
}

static inline void transfer_RGB161616_to_BGR8888(unsigned char *(*output), uint16_t *input)
{
	(*output)[0] = input[2] >> 8;
	(*output)[1] = input[1] >> 8;
	(*output)[2] = input[0] >> 8;
	(*output) += 4;
}


static inline void transfer_RGB161616_to_YUVA8888(unsigned char *(*output), uint16_t *input)
{
	int y, u, v, r, g, b;

	r = input[0] >> 8;
	g = input[1] >> 8;
	b = input[2] >> 8;

	RGB_24_TO_YUV_8(r, g, b, y, u, v);

	(*output)[0] = y;
	(*output)[1] = u;
	(*output)[2] = v;
	(*output)[3] = 255;
	(*output) += 4;
}


static inline void transfer_RGB161616_to_YUV420P_YUV422P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	uint16_t *input,
	int output_column)
{
	int y, u, v, r, g, b;
	r = input[0] >> 8;
	g = input[1] >> 8;
	b = input[2] >> 8;

	RGB_24_TO_YUV_8(r, g, b, y, u, v);

	output_y[output_column] = y;
	output_u[output_column / 2] = u;
	output_v[output_column / 2] = v;
}

static inline void transfer_RGB161616_to_YUV422(unsigned char *(*output),
  uint16_t *input,
  int j)
{
  int y, u, v, r, g, b;
  r = input[0];
  g = input[1];
  b = input[2];

  RGB_48_TO_YUV_8(r,g,b,y,u,v);

  if(!(j & 1))
  {
// Store U and V for even pixels only
     (*output)[1] = u;
     (*output)[3] = v;
     (*output)[0] = y;
  }
  else
  {
// Store Y and advance output for odd pixels only
     (*output)[2] = y;
     (*output) += 4;
  }

}

static inline void transfer_RGB161616_to_YUV422P16(uint16_t *output_y, 
	uint16_t *output_u, 
	uint16_t *output_v, 
	uint16_t *input,
	int output_column)
{
	int y, u, v;

	RGB_48_TO_YUV_16(input[0], input[1], input[2], y, u, v);

	output_y[output_column] = y;
	output_u[output_column / 2] = u;
	output_v[output_column / 2] = v;
}

static inline void transfer_RGB161616_to_YUV444P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	uint16_t *input,
	int output_column)
{
	int y, u, v, r, g, b;
	r = input[0] >> 8;
	g = input[1] >> 8;
	b = input[2] >> 8;

	RGB_24_TO_YUV_8(r, g, b, y, u, v);

	output_y[output_column] = y;
	output_u[output_column] = u;
	output_v[output_column] = v;
}

static inline void transfer_RGB161616_to_YUV444P16(uint16_t *output_y, 
	uint16_t *output_u, 
	uint16_t *output_v, 
	uint16_t *input,
	int output_column)
{
	int y, u, v;

	RGB_48_TO_YUV_16(input[0], input[1], input[2], y, u, v);

	output_y[output_column] = y;
	output_u[output_column] = u;
	output_v[output_column] = v;
}

static inline void transfer_RGB161616_to_YUV422P10(uint16_t *output_y,
    uint16_t *output_u,
    uint16_t *output_v,
    uint16_t *input,
    int output_column)
{
    int y, u, v;

    RGB_48_TO_YUV_10(input[0], input[1], input[2], y, u, v);

    output_y[output_column] = y;
    output_u[output_column / 2] = u;
    output_v[output_column / 2] = v;
}

static inline void transfer_RGB161616_to_YUVJ422P10(uint16_t *output_y,
    uint16_t *output_u,
    uint16_t *output_v,
    uint16_t *input,
    int output_column)
{
    int y, u, v;

    RGB_48_TO_YUVJ_10(input[0], input[1], input[2], y, u, v);

    output_y[output_column] = y;
    output_u[output_column / 2] = u;
    output_v[output_column / 2] = v;
}


// ****************************** RGBA16161616 -> *****************************


static inline void transfer_RGBA16161616_to_BGR565(uint16_t *(*output), uint16_t *input)
{
	unsigned int r, g, b, a;
	a = (input)[3] >> 8;
	r = (unsigned int)(input)[0] * a;
	g = (unsigned int)(input)[1] * a;
	b = (unsigned int)(input)[2] * a;

	*(uint16_t*)(*output) = (uint16_t)(((b & 0xf80000) >> 8) + 
				((g & 0xfc0000) >> 13) + 
				((r & 0xf80000) >> 19));
	(*output) ++;
}

static inline void transfer_RGBA16161616_to_RGB565(uint16_t *(*output), uint16_t *input)
{
	unsigned int r, g, b, a;
	a = (input)[3] >> 8;
	r = (unsigned int)(input)[0] * a;
	g = (unsigned int)(input)[1] * a;
	b = (unsigned int)(input)[2] * a;

	*(uint16_t*)(*output) = (uint16_t)(((r & 0xf80000) >> 8) + 
				((g & 0xfc0000) >> 13) + 
				((b & 0xf80000) >> 19));
	(*output) ++;
}

static inline void transfer_RGBA16161616_to_BGR888(unsigned char *(*output), uint16_t *input)
{
	unsigned int r, g, b, a;
	a = (input)[3] >> 8;
	r = (unsigned int)(input)[0] * a;
	g = (unsigned int)(input)[1] * a;
	b = (unsigned int)(input)[2] * a;

	(*output)[0] = (unsigned char)(b >> 16);
	(*output)[1] = (unsigned char)(g >> 16);
	(*output)[2] = (unsigned char)(r >> 16);
	(*output) += 3;
}

static inline void transfer_RGBA16161616_to_RGB888(unsigned char *(*output), uint16_t *input)
{
	unsigned int r, g, b, a;
	a = (input)[3] >> 8;
	r = (unsigned int)(input)[0] * a;
	g = (unsigned int)(input)[1] * a;
	b = (unsigned int)(input)[2] * a;

	(*output)[0] = (unsigned char)(r >> 16);
	(*output)[1] = (unsigned char)(g >> 16);
	(*output)[2] = (unsigned char)(b >> 16);
	(*output) += 3;
}


static inline void transfer_RGBA16161616_to_RGBA8888(unsigned char *(*output), uint16_t *input)
{
	(*output)[0] = input[0] >> 8;
	(*output)[1] = input[1] >> 8;
	(*output)[2] = input[2] >> 8;
	(*output)[3] = input[3] >> 8;
	(*output) += 4;
}


static inline void transfer_RGBA16161616_to_BGR8888(unsigned char *(*output), uint16_t *input)
{
	unsigned int r, g, b, a;
	a = (input)[3] >> 8;
	r = (input)[0] * a;
	g = (input)[1] * a;
	b = (input)[2] * a;

	(*output)[0] = (unsigned char)(b >> 16);
	(*output)[1] = (unsigned char)(g >> 16);
	(*output)[2] = (unsigned char)(r >> 16);
	(*output) += 4;
}


static inline void transfer_RGBA16161616_to_YUV420P_YUV422P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	uint16_t *input,
	int output_column)
{
	int y, u, v, r, g, b;
	int64_t a;
	a = input[3];
	r = (int64_t)input[0] * a / 0xffffff;
	g = (int64_t)input[1] * a / 0xffffff;
	b = (int64_t)input[2] * a / 0xffffff;

	RGB_24_TO_YUV_8(r, g, b, y, u, v);

	output_y[output_column] = y;
	output_u[output_column / 2] = u;
	output_v[output_column / 2] = v;
}

static inline void transfer_RGBA16161616_to_YUV444P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	uint16_t *input,
	int output_column)
{
	int y, u, v, r, g, b;
	int64_t a;
	a = input[3];
	r = (int64_t)input[0] * a / 0xffffff;
	g = (int64_t)input[1] * a / 0xffffff;
	b = (int64_t)input[2] * a / 0xffffff;

	RGB_24_TO_YUV_8(r, g, b, y, u, v);

	output_y[output_column] = y;
	output_u[output_column] = u;
	output_v[output_column] = v;
}
















static inline void transfer_BGR8888_to_RGB888(unsigned char *(*output), unsigned char *input)
{
	(*output)[0] = input[2];
	(*output)[1] = input[1];
	(*output)[2] = input[0];
	(*output) += 3;
}

static inline void transfer_BGR8888_to_RGBA8888(unsigned char *(*output), unsigned char *input)
{
	(*output)[0] = input[2];
	(*output)[1] = input[1];
	(*output)[2] = input[0];
        (*output)[3] = 0xff;
	(*output) += 4;
}

static inline void transfer_BGR8888_to_BGR8888(unsigned char *(*output), unsigned char *input)
{
	(*output)[0] = input[0];
	(*output)[1] = input[1];
	(*output)[2] = input[2];
	(*output) += 4;
}

// ******************************** BGR888 -> *********************************

static inline void transfer_BGR888_to_RGB888(unsigned char *(*output), unsigned char *input)
{
	(*output)[0] = input[2];
	(*output)[1] = input[1];
	(*output)[2] = input[0];
	(*output) += 3;
}

static inline void transfer_BGR888_to_BGR888(unsigned char *(*output), unsigned char *input)
{
	(*output)[0] = input[0];
	(*output)[1] = input[1];
	(*output)[2] = input[2];
	(*output) += 3;
}

static inline void transfer_BGR888_to_BGR565(uint16_t *(*output), unsigned char *input)
{
        PACK_8_TO_BGR16(input[2], input[1], input[0], *(*output));
                
	(*output) ++;
}

static inline void transfer_BGR888_to_RGB565(uint16_t *(*output), unsigned char *input)
{
        PACK_8_TO_RGB16(input[2], input[1], input[0], *(*output));
        (*output) ++;
}

static inline void transfer_BGR888_to_RGBA8888(unsigned char *(*output), unsigned char *input)
{
	(*output)[0] = input[2];
	(*output)[1] = input[1];
	(*output)[2] = input[0];
	(*output)[3] = 0xff;
	(*output) += 4;
}

static inline void transfer_BGR888_to_RGB161616(uint16_t *(*output), unsigned char *input)
{
	(*output)[0] = (input[2] << 8) | input[2];
	(*output)[1] = (input[1] << 8) | input[1];
	(*output)[2] = (input[0] << 8) | input[0];
	(*output) += 3;
}

static inline void transfer_BGR888_to_RGBA16161616(uint16_t *(*output), unsigned char *input)
{
	(*output)[0] = (input[2] << 8) | input[2];
	(*output)[1] = (input[1] << 8) | input[1];
	(*output)[2] = (input[0] << 8) | input[0];
	(*output)[3] = 0xffff;
	(*output) += 4;
}


static inline void transfer_BGR888_to_BGR8888(unsigned char *(*output), unsigned char *input)
{
	(*output)[0] = input[0];
	(*output)[1] = input[1];
	(*output)[2] = input[2];
	(*output) += 4;
}


static inline void transfer_BGR888_to_YUVA8888(unsigned char *(*output), unsigned char *input)
{
	int y, u, v;

	RGB_24_TO_YUV_8(input[2], input[1], input[0], y, u, v);

	(*output)[0] = y;
	(*output)[1] = u;
	(*output)[2] = v;
	(*output)[3] = 255;
	(*output) += 4;
}


static inline void transfer_BGR888_to_YUV420P_YUV422P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	unsigned char *input,
	int output_column)
{
	int y, u, v;

	RGB_24_TO_YUV_8(input[2], input[1], input[0], y, u, v);

	output_y[output_column] = y;
	output_u[output_column / 2] = u;
	output_v[output_column / 2] = v;
}

static inline void transfer_BGR888_to_YUV444P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	unsigned char *input,
	int output_column)
{
	int y, u, v;

	RGB_24_TO_YUV_8(input[2], input[1], input[0], y, u, v);

	output_y[output_column] = y;
	output_u[output_column] = u;
	output_v[output_column] = v;
}

static inline void transfer_BGR888_to_YUV422(unsigned char *(*output), 
	unsigned char *input,
	int j)
{
	int y, u, v;

	RGB_24_TO_YUV_8(input[2], input[1], input[0], y, u, v);

	if(!(j & 1))
	{ 
// Store U and V for even pixels only
		 (*output)[1] = u;
		 (*output)[3] = v;
		 (*output)[0] = y;
	}
	else
	{ 
// Store Y and advance output for odd pixels only
		 (*output)[2] = y;
		 (*output) += 4;
	}

}

// ******************************** YUVA8888 -> *******************************

static inline void transfer_YUVA8888_to_BGR565(uint16_t *(*output), unsigned char *input)
{
        int a, i_tmp;
	int r, g, b;
	
	a = input[3];
	YUV_8_TO_RGB_24(input[0], input[1], input[2], r, g, b);
		
	r *= a;
	g *= a;
	b *= a;

        PACK_16_TO_BGR16(r, g, b, *(*output));
        (*output) ++;
}

static inline void transfer_YUVA8888_to_RGB565(uint16_t *(*output), unsigned char *input)
{
	int a, i_tmp;
	int r, g, b;
	
	a = input[3];
	YUV_8_TO_RGB_24(input[0], input[1], input[2], r, g, b);
		
	r *= a;
	g *= a;
	b *= a;

        PACK_16_TO_RGB16(r, g, b, *(*output));
        (*output) ++;
}

static inline void transfer_YUVA8888_to_BGR888(unsigned char *(*output), unsigned char *input)
{
	int a, i_tmp;
	int r, g, b;
	
	a = input[3];

	YUV_8_TO_RGB_24(input[0], input[1], input[2], r, g, b);
        
	r *= a;
	g *= a;
	b *= a;

	(*output)[0] = RGB_16_TO_8(b);
	(*output)[1] = RGB_16_TO_8(g);
	(*output)[2] = RGB_16_TO_8(r);
	(*output) += 3;
}

static inline void transfer_YUVA8888_to_RGB888(unsigned char *(*output), unsigned char *input)
{
	int a, i_tmp;
	int r, g, b;
	
	a = input[3];

	YUV_8_TO_RGB_24(input[0], input[1], input[2], r, g, b);
        
	r *= a;
	g *= a;
	b *= a;

	(*output)[2] = RGB_16_TO_8(b);
	(*output)[1] = RGB_16_TO_8(g);
	(*output)[0] = RGB_16_TO_8(r);
	(*output) += 3;
}

static inline void transfer_YUVA8888_to_RGBA8888(unsigned char *(*output), unsigned char *input)
{
        int i_tmp;
	YUV_8_TO_RGB_24(input[0], input[1], input[2], (*output)[0], (*output)[1], (*output)[2]);
	(*output)[3] = input[3];
	(*output) += 4;
}

static inline void transfer_YUVA8888_to_BGR8888(unsigned char *(*output), unsigned char *input)
{
	int a, i_tmp;
	int r, g, b;
	
	a = input[3];

	YUV_8_TO_RGB_24(input[0], input[1], input[2], r, g, b);
        
	r *= a;
	g *= a;
	b *= a;

	(*output)[0] = RGB_16_TO_8(b);
	(*output)[1] = RGB_16_TO_8(g);
	(*output)[2] = RGB_16_TO_8(r);
	(*output) += 4;
}

static inline void transfer_YUVA8888_to_YUVA8888(unsigned char *(*output), unsigned char *input)
{
	(*output)[0] = input[0];
	(*output)[1] = input[1];
	(*output)[2] = input[2];
	(*output)[3] = input[3];
	(*output) += 4;
}



static inline void transfer_YUVA8888_to_YUV420P_YUV422P(unsigned char *output_y,  
        unsigned char *output_u,
        unsigned char *output_v,
        unsigned char *input,
        int output_column)
{
        int opacity = input[3];
        int transparency = 0xff - opacity;
 
        output_y[output_column] =     ((input[0] * opacity) >> 8) + 1;
        output_u[output_column / 2] = ((input[1] * opacity + 0x80 * transparency) >> 8) + 1;
        output_v[output_column / 2] = ((input[2] * opacity + 0x80 * transparency) >> 8) + 1;
}

static inline void transfer_YUVA8888_to_YUV444P(unsigned char *output_y,
        unsigned char *output_u,
        unsigned char *output_v,
        unsigned char *input,
        int output_column)
{
        int opacity = input[3];
        int transparency = 0xff - opacity;
                                                                                
        output_y[output_column] =     ((input[0] * opacity) >> 8) + 1;
        output_u[output_column] = ((input[1] * opacity + 0x80 * transparency) >> 8) + 1;
        output_v[output_column] = ((input[2] * opacity + 0x80 * transparency) >> 8) + 1;
}


static inline void transfer_YUVA8888_to_YUV422(unsigned char *(*output),
        unsigned char *input,
        int j)
{
        int opacity = input[3];
        int transparency = 0xff - opacity;
// Store U and V for even pixels only
        if(!(j & 1))
        {
                (*output)[0] = ((input[0] * opacity) >> 8) + 1;
                (*output)[1] = ((input[1] * opacity + 0x80 * transparency) >> 8) + 1;
                (*output)[3] = ((input[2] * opacity + 0x80 * transparency) >> 8) + 1;
        }
        else
// Store Y and advance output for odd pixels only
        {
                (*output)[2] = ((input[0] * opacity) >> 8) + 1;
                (*output) += 4;
        }
}

// ******************************** YUV422P -> ********************************

static inline void transfer_YUV422P_to_BGR565(uint16_t *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
int r, g, b, i_tmp;
	YUV_8_TO_RGB_24(*input_y, *input_u, *input_v, r, g, b);
        PACK_8_TO_BGR16(r, g, b, *(*output))
                 
	(*output) ++;
}

static inline void transfer_YUV422P_to_RGB565(uint16_t *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
	int r, g, b, i_tmp;
	YUV_8_TO_RGB_24(*input_y, *input_u, *input_v, r, g, b);
        PACK_8_TO_RGB16(r, g, b, *(*output))
                 
	(*output) ++;
}

static inline void transfer_YUV422P_to_BGR888(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
        int i_tmp;
	YUV_8_TO_RGB_24(*input_y, *input_u, *input_v, (*output)[2], (*output)[1], (*output)[0])
	(*output) += 3;
}

static inline void transfer_YUV422P_to_BGR8888(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
        int i_tmp;
	YUV_8_TO_RGB_24(*input_y, *input_u, *input_v, (*output)[2], (*output)[1], (*output)[0])
	(*output) += 4;
}

static inline void transfer_YUV422P_to_RGB888(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
        int i_tmp;
	YUV_8_TO_RGB_24(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
	(*output) += 3;
}

static inline void transfer_YUV422P_to_RGBA8888(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
        int i_tmp;
	YUV_8_TO_RGB_24(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
	(*output)[3] = 0xff;
	(*output) += 4;
}

static inline void transfer_YUV422P_to_RGB161616(uint16_t *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
        int i_tmp;
	YUV_8_TO_RGB_48(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
	(*output) += 3;
}


static inline void transfer_YUV422P_to_RGBA16161616(uint16_t *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
        int i_tmp;
	YUV_8_TO_RGB_48(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
	(*output)[3] = 0xffff;
	(*output) += 4;
}

static inline void transfer_YUV422P_to_YUVA8888(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
	(*output)[0] = *input_y;
	(*output)[1] = *input_u;
	(*output)[2] = *input_v;
	(*output)[3] = 0xff;
	(*output) += 4;
}


static inline void transfer_YUV422P_to_YUV420P(unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v,
	unsigned char *output_y,
	unsigned char *output_u,
	unsigned char *output_v,
	int j)
{
	output_y[j] = *input_y;
	output_u[j / 2] = *input_u;
	output_v[j / 2] = *input_v;
}

static inline void transfer_YUV422P_to_YUV444P(unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v,
	unsigned char *output_y,
	unsigned char *output_u,
	unsigned char *output_v,
	int j)
{
	output_y[j] = *input_y;
	output_u[j] = *input_u;
	output_v[j] = *input_v;
}

static inline void transfer_YUV422P_to_YUVJ420P(unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v,
	unsigned char *output_y,
	unsigned char *output_u,
	unsigned char *output_v,
	int j)
{
	output_y[j] = Y_8_TO_YJ_8(*input_y);
	output_u[j / 2] = UV_8_TO_UVJ_8(*input_u);
	output_v[j / 2] = UV_8_TO_UVJ_8(*input_v);
}

static inline void transfer_YUV422P_to_YUVJ422P(unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v,
	unsigned char *output_y,
	unsigned char *output_u,
	unsigned char *output_v,
	int j)
{
	output_y[j] = Y_8_TO_YJ_8(*input_y);
	output_u[j] = UV_8_TO_UVJ_8(*input_u);
	output_v[j] = UV_8_TO_UVJ_8(*input_v);
}

static inline void transfer_YUV422P_to_YUV422(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v,
	int j)
{
// Store U and V for even pixels only
	if(!(j & 1))
	{
		(*output)[1] = *input_u;
		(*output)[3] = *input_v;
		(*output)[0] = *input_y;
	}
	else
// Store Y and advance output for odd pixels only
	{
		(*output)[2] = *input_y;
		(*output) += 4;
	}
}

// ******************************** YUV422P16 -> ********************************

static inline void transfer_YUV422P16_to_RGB888(unsigned char *(*output), 
	uint16_t *input_y,
	uint16_t *input_u,
	uint16_t *input_v)
{
        int i_tmp;
	YUV_16_TO_RGB_24(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
	(*output) += 3;
}

static inline void transfer_YUV422P16_to_RGBA8888(unsigned char *(*output), 
	uint16_t *input_y,
	uint16_t *input_u,
	uint16_t *input_v)
{
        int i_tmp;
	YUV_16_TO_RGB_24(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
        (*output)[3] = 0xff;
	(*output) += 4;
}

static inline void transfer_YUV422P16_to_RGB161616(uint16_t *(*output), 
	uint16_t *input_y,
	uint16_t *input_u,
	uint16_t *input_v)
{
        int i_tmp;
	YUV_16_TO_RGB_48(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
	(*output) += 3;
}

static inline void transfer_YUV422P16_to_YUV420P16(uint16_t *input_y,
                                                   uint16_t *input_u,
                                                   uint16_t *input_v,
                                                   uint16_t *output_y,
                                                   uint16_t *output_u,
                                                   uint16_t *output_v,
                                                   int j)
{
	output_y[j] = *input_y;
	output_u[j / 2] = *input_u;
	output_v[j / 2] = *input_v;
}

static inline void transfer_YUV422P16_to_YUV420P(uint16_t *input_y,
                                                   uint16_t *input_u,
                                                   uint16_t *input_v,
                                                   uint8_t *output_y,
                                                   uint8_t *output_u,
                                                   uint8_t *output_v,
                                                   int j)
{
	output_y[j] = Y_16_TO_Y_8(*input_y);
	output_u[j / 2] = UV_16_TO_UV_8(*input_u);
	output_v[j / 2] = UV_16_TO_UV_8(*input_v);
}

// ******************************** YUV422P10 -> ********************************

static inline void transfer_YUV422P10_to_RGB888(unsigned char *(*output),
    uint16_t *input_y,
    uint16_t *input_u,
    uint16_t *input_v)
{
    int i_tmp;
    YUV_10_TO_RGB_24(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
    (*output) += 3;
}

static inline void transfer_YUV422P10_to_RGB161616(uint16_t *(*output),
    uint16_t *input_y,
    uint16_t *input_u,
    uint16_t *input_v)
{
        int i_tmp;
    YUV_10_TO_RGB_48(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
    (*output) += 3;
}

// ******************************** YUVJ422P10 -> ********************************

static inline void transfer_YUVJ422P10_to_RGB888(unsigned char *(*output),
    uint16_t *input_y,
    uint16_t *input_u,
    uint16_t *input_v)
{
    int i_tmp;
    YUVJ_10_TO_RGB_24(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
    (*output) += 3;
}

static inline void transfer_YUVJ422P10_to_RGB161616(uint16_t *(*output),
	uint16_t *input_y,
	uint16_t *input_u,
	uint16_t *input_v)
{
    int i_tmp;
    YUVJ_10_TO_RGB_48(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
	(*output) += 3;
}

// ******************************** YUV444P16 -> ********************************

static inline void transfer_YUV444P16_to_RGB888(unsigned char *(*output), 
	uint16_t *input_y,
	uint16_t *input_u,
	uint16_t *input_v)
{
        int i_tmp;
	YUV_16_TO_RGB_24(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
	(*output) += 3;
}

static inline void transfer_YUV444P16_to_RGBA8888(unsigned char *(*output), 
	uint16_t *input_y,
	uint16_t *input_u,
	uint16_t *input_v)
{
        int i_tmp;
	YUV_16_TO_RGB_24(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
        (*output)[3] = 0xff;
	(*output) += 4;
}

static inline void transfer_YUV444P16_to_RGB161616(uint16_t *(*output), 
	uint16_t *input_y,
	uint16_t *input_u,
	uint16_t *input_v)
{
        int i_tmp;
	YUV_16_TO_RGB_48(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
	(*output) += 3;
}

static inline void transfer_YUV444P16_to_YUV444P16(uint16_t *input_y,
	uint16_t *input_u,
	uint16_t *input_v,
	uint16_t *output_y,
	uint16_t *output_u,
	uint16_t *output_v,
	int j)
{
	output_y[j] = *input_y;
	output_u[j] = *input_u;
	output_v[j] = *input_v;
}

static inline void transfer_YUV444P16_to_YUV444P(uint16_t *input_y,
	uint16_t *input_u,
	uint16_t *input_v,
	uint8_t *output_y,
	uint8_t *output_u,
	uint8_t *output_v,
	int j)
{
	output_y[j] = Y_16_TO_Y_8(*input_y);
	output_u[j] = UV_16_TO_UV_8(*input_u);
	output_v[j] = UV_16_TO_UV_8(*input_v);
}

// ******************************** YUV411P -> ********************************


static inline void transfer_YUV411P_to_RGB888(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
        int i_tmp;
	YUV_8_TO_RGB_24(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
	(*output) += 3;
}

static inline void transfer_YUV411P_to_RGBA8888(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
        int i_tmp;
	YUV_8_TO_RGB_24(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
        (*output)[3] = 0xff;
	(*output) += 4;
}


// ******************************** YUV444P -> ********************************

static inline void transfer_YUV444P_to_YUV444P(unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v,
	unsigned char *output_y,
	unsigned char *output_u,
	unsigned char *output_v,
	int j)
{
	output_y[j] = *input_y;
	output_u[j] = *input_u;
	output_v[j] = *input_v;
}

// ******************************** YUVJ444P -> ********************************

static inline void transfer_YUVJ444P_to_RGB888(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
        int i_tmp;
	YUVJ_8_TO_RGB_24(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
	(*output) += 3;
}

static inline void transfer_YUVJ444P_to_RGBA8888(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
        int i_tmp;
	YUVJ_8_TO_RGB_24(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
        (*output)[3] = 0xff;
	(*output) += 4;
}

// ******************************** YUVJ422P -> ********************************

static inline void transfer_YUVJ422P_to_RGB888(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
        int i_tmp;
	YUVJ_8_TO_RGB_24(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
	(*output) += 3;
}

static inline void transfer_YUVJ422P_to_RGBA8888(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
        int i_tmp;
	YUVJ_8_TO_RGB_24(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
        (*output)[3] = 0xff;
	(*output) += 4;
}

static inline void transfer_YUVJ422P_to_YUV422(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v,
	int j)
{
// Store U and V for even pixels only
	if(!(j & 1))
	{
		(*output)[1] = UVJ_8_TO_UV_8(*input_u);
		(*output)[3] = UVJ_8_TO_UV_8(*input_v);
		(*output)[0] = YJ_8_TO_Y_8(*input_y);
	}
	else
// Store Y and advance output for odd pixels only
	{
		(*output)[2] = YJ_8_TO_Y_8(*input_y);
		(*output) += 4;
	}
}

static inline void transfer_YUVJ422P_to_YUV420P(unsigned char *input_y,
       unsigned char *input_u,
       unsigned char *input_v,
       unsigned char *output_y,
       unsigned char *output_u,
       unsigned char *output_v,
       int j)
{
       output_y[j] = YJ_8_TO_Y_8(*input_y);
       output_u[j / 2] = UVJ_8_TO_UV_8(*input_u);
       output_v[j / 2] = UVJ_8_TO_UV_8(*input_v);
}

// ******************************** YUVJ420P -> ********************************

static inline void transfer_YUVJ420P_to_RGB888(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
        int i_tmp;
	YUVJ_8_TO_RGB_24(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
	(*output) += 3;
}

static inline void transfer_YUVJ420P_to_RGBA8888(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
        int i_tmp;
	YUVJ_8_TO_RGB_24(*input_y, *input_u, *input_v, (*output)[0], (*output)[1], (*output)[2])
        (*output)[3] = 0xff;
	(*output) += 4;
}


// ******************************** YUV422 -> *********************************

static inline void transfer_YUV422_to_BGR565(uint16_t *(*output), 
	unsigned char *input, 
	int column)
{
        int y, i_tmp;
	int r, g, b;

// Even pixel
	if(!(column & 1))
		y = input[0];
	else
// Odd pixel
		y = input[2];
	YUV_8_TO_RGB_24(y, input[1], input[3], r, g, b);
        PACK_8_TO_BGR16(r, g, b, *(*output));
        (*output)++;
}

static inline void transfer_YUV422_to_RGB565(uint16_t *(*output), 
	unsigned char *input, 
	int column)
{
        int y, i_tmp;
	int r, g, b;

// Even pixel
	if(!(column & 1))
		y = input[0];
	else
// Odd pixel
		y = input[2];
	YUV_8_TO_RGB_24(y, input[1], input[3], r, g, b);
        PACK_8_TO_RGB16(r, g, b, *(*output));
        (*output)++;
}

static inline void transfer_YUV422_to_BGR888(unsigned char *(*output), 
	unsigned char *input, 
	int column)
{
        int y, i_tmp;

// Even pixel
	if(!(column & 1))
		y = input[0];
	else
// Odd pixel
		y = input[2];
	YUV_8_TO_RGB_24(y, input[1], input[3], (*output)[2], (*output)[1], (*output)[0]);
        (*output) += 3;
}

static inline void transfer_YUV422_to_RGB888(unsigned char *(*output), 
	unsigned char *input, 
	int column)
{
        int y, i_tmp;

// Even pixel
	if(!(column & 1))
		y = input[0];
	else
// Odd pixel
		y = input[2];
	YUV_8_TO_RGB_24(y, input[1], input[3], (*output)[0], (*output)[1], (*output)[2]);
        (*output) += 3;
}

static inline void transfer_YUV422_to_RGBA8888(unsigned char *(*output), 
	unsigned char *input, 
	int column)
{
        int y, i_tmp;

// Even pixel
	if(!(column & 1))
		y = input[0];
	else
// Odd pixel
		y = input[2];
	YUV_8_TO_RGB_24(y, input[1], input[3], (*output)[0], (*output)[1], (*output)[2]);
	(*output)[3] = 0xff;
	(*output) += 4;
}

static inline void transfer_YUV422_to_RGB161616(uint16_t *(*output), 
	unsigned char *input, 
	int column)
{
        int y, i_tmp;

// Even pixel
	if(!(column & 1))
		y = input[0];
	else
// Odd pixel
		y = input[2];
	YUV_8_TO_RGB_48(y, input[1], input[3], (*output)[0], (*output)[1], (*output)[2]);
	(*output) += 3;
}

static inline void transfer_YUV422_to_RGBA16161616(uint16_t *(*output), 
	unsigned char *input, 
	int column)
{
        int y, i_tmp;

// Even pixel
	if(!(column & 1))
		y = input[0];
	else
// Odd pixel
		y = input[2];
	YUV_8_TO_RGB_24(y, input[1], input[3], (*output)[0], (*output)[1], (*output)[2]);
	(*output)[3] = 0xffff;
	(*output) += 4;
}


static inline void transfer_YUV422_to_YUVA8888(unsigned char *(*output), 
	unsigned char *input, 
	int column)
{
// Even pixel
	if(!(column & 1))
		(*output)[0] = input[0];
	else
// Odd pixel
		(*output)[0] = input[2];

	(*output)[1] = input[1];
	(*output)[2] = input[3];
	(*output)[3] = 255;
	(*output) += 4;
}


static inline void transfer_YUV422_to_BGR8888(unsigned char *(*output), 
	unsigned char *input, 
	int column)
{
        int y, i_tmp;
	
// Even pixel
	if(!(column & 1))
		y = input[0];
	else
// Odd pixel
		y = input[2];

	YUV_8_TO_RGB_24(y, input[1], input[3], (*output)[2], (*output)[1], (*output)[0])
	(*output) += 4;
}


static inline void transfer_YUV422_to_YUV422P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	unsigned char *input,
	int output_column)
{
// Store U and V for even pixels only
	if(!(output_column & 1))
	{
		output_y[output_column] = input[0];
		output_u[output_column / 2] = input[1];
		output_v[output_column / 2] = input[3];
	}
	else
// Store Y and advance output for odd pixels only
	{
		output_y[output_column] = input[2];
	}
}

static inline void transfer_YUV422_to_YUV420P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	unsigned char *input,
	int output_column,
	int output_row)
{
// Even column
	if(!(output_column & 1))
	{
		output_y[output_column] = input[0];
// Store U and V for even columns and even rows only
		if(!(output_row & 1))
		{
			output_u[output_column / 2] = input[1];
			output_v[output_column / 2] = input[3];
		}
	}
	else
// Odd column
	{
		output_y[output_column] = input[2];
	}
}

static inline void transfer_YUV422_to_YUV422(unsigned char *(*output), 
	unsigned char *input,
	int j)
{
// Store U and V for even pixels only
	if(!(j & 1))
	{
		(*output)[0] = input[0];
		(*output)[1] = input[1];
		(*output)[3] = input[3];
	}
	else
// Store Y and advance output for odd pixels only
	{
		(*output)[2] = input[2];
		(*output) += 4;
	}
}

static inline void transfer_YUV422_to_YUVJ422P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	unsigned char *input,
	int output_column)
{
// Store U and V for even pixels only
	if(!(output_column & 1))
	{
		output_y[output_column] = Y_8_TO_YJ_8(input[0]);
		output_u[output_column / 2] = UV_8_TO_UVJ_8(input[1]);
		output_v[output_column / 2] = UV_8_TO_UVJ_8(input[3]);
	}
	else
// Store Y and advance output for odd pixels only
	{
		output_y[output_column] = Y_8_TO_YJ_8(input[2]);
	}
}

// ******************************** Loops *************************************
// 

#define TRANSFER_FRAME_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		unsigned char *output_row = output_rows[i]; \
		unsigned char *input_row = input_rows[row_table[i]]; \
		for(j = 0; j < out_w; j++) \
		{

#define TRANSFER_FRAME_HEAD_16 \
	for(i = 0; i < out_h; i++) \
	{ \
                uint16_t *output_row = (uint16_t*)(output_rows[i]); \
		unsigned char *input_row = input_rows[row_table[i]]; \
		for(j = 0; j < out_w; j++) \
		{



#define TRANSFER_FRAME_TAIL \
		} \
	}

#define TRANSFER_YUV420P_OUT_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		unsigned char *input_row = input_rows[row_table[i]]; \
		unsigned char *output_y = output_rows[0] + i * out_rowspan; \
		unsigned char *output_u = output_rows[1] + i / 2 * out_rowspan_uv; \
		unsigned char *output_v = output_rows[2] + i / 2 * out_rowspan_uv; \
		for(j = 0; j < out_w; j++) \
		{

#define TRANSFER_YUV422P_OUT_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		unsigned char *input_row = input_rows[row_table[i]]; \
		unsigned char *output_y = output_rows[0] + i * out_rowspan; \
		unsigned char *output_u = output_rows[1] + i * out_rowspan_uv; \
		unsigned char *output_v = output_rows[2] + i * out_rowspan_uv; \
		for(j = 0; j < out_w; j++) \
		{

#define TRANSFER_YUV422P16_OUT_HEAD  \
	for(i = 0; i < out_h; i++) \
	{ \
		unsigned char *input_row = input_rows[row_table[i]]; \
		unsigned char *output_y = output_rows[0] + i * out_rowspan; \
		unsigned char *output_u = output_rows[1] + i * out_rowspan_uv; \
		unsigned char *output_v = output_rows[2] + i * out_rowspan_uv; \
		for(j = 0; j < out_w; j++) \
		{

#define TRANSFER_YUV422P16_OUT_HEAD_16   \
	for(i = 0; i < out_h; i++) \
	{ \
                unsigned char *input_row = input_rows[row_table[i]]; \
		uint16_t *output_y = (uint16_t *)(output_rows[0] + i * out_rowspan); \
		uint16_t *output_u = (uint16_t *)(output_rows[1] + i * out_rowspan_uv); \
		uint16_t *output_v = (uint16_t *)(output_rows[2] + i * out_rowspan_uv); \
                for(j = 0; j < out_w; j++)                        \
		{

#define TRANSFER_YUV411P_OUT_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		unsigned char *input_row = input_rows[row_table[i]]; \
		unsigned char *output_y = output_rows[0] + i * out_rowspan; \
		unsigned char *output_u = output_rows[1] + i * out_rowspan_uv; \
		unsigned char *output_v = output_rows[2] + i * out_rowspan_uv; \
		for(j = 0; j < out_w; j++) \
		{


#define TRANSFER_YUV444P_OUT_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		unsigned char *input_row = input_rows[row_table[i]]; \
		unsigned char *output_y = output_rows[0] + i * out_rowspan; \
		unsigned char *output_u = output_rows[1] + i * out_rowspan_uv; \
		unsigned char *output_v = output_rows[2] + i * out_rowspan_uv; \
		for(j = 0; j < out_w; j++) \
		{

#define TRANSFER_YUV444P16_OUT_HEAD_16   \
	for(i = 0; i < out_h; i++) \
	{ \
                unsigned char *input_row = input_rows[row_table[i]]; \
		uint16_t *output_y = (uint16_t *)(output_rows[0] + i * out_rowspan); \
		uint16_t *output_u = (uint16_t *)(output_rows[1] + i * out_rowspan_uv); \
		uint16_t *output_v = (uint16_t *)(output_rows[2] + i * out_rowspan_uv); \
                for(j = 0; j < out_w; j++)                        \
		{

#define TRANSFER_YUV420P_IN_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		unsigned char *output_row = output_rows[i]; \
		unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
		unsigned char *input_u = input_rows[1] + (row_table[i] / 2) * in_rowspan_uv; \
		unsigned char *input_v = input_rows[2] + (row_table[i] / 2) * in_rowspan_uv; \
		for(j = 0; j < out_w; j++) \
		{

#define TRANSFER_YUV411P_IN_HEAD   \
	for(i = 0; i < out_h; i++) \
	{ \
		unsigned char *output_row = output_rows[i]; \
		unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
		unsigned char *input_u = input_rows[1] + row_table[i] * in_rowspan_uv; \
		unsigned char *input_v = input_rows[2] + row_table[i] * in_rowspan_uv; \
		for(j = 0; j < out_w; j++) \
		{

#define TRANSFER_YUV420P_IN_HEAD_16 \
	for(i = 0; i < out_h; i++) \
	{ \
                uint16_t *output_row = (uint16_t*)(output_rows[i]); \
		unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
		unsigned char *input_u = input_rows[1] + (row_table[i] / 2) * in_rowspan_uv; \
		unsigned char *input_v = input_rows[2] + (row_table[i] / 2) * in_rowspan_uv; \
		for(j = 0; j < out_w; j++) \
		{


#define TRANSFER_YUV422P_IN_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		unsigned char *output_row = output_rows[i]; \
		unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
		unsigned char *input_u = input_rows[1] + row_table[i] * in_rowspan_uv; \
		unsigned char *input_v = input_rows[2] + row_table[i] * in_rowspan_uv; \
		for(j = 0; j < out_w; j++) \
		{

#define TRANSFER_YUV422P16_IN_HEAD   \
	for(i = 0; i < out_h; i++) \
	{ \
		uint8_t * output_row = output_rows[i]; \
		uint16_t * input_y = (uint16_t *)(input_rows[0] + row_table[i] * in_rowspan); \
		uint16_t * input_u = (uint16_t *)(input_rows[1] + row_table[i] * in_rowspan_uv); \
		uint16_t * input_v = (uint16_t *)(input_rows[2] + row_table[i] * in_rowspan_uv); \
                for(j = 0; j < out_w; j++)                        \
		{

#define TRANSFER_YUV422P_IN_HEAD_16   \
	for(i = 0; i < out_h; i++) \
	{ \
                uint16_t *output_row = (uint16_t *)(output_rows[i]); \
		unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
		unsigned char *input_u = input_rows[1] + row_table[i] * in_rowspan_uv; \
		unsigned char *input_v = input_rows[2] + row_table[i] * in_rowspan_uv; \
		for(j = 0; j < out_w; j++) \
		{

#define TRANSFER_YUV422P16_IN_HEAD_16   \
	for(i = 0; i < out_h; i++) \
	{ \
                uint16_t *output_row = (uint16_t *)(output_rows[i]); \
		uint16_t * input_y = (uint16_t *)(input_rows[0] + row_table[i] * in_rowspan); \
		uint16_t * input_u = (uint16_t *)(input_rows[1] + row_table[i] * in_rowspan_uv); \
		uint16_t * input_v = (uint16_t *)(input_rows[2] + row_table[i] * in_rowspan_uv); \
                for(j = 0; j < out_w; j++)                        \
		{

#define TRANSFER_YUV444P_IN_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		unsigned char *output_row = output_rows[i]; \
		unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
		unsigned char *input_u = input_rows[1] + row_table[i] * in_rowspan_uv; \
		unsigned char *input_v = input_rows[2] + row_table[i] * in_rowspan_uv; \
		for(j = 0; j < out_w; j++) \
		{

#define TRANSFER_YUV444P16_IN_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		unsigned char *output_row = output_rows[i]; \
		uint16_t *input_y = (uint16_t *)(input_rows[0] + row_table[i] * in_rowspan); \
		uint16_t *input_u = (uint16_t *)(input_rows[1] + row_table[i] * in_rowspan_uv); \
		uint16_t *input_v = (uint16_t *)(input_rows[2] + row_table[i] * in_rowspan_uv); \
		for(j = 0; j < out_w; j++) \
		{


#define TRANSFER_YUV444P_IN_HEAD_16   \
	for(i = 0; i < out_h; i++) \
	{ \
                uint16_t *output_row = (uint16_t *)(output_rows[i]); \
		unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
		unsigned char *input_u = input_rows[1] + row_table[i] * in_rowspan_uv; \
		unsigned char *input_v = input_rows[2] + row_table[i] * in_rowspan_uv; \
		for(j = 0; j < out_w; j++) \
		{

#define TRANSFER_YUV444P16_IN_HEAD_16 \
	for(i = 0; i < out_h; i++) \
	{ \
                uint16_t *output_row = (uint16_t *)(output_rows[i]); \
		uint16_t *input_y = (uint16_t *)(input_rows[0] + row_table[i] * in_rowspan); \
		uint16_t *input_u = (uint16_t *)(input_rows[1] + row_table[i] * in_rowspan_uv); \
		uint16_t *input_v = (uint16_t *)(input_rows[2] + row_table[i] * in_rowspan_uv); \
		for(j = 0; j < out_w; j++) \
		{

#define TRANSFER_YUV422_IN_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		unsigned char *output_row = output_rows[i]; \
		unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
		unsigned char *input_u = input_rows[1] + row_table[i] * in_rowspan_uv; \
		unsigned char *input_v = input_rows[2] + row_table[i] * in_rowspan_uv; \
		for(j = 0; j < out_w; j++) \
		{


/*******************************************************************************
 colormodels.h

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
 
#ifndef COLORMODELS_H
#define COLORMODELS_H

#pragma GCC visibility push(default)

/** \defgroup color Color handling

   Libquicktime has a built in colormodel converter, which can do implicit colormodel
   conversions while en-/decoding. It is, however, far from perfect: It is incomplete
   (i.e. not all conversions are present), slow and sometimes inaccurate. Therefore,
   there is a possibility to bypass internal colormodel conversions.
*/

// Colormodels
#define BC_COMPRESSED   1

/** \ingroup color

    16 bit RGB. Each pixel is a uint16_t in native byte order. Color masks are:
    for red: 0xf800, for green: 0x07e0, for blue: 0x001f
*/

#define BC_RGB565       2

/** \ingroup color

    16 bit BGR. Each pixel is a uint16_t in native byte order. Color masks are:
    for red: 0x001f, for green: 0x07e0, for blue: 0xf800
*/

#define BC_BGR565       3

/** \ingroup color

    24 bit BGR. Each color is an uint8_t. Color order is BGR
*/

#define BC_BGR888       4

/** \ingroup color

 32 bit BGR. Each color is an uint8_t. Color order is BGRXBGRX, where X is unused
*/

#define BC_BGR8888      5

/** \ingroup color

    24 bit RGB. Each color is an uint8_t. Color order is RGB
*/


#define BC_RGB888       6

/** \ingroup color

    32 bit RGBA. Each color is an uint8_t. Color order is RGBARGBA
*/

#define BC_RGBA8888     7

/** \ingroup color

    48 bit RGB. Each color is an uint16_t in native byte order. Color order is RGB
*/

#define BC_RGB161616    8

/** \ingroup color

    64 bit RGBA. Each color is an uint16_t in native byte order. Color order is RGBA
*/

#define BC_RGBA16161616 9

/** \ingroup color

    Packed YCbCrA 4:4:4:4. Each component is an uint8_t. Component order is YUVA
*/

#define BC_YUVA8888     10

/** \ingroup color

    Packed YCbCr 4:2:2. Each component is an uint8_t. Component order is Y1 U1 Y2 V1
*/

#define BC_YUV422       13
// Planar

/** \ingroup color

    Planar YCbCr 4:2:0. Each component is an uint8_t. Chroma placement is defined by
    \ref lqt_chroma_placement_t
*/

#define BC_YUV420P      14

/** \ingroup color

    Planar YCbCr 4:2:2. Each component is an uint8_t
*/

#define BC_YUV422P      15

/** \ingroup color

    Planar YCbCr 4:4:4. Each component is an uint8_t
*/

#define BC_YUV444P      16

/** \ingroup color

    Planar YCbCr 4:1:1. Each component is an uint8_t
*/

#define BC_YUV411P      17
/* JPEG scaled colormodels */

/** \ingroup color

    Planar YCbCr 4:2:0. Each component is an uint8_t, luma and chroma values are full range (0x00 .. 0xff)
*/

#define BC_YUVJ420P     18

/** \ingroup color

    Planar YCbCr 4:2:2. Each component is an uint8_t, luma and chroma values are full range (0x00 .. 0xff)
*/

#define BC_YUVJ422P     19

/** \ingroup color

    Planar YCbCr 4:4:4. Each component is an uint8_t, luma and chroma values are full range (0x00 .. 0xff)
*/

#define BC_YUVJ444P     20
/* 16 bit per component planar formats */

/** \ingroup color

    16 bit Planar YCbCr 4:2:2. Each component is an uint16_t in native byte order.
*/

#define BC_YUV422P16    21

/** \ingroup color

    16 bit Planar YCbCr 4:4:4. Each component is an uint16_t in native byte order.
*/

#define BC_YUV444P16    22

/** \ingroup color

    10 bit values in uint16_t native byte order containers, planar YCbCr 4:2:2.
*/
#define BC_YUV422P10    23

/** \ingroup color

    10 bit values in uint16_t native byte order containers, planar YCbCr 4:2:2.
    Luma and chroma values are full range (0 .. 1023)
*/
#define BC_YUVJ422P10   24

// Colormodels purely used by Quicktime are done in Quicktime.

// For communication with the X Server
#define FOURCC_YV12 0x32315659  /* YV12   YUV420P */
#define FOURCC_YUV2 0x32595559  /* YUV2   YUV422 */
#define FOURCC_I420 0x30323449  /* I420   Intel Indeo 4 */

// #undef RECLIP
// #define RECLIP(x, y, z) ((x) = ((x) < (y) ? (y) : ((x) > (z) ? (z) : (x))))

#ifdef __cplusplus
extern "C" {
#endif

int cmodel_calculate_pixelsize(int colormodel);
int cmodel_calculate_datasize(int w, int h, int bytes_per_line, int color_model);
int cmodel_calculate_max(int colormodel);
int cmodel_components(int colormodel);
int cmodel_is_yuv(int colormodel);

void cmodel_transfer(unsigned char **output_rows, /* Leave NULL if non existent */
	unsigned char **input_rows,
	int in_x,        /* Dimensions to capture from input frame */
	int in_y, 
	int in_w, 
	int in_h,
	int out_w, 
	int out_h,
	int in_colormodel, 
	int out_colormodel,
	int in_rowspan,       /* For planar use the luma rowspan */
        int out_rowspan,      /* For planar use the luma rowspan */
        int in_rowspan_uv,    /* Chroma rowspan */
        int out_rowspan_uv    /* Chroma rowspan */);     

int cmodel_bc_to_x(int color_model);
// Tell when to use plane arguments or row pointer arguments to functions
int cmodel_is_planar(int color_model);





#ifdef __cplusplus
}
#endif

#pragma GCC visibility pop

#endif

/*******************************************************************************
 colormodels.c

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

#include "lqt_private.h"
#include <quicktime/colormodels.h>
#include <stdlib.h>

int cmodel_is_planar(int colormodel)
{
	switch(colormodel)
	{
		case BC_YUV420P:      return 1; break;
		case BC_YUV422P:      return 1; break;
        case BC_YUV444P:      return 1; break;
		case BC_YUV422P16:    return 1; break;
        case BC_YUV444P16:    return 1; break;
		case BC_YUVJ420P:     return 1; break;
		case BC_YUVJ422P:     return 1; break;
        case BC_YUVJ444P:     return 1; break;
		case BC_YUV411P:      return 1; break;
        case BC_YUV422P10:    return 1; break;
        case BC_YUVJ422P10:   return 1; break;
	}
	return 0;
}

int cmodel_calculate_pixelsize(int colormodel)
{
	switch(colormodel)
	{
		case BC_COMPRESSED:   return 1; break;
		case BC_RGB565:       return 2; break;
		case BC_BGR565:       return 2; break;
		case BC_BGR888:       return 3; break;
		case BC_BGR8888:      return 4; break;
// Working bitmaps are packed to simplify processing
		case BC_RGB888:       return 3; break;
		case BC_RGBA8888:     return 4; break;
		case BC_RGB161616:    return 6; break;
		case BC_RGBA16161616: return 8; break;
		case BC_YUVA8888:     return 4; break;
// Planar
		case BC_YUV420P:      return 1; break;
		case BC_YUV422P:      return 1; break;
        case BC_YUV444P:      return 1; break;
		case BC_YUVJ420P:     return 1; break;
		case BC_YUVJ422P:     return 1; break;
        case BC_YUVJ444P:     return 1; break;
		case BC_YUV422P16:    return 2; break;
        case BC_YUV444P16:    return 2; break;
        case BC_YUV422:       return 2; break;
		case BC_YUV411P:      return 1; break;
        case BC_YUV422P10:    return 2; break;
        case BC_YUVJ422P10:   return 2; break;
	}
	return 0;
}

static void get_scale_tables(int **column_table, 
	int **row_table, 
	int in_x1, 
	int in_y1, 
	int in_x2, 
	int in_y2,
	int out_x1, 
	int out_y1, 
	int out_x2, 
	int out_y2)
{
	int i;
	float w_in = in_x2 - in_x1;
	float h_in = in_y2 - in_y1;
	int w_out = out_x2 - out_x1;
	int h_out = out_y2 - out_y1;

	float hscale = w_in / w_out;
	float vscale = h_in / h_out;

	(*column_table) = malloc(sizeof(int) * w_out);
	(*row_table) = malloc(sizeof(int) * h_out);
	for(i = 0; i < w_out; i++)
	{
		(*column_table)[i] = (int)(hscale * i) + in_x1;
	}

	for(i = 0; i < h_out; i++)
	{
		(*row_table)[i] = (int)(vscale * i) + in_y1;
	}
}

void cmodel_transfer(unsigned char **output_rows, 
	unsigned char **input_rows,
	int in_x, 
	int in_y, 
	int in_w, 
	int in_h,
	int out_w, 
	int out_h,
	int in_colormodel, 
	int out_colormodel,
	int in_rowspan,
        int out_rowspan,
	int in_rowspan_uv,
	int out_rowspan_uv)
{
	int *column_table;
	int *row_table;
	int scale;
	int in_pixelsize = cmodel_calculate_pixelsize(in_colormodel);
	int out_pixelsize = cmodel_calculate_pixelsize(out_colormodel);

// Get scaling
	scale = (out_w != in_w) || (in_x != 0);
	get_scale_tables(&column_table, &row_table, 
		in_x, in_y, in_x + in_w, in_y + in_h,
		0, 0, out_w, out_h);


// Handle planar cmodels separately
	switch(in_colormodel)
	{
		case BC_YUV420P:
		case BC_YUV422P:
		case BC_YUV422P16:
		case BC_YUVJ420P:
		case BC_YUVJ422P:
        case BC_YUV422P10:
        case BC_YUVJ422P10:
 			cmodel_yuv420p(output_rows,  \
				input_rows, \
				in_x,  \
				in_y,  \
				in_w,  \
				in_h, \
				out_w,  \
				out_h, \
				in_colormodel,  \
				out_colormodel, \
				in_rowspan, \
				out_rowspan, \
				in_rowspan_uv, \
				out_rowspan_uv, \
				scale, \
				out_pixelsize, \
				in_pixelsize, \
				row_table, \
				column_table); \
			break; \
		case BC_YUV411P:
 			cmodel_yuv411p(output_rows,  \
				input_rows, \
				in_x,  \
				in_y,  \
				in_w,  \
				in_h, \
				out_w,  \
				out_h, \
				in_colormodel,  \
				out_colormodel, \
				in_rowspan, \
				out_rowspan, \
				in_rowspan_uv, \
				out_rowspan_uv, \
				scale, \
				out_pixelsize, \
				in_pixelsize, \
				row_table, \
				column_table); \
			break; \
                case BC_YUV444P:
                case BC_YUV444P16:
                case BC_YUVJ444P:
                        cmodel_yuv444p(output_rows,  \
                                input_rows, \
                                in_x,  \
                                in_y,  \
                                in_w,  \
                                in_h, \
                                out_w,  \
                                out_h, \
                                in_colormodel,  \
                                out_colormodel, \
                                in_rowspan, \
                                out_rowspan, \
                                in_rowspan_uv, \
                                out_rowspan_uv, \
                                scale, \
                                out_pixelsize, \
                                in_pixelsize, \
                                row_table, \
                                column_table); \
                        break;

		case BC_YUV422:
			cmodel_yuv422(output_rows,  \
				input_rows, \
				in_x,  \
				in_y,  \
				in_w,  \
				in_h, \
				out_w,  \
				out_h, \
				in_colormodel,  \
				out_colormodel, \
				in_rowspan, \
				out_rowspan, \
				in_rowspan_uv, \
				out_rowspan_uv, \
				scale, \
				out_pixelsize, \
				in_pixelsize, \
				row_table, \
				column_table);
			break;

		default:
			cmodel_default(output_rows,  \
				input_rows, \
				in_x,  \
				in_y,  \
				in_w,  \
				in_h, \
				out_w,  \
				out_h, \
				in_colormodel,  \
				out_colormodel, \
				in_rowspan, \
				out_rowspan, \
				in_rowspan_uv, \
				out_rowspan_uv, \
				scale, \
				out_pixelsize, \
				in_pixelsize, \
				row_table, \
				column_table);
			break;
	}

	free(column_table);
	free(row_table);
}

int cmodel_bc_to_x(int color_model)
{
	switch(color_model)
	{
		case BC_YUV420P:
			return FOURCC_YV12;
			break;
		case BC_YUV422:
			return FOURCC_YUV2;
			break;
	}
	return -1;
}


int cmodel_is_yuv(int colormodel)
{
	switch(colormodel)
	{
		case BC_YUVA8888:
		case BC_YUV422:
		case BC_YUVJ422P:
		case BC_YUV420P:
		case BC_YUV422P:
        case BC_YUV444P:
        case BC_YUV411P:
        case BC_YUV422P16:
        case BC_YUV444P16:
        case BC_YUV422P10:
        case BC_YUVJ422P10:
			return 1;
			break;
		default:
			return 0;
			break;
	}
}






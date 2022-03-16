/*******************************************************************************
 cmodel_yuv420p.c

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

#define HAVE_RGB_16_TO_RGB_24
#define HAVE_RGB_TO_YUV
#define HAVE_RGB_TO_YUVJ
#define HAVE_YUV_TO_RGB
#define HAVE_YUVJ_TO_RGB
#define HAVE_YUV_8_TO_YUVJ
#define HAVE_YUVJ_TO_YUV_8

#include "lqt_private.h"
#include "cmodel_permutation.h"

#define TRANSFER_FRAME_DEFAULT(output, \
	y_in_offset, \
	u_in_offset, \
	v_in_offset, \
	input_column) \
{ \
	register int i, j; \
 \
	switch(in_colormodel) \
	{ \
		case BC_YUV420P: \
			switch(out_colormodel) \
			{ \
				case BC_BGR565: \
					TRANSFER_YUV420P_IN_HEAD_16 \
                                          transfer_YUV422P_to_BGR565((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB565: \
					TRANSFER_YUV420P_IN_HEAD_16 \
					transfer_YUV422P_to_RGB565((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888:      \
					TRANSFER_YUV420P_IN_HEAD \
					transfer_YUV422P_to_BGR888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_YUV420P_IN_HEAD \
					transfer_YUV422P_to_BGR8888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV420P: \
					for(i = 0; i < out_h; i++) \
					{ \
						unsigned char *output_y = output_rows[0] + i * out_rowspan; \
						unsigned char *output_u = output_rows[1] + i / 2 * out_rowspan_uv; \
						unsigned char *output_v = output_rows[2] + i / 2 * out_rowspan_uv; \
						unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
						unsigned char *input_u = input_rows[1] + row_table[i] / 2 * in_rowspan_uv; \
						unsigned char *input_v = input_rows[2] + row_table[i] / 2 * in_rowspan_uv; \
						for(j = 0; j < out_w; j++) \
						{ \
							transfer_YUV422P_to_YUV420P(input_y + (y_in_offset), \
								input_u + (u_in_offset), \
								input_v + (v_in_offset), \
								output_y, \
								output_u, \
								output_v, \
								j); \
						} \
					} \
					break; \
				case BC_YUV422: \
					TRANSFER_YUV420P_IN_HEAD \
					transfer_YUV422P_to_YUV422((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P: \
					for(i = 0; i < out_h; i++) \
					{ \
						unsigned char *output_y = output_rows[0] + i * out_rowspan; \
						unsigned char *output_u = output_rows[1] + i * out_rowspan_uv; \
						unsigned char *output_v = output_rows[2] + i * out_rowspan_uv; \
						unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
						unsigned char *input_u = input_rows[1] + row_table[i] / 2 * in_rowspan_uv; \
						unsigned char *input_v = input_rows[2] + row_table[i] / 2 * in_rowspan_uv; \
						for(j = 0; j < out_w; j++) \
						{ \
							transfer_YUV422P_to_YUV420P(input_y + (y_in_offset), \
								input_u + (u_in_offset), \
								input_v + (v_in_offset), \
								output_y, \
								output_u, \
								output_v, \
								j); \
						} \
					} \
					break; \
				case BC_YUV444P: \
					for(i = 0; i < out_h; i++) \
					{ \
						unsigned char *output_y = output_rows[0] + i * out_rowspan; \
						unsigned char *output_u = output_rows[1] + i * out_rowspan_uv; \
						unsigned char *output_v = output_rows[2] + i * out_rowspan_uv; \
						unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
						unsigned char *input_u = input_rows[1] + row_table[i] / 2 * in_rowspan_uv; \
						unsigned char *input_v = input_rows[2] + row_table[i] / 2 * in_rowspan_uv; \
						for(j = 0; j < out_w; j++) \
						{ \
							transfer_YUV422P_to_YUV444P(input_y + (y_in_offset), \
								input_u + (u_in_offset), \
								input_v + (v_in_offset), \
								output_y, \
								output_u, \
								output_v, \
								j); \
						} \
					} \
					break; \
				case BC_RGB888:      \
					TRANSFER_YUV420P_IN_HEAD \
					transfer_YUV422P_to_RGB888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888:      \
					TRANSFER_YUV420P_IN_HEAD \
					transfer_YUV422P_to_RGBA8888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB161616:      \
					TRANSFER_YUV420P_IN_HEAD_16 \
                                        transfer_YUV422P_to_RGB161616((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA16161616:      \
					TRANSFER_YUV420P_IN_HEAD_16 \
                                                transfer_YUV422P_to_RGBA16161616((output), \
                                                input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA8888: \
					TRANSFER_YUV420P_IN_HEAD \
					transfer_YUV422P_to_YUVA8888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
			} \
			break; \
		case BC_YUV411P: \
			switch(out_colormodel) \
			{ \
				case BC_RGB888:      \
					TRANSFER_YUV411P_IN_HEAD \
					transfer_YUV411P_to_RGB888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888:      \
					TRANSFER_YUV411P_IN_HEAD \
					transfer_YUV411P_to_RGBA8888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
			} \
			break; \
 \
		case BC_YUVJ444P: \
			switch(out_colormodel) \
			{ \
				case BC_RGB888:      \
					TRANSFER_YUV444P_IN_HEAD \
					transfer_YUVJ444P_to_RGB888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888:      \
					TRANSFER_YUV444P_IN_HEAD \
					transfer_YUVJ444P_to_RGBA8888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVJ444P:      \
					for(i = 0; i < out_h; i++) \
					{ \
						unsigned char *output_y = output_rows[0] + i * out_rowspan; \
						unsigned char *output_u = output_rows[1] + i * out_rowspan_uv; \
						unsigned char *output_v = output_rows[2] + i * out_rowspan_uv; \
						unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
						unsigned char *input_u = input_rows[1] + row_table[i] * in_rowspan_uv; \
						unsigned char *input_v = input_rows[2] + row_table[i] * in_rowspan_uv; \
						for(j = 0; j < out_w; j++) \
						{ \
							transfer_YUV444P_to_YUV444P(input_y + (y_in_offset), \
								input_u + (u_in_offset), \
								input_v + (v_in_offset), \
								output_y, \
								output_u, \
								output_v, \
								j); \
						} \
					} \
					break; \
			} \
			break; \
 \
		case BC_YUVJ422P: \
			switch(out_colormodel) \
			{ \
				case BC_RGB888:      \
					TRANSFER_YUV422P_IN_HEAD \
					transfer_YUVJ422P_to_RGB888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888:      \
					TRANSFER_YUV422P_IN_HEAD \
					transfer_YUVJ422P_to_RGBA8888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVJ422P: \
					for(i = 0; i < out_h; i++) \
					{ \
						unsigned char *output_y = output_rows[0] + i * out_rowspan; \
						unsigned char *output_u = output_rows[1] + i * out_rowspan_uv; \
						unsigned char *output_v = output_rows[2] + i * out_rowspan_uv; \
						unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
						unsigned char *input_u = input_rows[1] + row_table[i] * in_rowspan_uv; \
						unsigned char *input_v = input_rows[2] + row_table[i] * in_rowspan_uv; \
						for(j = 0; j < out_w; j++) \
						{ \
							transfer_YUV422P_to_YUV420P(input_y + (y_in_offset), \
								input_u + (u_in_offset), \
								input_v + (v_in_offset), \
								output_y, \
								output_u, \
								output_v, \
								j); \
						} \
					} \
					break; \
                               case BC_YUV420P: \
                                       for(i = 0; i < out_h; i++) \
                                       { \
                                                unsigned char *output_y = output_rows[0] + i * out_rowspan; \
                                                unsigned char *output_u = output_rows[1] + i / 2 * out_rowspan_uv; \
                                                unsigned char *output_v = output_rows[2] + i / 2 * out_rowspan_uv; \
                                                unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
                                                unsigned char *input_u = input_rows[1] + row_table[i] * in_rowspan_uv; \
                                                unsigned char *input_v = input_rows[2] + row_table[i] * in_rowspan_uv; \
                                                for(j = 0; j < out_w; j++) \
                                                { \
                                                        transfer_YUVJ422P_to_YUV420P(input_y + (y_in_offset), \
                                                                input_u + (u_in_offset), \
                                                                input_v + (v_in_offset), \
                                                                output_y, \
                                                                output_u, \
                                                                output_v, \
                                                                j); \
                                                } \
                                        } \
                                        break; \
				case BC_YUV422P: \
					for(i = 0; i < out_h; i++) \
					{ \
						unsigned char *output_y = output_rows[0] + i * out_rowspan; \
						unsigned char *output_u = output_rows[1] + i * out_rowspan_uv; \
						unsigned char *output_v = output_rows[2] + i * out_rowspan_uv; \
						unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
						unsigned char *input_u = input_rows[1] + row_table[i] * in_rowspan_uv; \
						unsigned char *input_v = input_rows[2] + row_table[i] * in_rowspan_uv; \
						for(j = 0; j < out_w; j++) \
						{ \
							transfer_YUVJ422P_to_YUV420P(input_y + (y_in_offset), \
								input_u + (u_in_offset), \
								input_v + (v_in_offset), \
								output_y, \
								output_u, \
								output_v, \
								j); \
						} \
					} \
					break; \
				case BC_YUV422:      \
					TRANSFER_YUV422_IN_HEAD \
					transfer_YUVJ422P_to_YUV422((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
			} \
			break; \
 \
		case BC_YUVJ420P: \
			switch(out_colormodel) \
			{ \
				case BC_RGB888:      \
					TRANSFER_YUV420P_IN_HEAD \
					transfer_YUVJ420P_to_RGB888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888:      \
					TRANSFER_YUV420P_IN_HEAD \
					transfer_YUVJ420P_to_RGBA8888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVJ420P: \
					for(i = 0; i < out_h; i++) \
					{ \
						unsigned char *output_y = output_rows[0] + i * out_rowspan; \
						unsigned char *output_u = output_rows[1] + i / 2 * out_rowspan_uv; \
						unsigned char *output_v = output_rows[2] + i / 2 * out_rowspan_uv; \
						unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
						unsigned char *input_u = input_rows[1] + row_table[i] / 2 * in_rowspan_uv; \
						unsigned char *input_v = input_rows[2] + row_table[i] / 2 * in_rowspan_uv; \
						for(j = 0; j < out_w; j++) \
						{ \
							transfer_YUV422P_to_YUV420P(input_y + (y_in_offset), \
								input_u + (u_in_offset), \
								input_v + (v_in_offset), \
								output_y, \
								output_u, \
								output_v, \
								j); \
						} \
					} \
					break; \
			} \
			break; \
 \
		case BC_YUV422P: \
			switch(out_colormodel) \
			{ \
				case BC_BGR565: \
					TRANSFER_YUV422P_IN_HEAD_16 \
					transfer_YUV422P_to_BGR565((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB565: \
					TRANSFER_YUV422P_IN_HEAD_16 \
					transfer_YUV422P_to_RGB565((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888:      \
					TRANSFER_YUV422P_IN_HEAD \
					transfer_YUV422P_to_BGR888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_YUV422P_IN_HEAD \
					transfer_YUV422P_to_BGR8888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888:      \
					TRANSFER_YUV422P_IN_HEAD \
					transfer_YUV422P_to_RGB888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888:      \
					TRANSFER_YUV422P_IN_HEAD \
					transfer_YUV422P_to_RGBA8888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB161616:      \
					TRANSFER_YUV422P_IN_HEAD_16 \
					transfer_YUV422P_to_RGB161616((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA16161616:      \
					TRANSFER_YUV422P_IN_HEAD_16 \
					transfer_YUV422P_to_RGBA16161616((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA8888: \
					TRANSFER_YUV422P_IN_HEAD \
					transfer_YUV422P_to_YUVA8888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV420P: \
					for(i = 0; i < out_h; i++) \
					{ \
						unsigned char *output_y = output_rows[0] + i * out_rowspan; \
						unsigned char *output_u = output_rows[1] + i / 2 * out_rowspan_uv; \
						unsigned char *output_v = output_rows[2] + i / 2 * out_rowspan_uv; \
						unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
						unsigned char *input_u = input_rows[1] + row_table[i] * in_rowspan_uv; \
						unsigned char *input_v = input_rows[2] + row_table[i] * in_rowspan_uv; \
						for(j = 0; j < out_w; j++) \
						{ \
							transfer_YUV422P_to_YUV420P(input_y + (y_in_offset), \
								input_u + (u_in_offset), \
								input_v + (v_in_offset), \
								output_y, \
								output_u, \
								output_v, \
								j); \
						} \
					} \
					break; \
				case BC_YUV422: \
					TRANSFER_YUV422_IN_HEAD \
					transfer_YUV422P_to_YUV422((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P: \
					for(i = 0; i < out_h; i++) \
					{ \
						unsigned char *output_y = output_rows[0] + i * out_rowspan; \
						unsigned char *output_u = output_rows[1] + i * out_rowspan_uv; \
						unsigned char *output_v = output_rows[2] + i * out_rowspan_uv; \
						unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
						unsigned char *input_u = input_rows[1] + row_table[i] * in_rowspan_uv; \
						unsigned char *input_v = input_rows[2] + row_table[i] * in_rowspan_uv; \
						for(j = 0; j < out_w; j++) \
						{ \
							transfer_YUV422P_to_YUV420P(input_y + (y_in_offset), \
								input_u + (u_in_offset), \
								input_v + (v_in_offset), \
								output_y, \
								output_u, \
								output_v, \
								j); \
						} \
					} \
					break; \
				case BC_YUV444P: \
					for(i = 0; i < out_h; i++) \
					{ \
						unsigned char *output_y = output_rows[0] + i * out_rowspan; \
						unsigned char *output_u = output_rows[1] + i * out_rowspan_uv; \
						unsigned char *output_v = output_rows[2] + i * out_rowspan_uv; \
						unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
						unsigned char *input_u = input_rows[1] + row_table[i] * in_rowspan_uv; \
						unsigned char *input_v = input_rows[2] + row_table[i] * in_rowspan_uv; \
						for(j = 0; j < out_w; j++) \
						{ \
							transfer_YUV422P_to_YUV444P(input_y + (y_in_offset), \
								input_u + (u_in_offset), \
								input_v + (v_in_offset), \
								output_y, \
								output_u, \
								output_v, \
								j); \
						} \
					} \
					break; \
				case BC_YUVJ422P: \
					for(i = 0; i < out_h; i++) \
					{ \
						unsigned char *output_y = output_rows[0] + i * out_rowspan; \
						unsigned char *output_u = output_rows[1] + i * out_rowspan_uv; \
						unsigned char *output_v = output_rows[2] + i * out_rowspan_uv; \
						unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
						unsigned char *input_u = input_rows[1] + row_table[i] * in_rowspan_uv; \
						unsigned char *input_v = input_rows[2] + row_table[i] * in_rowspan_uv; \
						for(j = 0; j < out_w; j++) \
						{ \
							transfer_YUV422P_to_YUVJ420P(input_y + (y_in_offset), \
								input_u + (u_in_offset), \
								input_v + (v_in_offset), \
								output_y, \
								output_u, \
								output_v, \
								j); \
						} \
					} \
					break; \
			} \
			break; \
 \
		case BC_YUV422P16: \
			switch(out_colormodel) \
			{ \
				case BC_RGB888:      \
					TRANSFER_YUV422P16_IN_HEAD \
					transfer_YUV422P16_to_RGB888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888:      \
					TRANSFER_YUV422P16_IN_HEAD \
					transfer_YUV422P16_to_RGBA8888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB161616:      \
					TRANSFER_YUV422P16_IN_HEAD_16 \
					transfer_YUV422P16_to_RGB161616((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
      				case BC_YUV422P16: \
					for(i = 0; i < out_h; i++) \
					{ \
                                        uint16_t *output_y = (uint16_t *)(output_rows[0] + i * out_rowspan); \
                                        uint16_t *output_u = (uint16_t *)(output_rows[1] + i * out_rowspan_uv); \
                                        uint16_t *output_v = (uint16_t *)(output_rows[2] + i * out_rowspan_uv); \
                                        uint16_t *input_y = (uint16_t *)(input_rows[0] + row_table[i] * in_rowspan); \
                                        uint16_t *input_u = (uint16_t *)(input_rows[1] + row_table[i] * in_rowspan_uv); \
                                        uint16_t *input_v = (uint16_t *)(input_rows[2] + row_table[i] * in_rowspan_uv); \
						for(j = 0; j < out_w; j++) \
						{ \
							transfer_YUV422P16_to_YUV420P16(input_y + (y_in_offset), \
								input_u + (u_in_offset), \
								input_v + (v_in_offset), \
								output_y, \
								output_u, \
								output_v, \
								j); \
						} \
					} \
					break; \
      				case BC_YUV422P: \
					for(i = 0; i < out_h; i++) \
					{ \
                                        uint8_t *output_y = (uint8_t *)(output_rows[0] + i * out_rowspan); \
                                        uint8_t *output_u = (uint8_t *)(output_rows[1] + i * out_rowspan_uv); \
                                        uint8_t *output_v = (uint8_t *)(output_rows[2] + i * out_rowspan_uv); \
                                        uint16_t *input_y = (uint16_t *)(input_rows[0] + row_table[i] * in_rowspan); \
                                        uint16_t *input_u = (uint16_t *)(input_rows[1] + row_table[i] * in_rowspan_uv); \
                                        uint16_t *input_v = (uint16_t *)(input_rows[2] + row_table[i] * in_rowspan_uv); \
						for(j = 0; j < out_w; j++) \
						{ \
							transfer_YUV422P16_to_YUV420P(input_y + (y_in_offset), \
								input_u + (u_in_offset), \
								input_v + (v_in_offset), \
								output_y, \
								output_u, \
								output_v, \
								j); \
						} \
					} \
					break; \
                        }                              \
			break; \
        case BC_YUV422P10: \
            switch(out_colormodel) \
            { \
                case BC_RGB888:      \
                    TRANSFER_YUV422P16_IN_HEAD \
                    transfer_YUV422P10_to_RGB888((output), \
                        input_y + (y_in_offset), \
                        input_u + (u_in_offset), \
                        input_v + (v_in_offset)); \
                    TRANSFER_FRAME_TAIL \
                    break; \
                case BC_RGB161616:      \
                    TRANSFER_YUV422P16_IN_HEAD_16 \
                    transfer_YUV422P10_to_RGB161616((output), \
                        input_y + (y_in_offset), \
                        input_u + (u_in_offset), \
                        input_v + (v_in_offset)); \
                    TRANSFER_FRAME_TAIL \
                    break; \
                case BC_YUV422P10: \
                    for(i = 0; i < out_h; i++) \
                    { \
                        uint16_t *output_y = (uint16_t *)(output_rows[0] + i * out_rowspan); \
                        uint16_t *output_u = (uint16_t *)(output_rows[1] + i * out_rowspan_uv); \
                        uint16_t *output_v = (uint16_t *)(output_rows[2] + i * out_rowspan_uv); \
                        uint16_t *input_y = (uint16_t *)(input_rows[0] + row_table[i] * in_rowspan); \
                        uint16_t *input_u = (uint16_t *)(input_rows[1] + row_table[i] * in_rowspan_uv); \
                        uint16_t *input_v = (uint16_t *)(input_rows[2] + row_table[i] * in_rowspan_uv); \
                        for(j = 0; j < out_w; j++) \
                        { \
                            transfer_YUV422P16_to_YUV420P16(input_y + (y_in_offset), \
                                input_u + (u_in_offset), \
                                input_v + (v_in_offset), \
                                output_y, \
                                output_u, \
                                output_v, \
                                j); \
                        } \
                    } \
                    break; \
            }                              \
            break; \
        case BC_YUVJ422P10: \
            switch(out_colormodel) \
            { \
                case BC_RGB888:      \
                    TRANSFER_YUV422P16_IN_HEAD \
                    transfer_YUVJ422P10_to_RGB888((output), \
                        input_y + (y_in_offset), \
                        input_u + (u_in_offset), \
                        input_v + (v_in_offset)); \
                    TRANSFER_FRAME_TAIL \
                    break; \
                case BC_RGB161616:      \
                    TRANSFER_YUV422P16_IN_HEAD_16 \
                    transfer_YUVJ422P10_to_RGB161616((output), \
                        input_y + (y_in_offset), \
                        input_u + (u_in_offset), \
                        input_v + (v_in_offset)); \
                    TRANSFER_FRAME_TAIL \
                    break; \
                case BC_YUVJ422P10: \
                    for(i = 0; i < out_h; i++) \
                    { \
                        uint16_t *output_y = (uint16_t *)(output_rows[0] + i * out_rowspan); \
                        uint16_t *output_u = (uint16_t *)(output_rows[1] + i * out_rowspan_uv); \
                        uint16_t *output_v = (uint16_t *)(output_rows[2] + i * out_rowspan_uv); \
                        uint16_t *input_y = (uint16_t *)(input_rows[0] + row_table[i] * in_rowspan); \
                        uint16_t *input_u = (uint16_t *)(input_rows[1] + row_table[i] * in_rowspan_uv); \
                        uint16_t *input_v = (uint16_t *)(input_rows[2] + row_table[i] * in_rowspan_uv); \
                        for(j = 0; j < out_w; j++) \
                        { \
                            transfer_YUV422P16_to_YUV420P16(input_y + (y_in_offset), \
                                input_u + (u_in_offset), \
                                input_v + (v_in_offset), \
                                output_y, \
                                output_u, \
                                output_v, \
                                j); \
                        } \
                    } \
                    break; \
            }                              \
            break; \
		case BC_YUV444P16: \
			switch(out_colormodel) \
			{ \
				case BC_RGB888:      \
					TRANSFER_YUV444P16_IN_HEAD \
					transfer_YUV444P16_to_RGB888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888:      \
					TRANSFER_YUV444P16_IN_HEAD \
					transfer_YUV444P16_to_RGBA8888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB161616:      \
					TRANSFER_YUV444P16_IN_HEAD_16 \
					transfer_YUV444P16_to_RGB161616((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
                                case BC_YUV444P16:                      \
					for(i = 0; i < out_h; i++) \
					{ \
                                        uint16_t *output_y = (uint16_t *)(output_rows[0] + i * out_rowspan); \
                                        uint16_t *output_u = (uint16_t *)(output_rows[1] + i * out_rowspan_uv); \
                                        uint16_t *output_v = (uint16_t *)(output_rows[2] + i * out_rowspan_uv); \
                                        uint16_t *input_y = (uint16_t *)(input_rows[0] + row_table[i] * in_rowspan); \
                                        uint16_t *input_u = (uint16_t *)(input_rows[1] + row_table[i] * in_rowspan_uv); \
                                        uint16_t *input_v = (uint16_t *)(input_rows[2] + row_table[i] * in_rowspan_uv); \
						for(j = 0; j < out_w; j++) \
						{ \
							transfer_YUV444P16_to_YUV444P16(input_y + (y_in_offset), \
								input_u + (u_in_offset), \
								input_v + (v_in_offset), \
								output_y, \
								output_u, \
								output_v, \
								j); \
						} \
					} \
					break; \
                                case BC_YUV444P:                      \
					for(i = 0; i < out_h; i++) \
					{ \
                                        uint8_t *output_y = (uint8_t *)(output_rows[0] + i * out_rowspan); \
                                        uint8_t *output_u = (uint8_t *)(output_rows[1] + i * out_rowspan_uv); \
                                        uint8_t *output_v = (uint8_t *)(output_rows[2] + i * out_rowspan_uv); \
                                        uint16_t *input_y = (uint16_t *)(input_rows[0] + row_table[i] * in_rowspan); \
                                        uint16_t *input_u = (uint16_t *)(input_rows[1] + row_table[i] * in_rowspan_uv); \
                                        uint16_t *input_v = (uint16_t *)(input_rows[2] + row_table[i] * in_rowspan_uv); \
						for(j = 0; j < out_w; j++) \
						{ \
							transfer_YUV444P16_to_YUV444P(input_y + (y_in_offset), \
								input_u + (u_in_offset), \
								input_v + (v_in_offset), \
								output_y, \
								output_u, \
								output_v, \
								j); \
						} \
					} \
					break; \
			} \
			break; \
		case BC_YUV444P: \
			switch(out_colormodel) \
			{ \
				case BC_BGR565: \
					TRANSFER_YUV444P_IN_HEAD_16 \
					transfer_YUV422P_to_BGR565((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB565: \
					TRANSFER_YUV444P_IN_HEAD_16 \
					transfer_YUV422P_to_RGB565((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888:      \
					TRANSFER_YUV444P_IN_HEAD \
					transfer_YUV422P_to_BGR888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_YUV444P_IN_HEAD \
					transfer_YUV422P_to_BGR8888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888:      \
					TRANSFER_YUV444P_IN_HEAD \
					transfer_YUV422P_to_RGB888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888:      \
					TRANSFER_YUV444P_IN_HEAD \
					transfer_YUV422P_to_RGBA8888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB161616:      \
					TRANSFER_YUV444P_IN_HEAD_16 \
					transfer_YUV422P_to_RGB161616((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA16161616:      \
					TRANSFER_YUV444P_IN_HEAD_16 \
					transfer_YUV422P_to_RGBA16161616((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA8888: \
					TRANSFER_YUV444P_IN_HEAD \
					transfer_YUV422P_to_YUVA8888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV420P: \
					for(i = 0; i < out_h; i++) \
					{ \
						unsigned char *output_y = output_rows[0] + i * out_rowspan; \
						unsigned char *output_u = output_rows[1] + i / 2 * out_rowspan_uv; \
						unsigned char *output_v = output_rows[2] + i / 2 * out_rowspan_uv; \
						unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
						unsigned char *input_u = input_rows[1] + row_table[i] * in_rowspan_uv; \
						unsigned char *input_v = input_rows[2] + row_table[i] * in_rowspan_uv; \
						for(j = 0; j < out_w; j++) \
						{ \
							transfer_YUV422P_to_YUV420P(input_y + (y_in_offset), \
								input_u + (u_in_offset), \
								input_v + (v_in_offset), \
								output_y, \
								output_u, \
								output_v, \
								j); \
						} \
					} \
					break; \
				case BC_YUV422: \
					TRANSFER_YUV444P_IN_HEAD \
					transfer_YUV422P_to_YUV422((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P: \
					for(i = 0; i < out_h; i++) \
					{ \
						unsigned char *output_y = output_rows[0] + i * out_rowspan; \
						unsigned char *output_u = output_rows[1] + i * out_rowspan_uv; \
						unsigned char *output_v = output_rows[2] + i * out_rowspan_uv; \
						unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
						unsigned char *input_u = input_rows[1] + row_table[i] * in_rowspan_uv; \
						unsigned char *input_v = input_rows[2] + row_table[i] * in_rowspan_uv; \
						for(j = 0; j < out_w; j++) \
						{ \
							transfer_YUV422P_to_YUV420P(input_y + (y_in_offset), \
								input_u + (u_in_offset), \
								input_v + (v_in_offset), \
								output_y, \
								output_u, \
								output_v, \
								j); \
						} \
					} \
					break; \
				case BC_YUV444P: \
					for(i = 0; i < out_h; i++) \
					{ \
						unsigned char *output_y = output_rows[0] + i * out_rowspan; \
						unsigned char *output_u = output_rows[1] + i * out_rowspan_uv; \
						unsigned char *output_v = output_rows[2] + i * out_rowspan_uv; \
						unsigned char *input_y = input_rows[0] + row_table[i] * in_rowspan; \
						unsigned char *input_u = input_rows[1] + row_table[i] * in_rowspan_uv; \
						unsigned char *input_v = input_rows[2] + row_table[i] * in_rowspan_uv; \
						for(j = 0; j < out_w; j++) \
						{ \
							transfer_YUV444P_to_YUV444P(input_y + (y_in_offset), \
								input_u + (u_in_offset), \
								input_v + (v_in_offset), \
								output_y, \
								output_u, \
								output_v, \
								j); \
						} \
					} \
					break; \
			} \
			break; \
	} \
}

void cmodel_yuv420p(PERMUTATION_ARGS)
{
	if(scale)
	{
		TRANSFER_FRAME_DEFAULT(&output_row, 
			column_table[j],
			column_table[j] / 2,
			column_table[j] / 2,
			0);
	}
	else
	{
		TRANSFER_FRAME_DEFAULT(&output_row, 
			j,
			j / 2,
			j / 2,
			0);
	}
}

void cmodel_yuv411p(PERMUTATION_ARGS)
{
	if(scale)
	{
		TRANSFER_FRAME_DEFAULT(&output_row, 
			column_table[j],
			column_table[j] / 4,
			column_table[j] / 4,
			0);
	}
	else
	{
		TRANSFER_FRAME_DEFAULT(&output_row, 
			j,
			j / 4,
			j / 4,
			0);
	}
}


void cmodel_yuv444p(PERMUTATION_ARGS)
{
	if(scale)
	{
		TRANSFER_FRAME_DEFAULT(&output_row, 
			column_table[j],
			column_table[j],
			column_table[j],
			0);
	}
	else
	{
		TRANSFER_FRAME_DEFAULT(&output_row, 
			j,
			j,
			j,
			0);
	}
}

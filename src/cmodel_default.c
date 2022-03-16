/*******************************************************************************
 cmodel_default.c

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
	input, \
	y_in_offset, \
	u_in_offset, \
	v_in_offset, \
	input_column) \
{ \
	register int i, j; \
 \
	switch(in_colormodel) \
	{ \
		case BC_RGB565: \
			switch(out_colormodel) \
			{ \
				case BC_RGB565: \
					TRANSFER_FRAME_HEAD_16 \
                                          transfer_RGB565_to_RGB565((output), (uint16_t*)(input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
                                          transfer_RGB565_to_RGB888((output), (uint16_t*)(input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
                                          transfer_RGB565_to_RGBA8888((output), (uint16_t*)(input)); \
					TRANSFER_FRAME_TAIL \
					break; \
                        } \
                        break; \
		case BC_BGR565: \
			switch(out_colormodel) \
			{ \
				case BC_BGR565: \
					TRANSFER_FRAME_HEAD_16 \
                                          transfer_BGR565_to_BGR565((output), (uint16_t*)(input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
                                          transfer_BGR565_to_RGB888((output), (uint16_t*)(input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
                                          transfer_BGR565_to_RGBA8888((output), (uint16_t*)(input)); \
					TRANSFER_FRAME_TAIL \
					break; \
                        } \
                        break; \
		case BC_YUVA8888: \
			switch(out_colormodel) \
			{ \
				case BC_BGR565: \
					TRANSFER_FRAME_HEAD_16 \
                                          transfer_YUVA8888_to_BGR565((output), (input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB565: \
					TRANSFER_FRAME_HEAD_16 \
					transfer_YUVA8888_to_RGB565((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA8888_to_BGR888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA8888_to_BGR8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA8888_to_RGB888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA8888_to_RGBA8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA8888_to_YUVA8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV420P: \
					TRANSFER_YUV420P_OUT_HEAD \
					transfer_YUVA8888_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P: \
					TRANSFER_YUV422P_OUT_HEAD \
					transfer_YUVA8888_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV444P: \
					TRANSFER_YUV444P_OUT_HEAD \
					transfer_YUVA8888_to_YUV444P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422: \
					TRANSFER_FRAME_HEAD \
					transfer_YUVA8888_to_YUV422((output), \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
			} \
			break; \
 \
		case BC_RGB888: \
			switch(out_colormodel) \
			{ \
				case BC_BGR565: \
					TRANSFER_FRAME_HEAD_16 \
					transfer_RGB888_to_BGR565((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB565: \
					TRANSFER_FRAME_HEAD_16 \
					transfer_RGB888_to_RGB565((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_BGR888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_RGB888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_RGBA8888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB161616: \
					TRANSFER_FRAME_HEAD_16 \
					transfer_RGB888_to_RGB161616((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA16161616: \
					TRANSFER_FRAME_HEAD_16 \
					transfer_RGB888_to_RGBA16161616((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_BGR8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_YUVA8888((output), (input));   \
                                        TRANSFER_FRAME_TAIL \
                                        break; \
				case BC_YUV420P: \
					TRANSFER_YUV420P_OUT_HEAD \
					transfer_RGB888_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVJ420P: \
					TRANSFER_YUV420P_OUT_HEAD \
					transfer_RGB888_to_YUVJ420P_YUVJ422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_YUV422((output), (input), j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P: \
					TRANSFER_YUV422P_OUT_HEAD \
					transfer_RGB888_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVJ422P: \
					TRANSFER_YUV422P_OUT_HEAD \
					transfer_RGB888_to_YUVJ420P_YUVJ422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P16: \
					TRANSFER_YUV422P16_OUT_HEAD \
                                          transfer_RGB888_to_YUV422P16((uint16_t *)output_y, \
						(uint16_t *)output_u, \
						(uint16_t *)output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
                case BC_YUV422P10: \
                    TRANSFER_YUV422P16_OUT_HEAD \
                    transfer_RGB888_to_YUV422P10((uint16_t *)output_y, \
                        (uint16_t *)output_u, \
                        (uint16_t *)output_v, \
                        (input), \
                        j); \
                    TRANSFER_FRAME_TAIL \
                    break; \
                case BC_YUVJ422P10: \
                    TRANSFER_YUV422P16_OUT_HEAD \
                    transfer_RGB888_to_YUVJ422P10((uint16_t *)output_y, \
                        (uint16_t *)output_u, \
                        (uint16_t *)output_v, \
                        (input), \
                        j); \
                    TRANSFER_FRAME_TAIL \
                    break; \
				case BC_YUV411P: \
					TRANSFER_YUV411P_OUT_HEAD \
					transfer_RGB888_to_YUV411P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV444P: \
					TRANSFER_YUV444P_OUT_HEAD \
					transfer_RGB888_to_YUV444P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVJ444P: \
					TRANSFER_YUV444P_OUT_HEAD \
					transfer_RGB888_to_YUVJ444P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
                                case BC_YUV444P16:                        \
					TRANSFER_YUV444P_OUT_HEAD \
                                          transfer_RGB888_to_YUV444P16((uint16_t*)output_y, \
                                                                       (uint16_t*)output_u, \
                                                                       (uint16_t*)output_v, \
                                                                       (input), \
                                                                       j); \
					TRANSFER_FRAME_TAIL \
					break; \
			} \
			break; \
 \
		case BC_RGBA8888: \
			switch(out_colormodel) \
			{ \
				case BC_BGR565: \
					TRANSFER_FRAME_HEAD_16 \
					transfer_RGBA8888_to_BGR565((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB565: \
					TRANSFER_FRAME_HEAD_16 \
					transfer_RGBA8888_to_RGB565((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA8888_to_BGR888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA8888_to_RGB888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA8888_to_RGBA8888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB161616: \
					TRANSFER_FRAME_HEAD_16 \
					transfer_RGBA8888_to_RGB161616((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA16161616: \
					TRANSFER_FRAME_HEAD_16 \
					transfer_RGBA8888_to_RGBA16161616((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA8888_to_BGR8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA8888_to_YUVA8888((output), (input));   \
                                        TRANSFER_FRAME_TAIL \
                                        break; \
				case BC_YUV420P: \
					TRANSFER_YUV420P_OUT_HEAD \
					transfer_RGBA8888_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVJ420P: \
					TRANSFER_YUV420P_OUT_HEAD \
					transfer_RGBA8888_to_YUVJ420P_YUVJ422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA8888_to_YUV422((output), (input), j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P: \
					TRANSFER_YUV422P_OUT_HEAD \
					transfer_RGBA8888_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVJ422P: \
					TRANSFER_YUV422P_OUT_HEAD \
					transfer_RGBA8888_to_YUVJ420P_YUVJ422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P16: \
					TRANSFER_YUV422P16_OUT_HEAD \
                                          transfer_RGBA8888_to_YUV422P16((uint16_t *)output_y, \
						(uint16_t *)output_u, \
						(uint16_t *)output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV411P: \
					TRANSFER_YUV411P_OUT_HEAD \
					transfer_RGBA8888_to_YUV411P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV444P: \
					TRANSFER_YUV444P_OUT_HEAD \
					transfer_RGBA8888_to_YUV444P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVJ444P: \
					TRANSFER_YUV444P_OUT_HEAD \
					transfer_RGBA8888_to_YUVJ444P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
                                case BC_YUV444P16:                        \
					TRANSFER_YUV444P_OUT_HEAD \
                                          transfer_RGBA8888_to_YUV444P16((uint16_t*)output_y, \
                                                                       (uint16_t*)output_u, \
                                                                       (uint16_t*)output_v, \
                                                                       (input), \
                                                                       j); \
					TRANSFER_FRAME_TAIL \
					break; \
			} \
			break; \
 \
		case BC_RGB161616: \
			switch(out_colormodel) \
			{ \
				case BC_BGR565: \
					TRANSFER_FRAME_HEAD_16 \
					transfer_RGB161616_to_BGR565((output), (uint16_t*)(input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB565: \
					TRANSFER_FRAME_HEAD_16 \
					transfer_RGB161616_to_RGB565((output), (uint16_t*)(input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB161616_to_BGR888((output), (uint16_t*)(input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB161616_to_BGR8888((output), (uint16_t*)(input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB161616_to_RGB888((output), (uint16_t*)(input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB161616_to_RGBA8888((output), (uint16_t*)(input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB161616_to_YUVA8888((output), (uint16_t*)(input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV420P: \
					TRANSFER_YUV420P_OUT_HEAD \
					transfer_RGB161616_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(uint16_t*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB161616_to_YUV422((output), (uint16_t*)(input), j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P: \
					TRANSFER_YUV422P_OUT_HEAD \
					transfer_RGB161616_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(uint16_t*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P16: \
					TRANSFER_YUV422P16_OUT_HEAD_16 \
					transfer_RGB161616_to_YUV422P16(output_y, \
						output_u, \
						output_v, \
						(uint16_t*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
                    break; \
                case BC_YUV422P10: \
                    TRANSFER_YUV422P16_OUT_HEAD_16 \
                    transfer_RGB161616_to_YUV422P10(output_y, \
                        output_u, \
                        output_v, \
                        (uint16_t*)(input), \
                        j); \
                    TRANSFER_FRAME_TAIL \
                    break; \
                case BC_YUVJ422P10: \
                    TRANSFER_YUV422P16_OUT_HEAD_16 \
                    transfer_RGB161616_to_YUVJ422P10(output_y, \
                        output_u, \
                        output_v, \
                        (uint16_t*)(input), \
                        j); \
                    TRANSFER_FRAME_TAIL \
                    break; \
				case BC_YUV444P: \
					TRANSFER_YUV444P_OUT_HEAD \
					transfer_RGB161616_to_YUV444P(output_y, \
						output_u, \
						output_v, \
						(uint16_t*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV444P16: \
					TRANSFER_YUV444P16_OUT_HEAD_16 \
					transfer_RGB161616_to_YUV444P16(output_y, \
						output_u, \
						output_v, \
						(uint16_t*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
			} \
			break; \
 \
		case BC_RGBA16161616: \
			switch(out_colormodel) \
			{ \
				case BC_BGR565: \
					TRANSFER_FRAME_HEAD_16 \
					transfer_RGBA16161616_to_BGR565((output), (uint16_t*)(input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB565: \
					TRANSFER_FRAME_HEAD_16 \
					transfer_RGBA16161616_to_RGB565((output), (uint16_t*)(input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888:      \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA16161616_to_BGR888((output), (uint16_t*)(input)); \
					TRANSFER_FRAME_TAIL \
				break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA16161616_to_BGR8888((output), (uint16_t*)(input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA16161616_to_RGB888((output), (uint16_t*)(input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA16161616_to_RGBA8888((output), (uint16_t*)(input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV420P: \
					TRANSFER_YUV420P_OUT_HEAD \
					transfer_RGBA16161616_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(uint16_t*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P: \
					TRANSFER_YUV422P_OUT_HEAD \
					transfer_RGBA16161616_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(uint16_t*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV444P: \
					TRANSFER_YUV444P_OUT_HEAD \
					transfer_RGBA16161616_to_YUV444P(output_y, \
						output_u, \
						output_v, \
						(uint16_t*)(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
			} \
			break; \
 \
		case BC_BGR8888: \
			switch(out_colormodel) \
			{ \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_BGR8888_to_RGB888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_BGR8888_to_RGBA8888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_BGR8888_to_BGR8888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
      				case BC_YUV420P: \
					TRANSFER_YUV420P_OUT_HEAD \
					transfer_BGR888_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
			} \
			break; \
 \
		case BC_BGR888: \
			switch(out_colormodel) \
			{ \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_BGR888_to_RGB888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR565: \
					TRANSFER_FRAME_HEAD_16 \
					transfer_BGR888_to_BGR565((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB565: \
					TRANSFER_FRAME_HEAD_16 \
					transfer_BGR888_to_RGB565((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888: \
					TRANSFER_FRAME_HEAD \
					transfer_BGR888_to_BGR888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_BGR888_to_RGBA8888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB161616: \
					TRANSFER_FRAME_HEAD_16 \
					transfer_BGR888_to_RGB161616((uint16_t**)(output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGBA16161616: \
					TRANSFER_FRAME_HEAD_16 \
					transfer_BGR888_to_RGBA16161616((uint16_t**)(output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_BGR888_to_BGR8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUVA8888: \
					TRANSFER_FRAME_HEAD \
					transfer_BGR888_to_YUVA8888((output), (input));   \
                                        TRANSFER_FRAME_TAIL \
                                        break; \
				case BC_YUV420P: \
					TRANSFER_YUV420P_OUT_HEAD \
					transfer_BGR888_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422: \
					TRANSFER_FRAME_HEAD \
					transfer_BGR888_to_YUV422((output), (input), j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P: \
					TRANSFER_YUV422P_OUT_HEAD \
					transfer_BGR888_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV444P: \
					TRANSFER_YUV444P_OUT_HEAD \
					transfer_BGR888_to_YUV444P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
			}      \
			break; \
	} \
}




void cmodel_default(PERMUTATION_ARGS)
{
	if(scale)
	{
		TRANSFER_FRAME_DEFAULT(&output_row, 
			input_row + column_table[j] * in_pixelsize,
			0,
			0,
			0,
			0);
	}
	else
	{
		TRANSFER_FRAME_DEFAULT(&output_row, 
			input_row + j * in_pixelsize,
			0,
			0,
			0,
			0);
	}
}

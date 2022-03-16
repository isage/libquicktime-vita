/*******************************************************************************
 lqt_divx.c

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

// Determine of the compressed frame is a keyframe for direct copy
int quicktime_divx_is_key(unsigned char *data, long size)
{
	int result = 0;
	int i;

	for(i = 0; i < size - 5; i++)
	{
		if( data[i]     == 0x00 && 
			data[i + 1] == 0x00 &&
			data[i + 2] == 0x01 &&
			data[i + 3] == 0xb6)
		{
			if((data[i + 4] & 0xc0) == 0x0) 
				return 1;
			else
				return 0;
		}
	}
	
	return result;
}


// Test for VOL header in frame
int quicktime_divx_has_vol(unsigned char *data)
{
	if( data[0] == 0x00 &&
		data[1] == 0x00 &&
		data[2] == 0x01 &&
		data[3] == 0x00 &&
		data[4] == 0x00 &&
		data[5] == 0x00 &&
		data[6] == 0x01 &&
		data[7] == 0x20)
		return 1;
	else
		return 0;
}



static void putbits(unsigned char **data, 
	int *bit_pos, 
	uint64_t *bit_store, 
	int *total, 
	int count, 
	uint64_t value)
{
	value &= 0xffffffffffffffffLL >> (64 - count);

	while(64 - *bit_pos < count)
	{
		*(*data)++ = (*bit_store) >> 56;
		(*bit_store) <<= 8;
		(*bit_pos) -= 8;
	}

	(*bit_store) |= value << (64 - count - *bit_pos);
	(*bit_pos) += count;
	(*total) += count;
}


static void flushbits(unsigned char **data, 
	int *bit_pos, 
	uint64_t *bit_store)
{
	while((*bit_pos) > 0)
	{
		*(*data)++ = (*bit_store) >> 56;
		(*bit_store) <<= 8;
		(*bit_pos) -= 8;
	}
}




#define VO_START_CODE 		0x8      
#define VO_START_CODE_LENGTH	27
#define VOL_START_CODE 0x12             /* 25-MAR-97 JDL : according to WD2 */
#define VOL_START_CODE_LENGTH 28

int quicktime_divx_write_vol(unsigned char *data_start,
	int vol_width, 
	int vol_height, 
	int time_increment_resolution, 
	double frame_rate)
{
	int written = 0;
	int bits, fixed_vop_time_increment;
	unsigned char *data = data_start;
	int bit_pos;
	uint64_t bit_store;

	bit_store = 0;
	bit_pos = 0;
	vol_width = (int)((float)vol_width / 16 + 0.5) * 16;
	vol_height = (int)((float)vol_height / 16 + 0.5) * 16;


	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		VO_START_CODE_LENGTH, VO_START_CODE);
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		5, 0);				/* vo_id = 0								*/

	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		VOL_START_CODE_LENGTH, VOL_START_CODE);




	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		4, 0);				/* vol_id = 0								*/

	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		1, 0);				/* random_accessible_vol = 0				*/
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		8, 1);				/* video_object_type_indication = 1 video	*/
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		1, 1);				/* is_object_layer_identifier = 1			*/
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		4, 2);				/* visual_object_layer_ver_id = 2			*/
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		3, 1);				/* visual_object_layer_priority = 1			*/
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		4, 1);				/* aspect_ratio_info = 1					*/







	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		1, 0);				/* vol_control_parameter = 0				*/
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		2, 0);				/* vol_shape = 0 rectangular				*/
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		1, 1);				/* marker									*/







	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		16, time_increment_resolution);
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		1, 1);				/* marker									*/
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		1, 1);				/* fixed_vop_rate = 1						*/


	bits = 1;
	while((1 << bits) < time_increment_resolution) bits++;

// Log calculation fails for some reason
//	bits = (int)ceil(log((double)time_increment_resolution) / log(2.0));
//    if (bits < 1) bits=1;

	fixed_vop_time_increment = 
		(int)(time_increment_resolution / frame_rate + 0.1);

	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		bits, fixed_vop_time_increment);

	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		1, 1);				/* marker									*/

	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		13, vol_width);
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		1, 1);				/* marker									*/
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		13, vol_height);
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		1, 1);				/* marker									*/

	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		1, 0);				/* interlaced = 0							*/
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		1, 1);				/* OBMC_disabled = 1						*/
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		2, 0);				/* vol_sprite_usage = 0						*/
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		1, 0);				/* not_8_bit = 0							*/

	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		1, 0);				/* vol_quant_type = 0						*/
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		1, 0);				/* vol_quarter_pixel = 0					*/
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		1, 1);				/* complexity_estimation_disabled = 1		*/
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		1, 1);				/* resync_marker_disabled = 1				*/
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		1, 0);				/* data_partitioning_enabled = 0			*/
	putbits(&data, 
		&bit_pos, 
		&bit_store, 
		&written,
		1, 0);				/* scalability = 0							*/

	flushbits(&data, 
		&bit_pos,
		&bit_store);






	return data - data_start;
}


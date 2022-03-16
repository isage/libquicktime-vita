/*******************************************************************************
 qtatom.c

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
#include <ctype.h>
#include <string.h>

static int read_type(uint8_t *data, uint8_t *type)
{
	type[0] = data[4];
	type[1] = data[5];
	type[2] = data[6];
	type[3] = data[7];

/* need this for quicktime_check_sig */
	if(isalpha(type[0]) && isalpha(type[1]) && isalpha(type[2]) && isalpha(type[3]))
	return 0;
	else
	return 1;
}


static unsigned long read_size(uint8_t *data)
{
	unsigned long result;
	unsigned long a, b, c, d;
	
	a = data[0];
	b = data[1];
	c = data[2];
	d = data[3];

	result = (a << 24) | (b << 16) | (c << 8) | d;

// extended header is size 1
//	if(result < HEADER_LENGTH) result = HEADER_LENGTH;
	return result;
}

static int64_t read_size64(uint8_t *data)
{
	uint64_t result, a, b, c, d, e, f, g, h;

	a = data[0];
	b = data[1];
	c = data[2];
	d = data[3];
	e = data[4];
	f = data[5];
	g = data[6];
	h = data[7];

	result = (a << 56) | 
		(b << 48) | 
		(c << 40) | 
		(d << 32) | 
		(e << 24) | 
		(f << 16) | 
		(g << 8) | 
		h;

	if(result < HEADER_LENGTH) result = HEADER_LENGTH;
	return (int64_t)result;
}

static unsigned long read_ID(uint8_t *data)
{
	unsigned long result;
	unsigned long a, b, c, d;
	
	a = data[0];
	b = data[1];
	c = data[2];
	d = data[3];

	result = (a << 24) | (b << 16) | (c << 8) | d;
	
	return result;
}

static uint16_t read_childcount(uint8_t *data)
{
	uint16_t result, a, b;

	a = data[0];
	b = data[1];

	result = (a << 8) | b;
	
	return (uint16_t)result;
}

static int reset_qtatom(quicktime_qtatom_t *atom)
{
	atom->end = 0;
	atom->type[0] = atom->type[1] = atom->type[2] = atom->type[3] = 0;
	return 0;
}

int quicktime_qtatom_read_header(quicktime_t *file, quicktime_qtatom_t *atom)
{
	int result = 0;
	uint8_t header[10];
	
	reset_qtatom(atom);

	atom->start = quicktime_position(file);

	if(!quicktime_read_data(file, header, HEADER_LENGTH)) return 1;
	result = read_type(header, atom->type);
	atom->size = read_size(header);
	atom->end = atom->start + atom->size;

/* Skip placeholder atom */
	if(quicktime_match_32(atom->type, "wide"))
	{
		atom->start = quicktime_position(file);
		reset_qtatom(atom);
		if(!quicktime_read_data(file, header, HEADER_LENGTH)) return 1;
		result = read_type(header, atom->type);
		atom->size -= 8;
		if(atom->size <= 0)
		{
/* Wrapper ended.  Get new atom size */
			atom->size = read_size(header);
		}
		atom->end = atom->start + atom->size;
	}
	else
/* Get extended size */
	if(atom->size == 1)
	{
		if(!quicktime_read_data(file, header, HEADER_LENGTH)) return 1;
		atom->size = read_size64(header);
		atom->end = atom->start + atom->size;
	}

	quicktime_read_data(file, header, 4);
	atom->ID = read_ID(header);
	quicktime_set_position(file, quicktime_position(file) + 2);
	quicktime_read_data(file, header, 2);
	atom->child_count = read_childcount(header);
	quicktime_set_position(file, quicktime_position(file) + 4);

	return result;
}

int quicktime_qtatom_write_header(quicktime_t *file, 
	quicktime_qtatom_t *atom, 
	char *text, long ID)
{
	int result = 0;

	atom->start = quicktime_position(file);
	result = !quicktime_write_int32(file, 0);
	if(!result) result = !quicktime_write_char32(file, text);
	if(!result) result = !quicktime_write_int32(file, ID);
	if(!result) result = !quicktime_write_int32(file, 0);
	if(!result) result = !quicktime_write_int32(file, 0);
	
	atom->child_count = 0;
	atom->use_64 = 0;
	
	return result;
}

void quicktime_qtatom_write_footer(quicktime_t *file, quicktime_qtatom_t *atom)
{
	atom->end = quicktime_position(file);
	
	if(atom->use_64)
	{
		quicktime_set_position(file, atom->start + 8);
		quicktime_write_int64(file, atom->end - atom->start);
	}
	else
	{	
				quicktime_set_position(file, atom->start);
		quicktime_write_int32(file, atom->end - atom->start);
		quicktime_set_position(file, atom->start + 14);
		if (atom->end - atom->start <= 20)
		{
			atom->child_count = 0;
		}
		else
		{
			quicktime_set_position(file, atom->start + 14);
			quicktime_write_int16(file, atom->child_count);
		}
	}
	quicktime_set_position(file, atom->end);
}

int quicktime_qtatom_is(quicktime_qtatom_t *atom, char *type)
{
if(atom->type[0] == (uint8_t)type[0] &&
		atom->type[1] == (uint8_t)type[1] &&
		atom->type[2] == (uint8_t)type[2] &&
		atom->type[3] == (uint8_t)type[3])
	return 1;
	else
	return 0;
}

void quicktime_qtatom_read_container_header(quicktime_t *file)
{
        uint8_t data[12];
	quicktime_read_data(file, data, 12);

	return;
}

void quicktime_qtatom_write_container_header(quicktime_t *file)
{
        uint8_t data[12];
	memset(data, 0, sizeof(data));
	quicktime_write_data(file, data, 12);

	return;
}

int quicktime_qtatom_skip(quicktime_t *file, quicktime_qtatom_t *atom)
{
	if(atom->start == atom->end) atom->end++;
	return quicktime_set_position(file, atom->end);
}

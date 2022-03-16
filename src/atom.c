/*******************************************************************************
 atom.c

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

static int read_type(uint8_t *data, uint8_t *type)
  {
  type[0] = data[4];
  type[1] = data[5];
  type[2] = data[6];
  type[3] = data[7];

  /* need this for quicktime_check_sig */
  if(isalpha(type[0]) && isalpha(type[1]) && isalpha(type[2]) && isalpha(type[3]))
    return 0;
  else if(type[0] | type[1] | type[2] | type[3] == 0)
    return 0; /* These kind of atoms do happen. */
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

static int reset(quicktime_atom_t *atom)
{
	atom->end = 0;
	atom->type[0] = atom->type[1] = atom->type[2] = atom->type[3] = 0;
	return 0;
}

int quicktime_atom_read_header(quicktime_t *file, quicktime_atom_t *atom)
{
	int result = 0;
	uint8_t header[10];

	if(file->file_type & (LQT_FILE_AVI|LQT_FILE_AVI_ODML))
	{
		reset(atom);
		atom->start = quicktime_position(file);
		if(!quicktime_read_data(file, header, HEADER_LENGTH)) return 1;
		atom->type[0] = header[0];
		atom->type[1] = header[1];
		atom->type[2] = header[2];
		atom->type[3] = header[3];
		atom->size = 
			(((unsigned char)header[4])      ) |
			(((unsigned char)header[5]) << 8 ) |
			(((unsigned char)header[6]) << 16) |
			(((unsigned char)header[7]) << 24);
		atom->end = quicktime_add3(atom->start, atom->size, 8);
	}
	else
	{

		reset(atom);

		atom->start = quicktime_position(file);

		if(!quicktime_read_data(file, header, HEADER_LENGTH)) return 1;
		result = read_type(header, atom->type);
		atom->size = read_size(header);
		atom->end = atom->start + atom->size;
/* Get extended size */
		if(atom->size == 1)
		{
			if(!quicktime_read_data(file, header, HEADER_LENGTH)) return 1;
			atom->size = read_size64(header);
			atom->end = atom->start + atom->size;
		}
	}


	return result;
}

int quicktime_atom_write_header64(quicktime_t *file, quicktime_atom_t *atom, char *text)
{
	int result = 0;
	atom->start = quicktime_position(file);

	result = !quicktime_write_int32(file, 1);
	if(!result) result = !quicktime_write_char32(file, text);
	if(!result) result = !quicktime_write_int64(file, 0);

	atom->use_64 = 1;
	return result;
}

int quicktime_atom_write_header(quicktime_t *file, 
	quicktime_atom_t *atom, 
	char *text)
{
	int result = 0;
	if(file->file_type & (LQT_FILE_AVI|LQT_FILE_AVI_ODML))
	{
		reset(atom);
		atom->start = quicktime_position(file) + 8;
		result = !quicktime_write_char32(file, text);
		if(!result) result = !quicktime_write_int32_le(file, 0);
		atom->use_64 = 0;
	}
	else
	{
		atom->start = quicktime_position(file);
		result = !quicktime_write_int32(file, 0);
		if(!result) result = !quicktime_write_char32(file, text);
		atom->use_64 = 0;
	}
	return result;
}

void quicktime_atom_write_footer(quicktime_t *file, quicktime_atom_t *atom)
{
	atom->end = quicktime_position(file);
	if(file->file_type & (LQT_FILE_AVI|LQT_FILE_AVI_ODML))
	{
		quicktime_set_position(file, atom->start - 4);
		quicktime_write_int32_le(file, atom->end - atom->start);
                quicktime_set_position(file, atom->end);
                if((atom->end - atom->start) % 2)
                  {
                  quicktime_write_char(file, 0x00);
                  }
                atom->size = atom->end - atom->start + 8;
	}
	else
	{
		if(atom->use_64)
		{
			quicktime_set_position(file, atom->start + 8);
			quicktime_write_int64(file, atom->end - atom->start);
		}
		else
		{
			quicktime_set_position(file, atom->start);
			quicktime_write_int32(file, atom->end - atom->start);
		}
	quicktime_set_position(file, atom->end);
	}

}

int quicktime_atom_is(quicktime_atom_t *atom, char *type)
{
if(atom->type[0] == (uint8_t)type[0] &&
		atom->type[1] == (uint8_t)type[1] &&
		atom->type[2] == (uint8_t)type[2] &&
		atom->type[3] == (uint8_t)type[3])
	return 1;
	else
	return 0;
}

int quicktime_atom_skip(quicktime_t *file, quicktime_atom_t *atom)
{
        if((file->file_type & (LQT_FILE_AVI|LQT_FILE_AVI_ODML)) &&
           (atom->end % 2))
          atom->end++;
        else if(atom->start == atom->end) atom->end++;
	return quicktime_set_position(file, atom->end);
}


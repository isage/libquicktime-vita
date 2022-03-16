/*******************************************************************************
 gmhd_text.c

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

#define LOG_DOMAIN "gmhd_text"

/* No idea, what these values mean, they are taken from
   the hexdump of a Quicktime file.
   
   It's not even clear, if they are organized as 32 bit
   integers or something else :)
*/


void quicktime_gmhd_text_init(quicktime_gmhd_text_t *gmhd_text)
  {
  gmhd_text->unk[0] = 0x00010000;
  gmhd_text->unk[1] = 0x00000000;
  gmhd_text->unk[2] = 0x00000000;
  gmhd_text->unk[3] = 0x00000000;
  gmhd_text->unk[4] = 0x00010000;
  gmhd_text->unk[5] = 0x00000000;
  gmhd_text->unk[6] = 0x00000000;
  gmhd_text->unk[7] = 0x00000000;
  gmhd_text->unk[8] = 0x40000000;
  }
     
void quicktime_gmhd_text_delete(quicktime_gmhd_text_t *gmhd_text)
  {

  }

void quicktime_gmhd_text_dump(quicktime_gmhd_text_t *gmhd_text)
  {
  int i;
  lqt_dump("     gmhd text atom (no idea what this is)\n");

  for(i = 0; i < 9; i++)
    {
    lqt_dump("       Unknown %d: 0x%08x\n", i, gmhd_text->unk[i]);
    }
  }

void quicktime_read_gmhd_text(quicktime_t *file,
                              quicktime_gmhd_text_t *gmhd_text,
                              quicktime_atom_t *parent_atom)
  {
  int i;
  for(i = 0; i < 9; i++)
    gmhd_text->unk[i] = quicktime_read_int32(file);
  if(quicktime_position(file) < parent_atom->end)
    {
    lqt_log(file, LQT_LOG_WARNING, LOG_DOMAIN, "More than 36 bytes in the gmhd text atom\n");
    quicktime_atom_skip(file, parent_atom);
    }
  }

void quicktime_write_gmhd_text(quicktime_t *file,
                               quicktime_gmhd_text_t *gmhd_text)
  {
  int i;
  quicktime_atom_t atom;
  quicktime_atom_write_header(file, &atom, "text");
  for(i = 0; i < 9; i++)
    {
    quicktime_write_int32(file, gmhd_text->unk[i]);
    }
  quicktime_atom_write_footer(file, &atom);
  }

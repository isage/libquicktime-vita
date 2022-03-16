/*******************************************************************************
 enda.c

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

void quicktime_read_enda(quicktime_t *file, quicktime_enda_t *enda,
                         quicktime_atom_t *enda_atom)
  {
  enda->littleEndian = quicktime_read_int16(file);
  }

void quicktime_write_enda(quicktime_t *file, quicktime_enda_t *enda)
  {
  quicktime_atom_t atom;
  quicktime_atom_write_header(file, &atom, "enda");
  quicktime_write_int16(file, !!enda->littleEndian);
  quicktime_atom_write_footer(file, &atom);
  }

void quicktime_set_enda(quicktime_stsd_table_t *table, int little_endian)
  {
  quicktime_wave_t * wave = &table->wave;
  wave->enda.littleEndian = little_endian;
  wave->has_enda = 1;
  table->has_wave = 1;
  }

void quicktime_enda_dump(quicktime_enda_t *enda)
  {
  lqt_dump("         enda: \n");
  lqt_dump("           littleEndian: %d\n", enda->littleEndian);
  }

/* Returns TRUE if little endian */
int quicktime_get_enda(quicktime_stsd_table_t *table)
  {
  quicktime_wave_t * wave = &table->wave;
  if(wave->has_enda)
    {
    return wave->enda.littleEndian;
    }
  else
    return 0;
  }

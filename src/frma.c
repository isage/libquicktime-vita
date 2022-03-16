/*******************************************************************************
 frma.c

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
#include <string.h>

void quicktime_read_frma(quicktime_t *file, quicktime_frma_t *frma,
                         quicktime_atom_t *frma_atom)
  {
  quicktime_read_data(file, (uint8_t*)frma->codec, 4);
  }

void quicktime_write_frma(quicktime_t *file, quicktime_frma_t *frma)
  {
  quicktime_atom_t atom;
  quicktime_atom_write_header(file, &atom, "frma");
  quicktime_write_data(file, (uint8_t*)frma->codec, 4);
  quicktime_atom_write_footer(file, &atom);
  }

void quicktime_frma_dump(quicktime_frma_t *frma)
  {
  lqt_dump("         frma: \n");
  lqt_dump("           codec: %c%c%c%c\n",
         frma->codec[0], frma->codec[1], frma->codec[2], frma->codec[3]);
  }

void quicktime_set_frma(quicktime_trak_t * trak, char * codec)
  {
  quicktime_wave_t * wave = &trak->mdia.minf.stbl.stsd.table[0].wave;
  memcpy(wave->frma.codec, codec, 4);
  wave->has_frma = 1;
  trak->mdia.minf.stbl.stsd.table[0].has_wave = 1;
  }

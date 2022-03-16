/*******************************************************************************
 avi_movi.c

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

void quicktime_delete_movi(quicktime_t *file, quicktime_movi_t *movi)
{
}

void quicktime_init_movi(quicktime_t *file, quicktime_riff_t *riff)
{
	quicktime_movi_t *movi = &riff->movi;

	quicktime_atom_write_header(file, &movi->atom, "LIST");
	quicktime_write_char32(file, "movi");

}

void quicktime_read_movi(quicktime_t *file, 
	quicktime_atom_t *parent_atom,
	quicktime_movi_t *movi)
{
	movi->atom.size = parent_atom->size;
// Relative to start of the movi string
	movi->atom.start = parent_atom->start + 8;
	quicktime_atom_skip(file, parent_atom);
}

void quicktime_finalize_movi(quicktime_t *file, quicktime_movi_t *movi)
  {
  quicktime_atom_write_footer(file, &movi->atom);
  }







/*******************************************************************************
 nmhd.c

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

int quicktime_nmhd_init(quicktime_nmhd_t *nmhd)
  {
  return 0;
  }

int quicktime_nmhd_delete(quicktime_nmhd_t *nmhd)
  {
  return 0;
  }

void quicktime_nmhd_dump(quicktime_nmhd_t *nmhd)
  {
  lqt_dump("   null media header (nmhd)\n");
  lqt_dump("    version %d\n", nmhd->version);
  lqt_dump("    flags %ld\n", nmhd->flags);
  }

int quicktime_read_nmhd(quicktime_t *file, quicktime_nmhd_t *nmhd)
  {
  nmhd->version = quicktime_read_char(file);
  nmhd->flags = quicktime_read_int24(file);
  return 0;
  }

void quicktime_write_nmhd(quicktime_t *file, quicktime_nmhd_t *nmhd)
  {
  quicktime_atom_t atom;
  quicktime_atom_write_header(file, &atom, "nmhd");
  quicktime_write_char(file, nmhd->version);
  quicktime_write_int24(file, nmhd->flags);
  quicktime_atom_write_footer(file, &atom);
  }

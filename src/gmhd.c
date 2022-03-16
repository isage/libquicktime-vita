/*******************************************************************************
 gmhd.c

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

void quicktime_gmhd_init(quicktime_gmhd_t *gmhd)
  {
  quicktime_gmin_init(&gmhd->gmin);
  }

void quicktime_gmhd_init_timecode(quicktime_gmhd_t *gmhd)
  {
  quicktime_gmin_init(&gmhd->gmin);
  quicktime_tmcd_init(&gmhd->tmcd);
  gmhd->has_tmcd = 1;
  }

void quicktime_gmhd_delete(quicktime_gmhd_t *gmhd)
  {
  quicktime_gmin_delete(&gmhd->gmin);
  quicktime_tmcd_delete(&gmhd->tmcd);
  }

void quicktime_gmhd_dump(quicktime_gmhd_t *gmhd)
  {
  lqt_dump("     base media header (gmhd)\n");
  quicktime_gmin_dump(&gmhd->gmin);
  if(gmhd->has_gmhd_text)
    quicktime_gmhd_text_dump(&gmhd->gmhd_text);
  if (gmhd->has_tmcd)
    quicktime_tmcd_dump(&gmhd->tmcd);
  }

void quicktime_read_gmhd(quicktime_t *file,
                         quicktime_gmhd_t *gmhd,
                         quicktime_atom_t *parent_atom)
  {
  quicktime_atom_t leaf_atom;
  
  do
    {
    quicktime_atom_read_header(file, &leaf_atom);
    
    if(quicktime_atom_is(&leaf_atom, "gmin"))
      quicktime_read_gmin(file, &gmhd->gmin);
    else if(quicktime_atom_is(&leaf_atom, "text"))
      {
      quicktime_read_gmhd_text(file, &gmhd->gmhd_text, &leaf_atom);
      gmhd->has_gmhd_text = 1;
      }
    else if(quicktime_atom_is(&leaf_atom, "tmcd"))
      {
      quicktime_read_tmcd(file, &gmhd->tmcd, &leaf_atom);
      gmhd->has_tmcd = 1; 
      }
    else
      quicktime_atom_skip(file, &leaf_atom);
    }while(quicktime_position(file) < parent_atom->end);
  
  }

void quicktime_write_gmhd(quicktime_t *file, quicktime_gmhd_t *gmhd)
  {
  quicktime_atom_t atom;
  quicktime_atom_write_header(file, &atom, "gmhd");
  quicktime_write_gmin(file, &gmhd->gmin);
  
  if(gmhd->has_gmhd_text)
    quicktime_write_gmhd_text(file, &gmhd->gmhd_text);
  if (gmhd->has_tmcd)
    quicktime_write_tmcd(file, &gmhd->tmcd);
  
  quicktime_atom_write_footer(file, &atom);
  }


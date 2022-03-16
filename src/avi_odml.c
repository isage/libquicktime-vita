/*******************************************************************************
 avi_odml.c

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
#if 0
void quicktime_read_odml(quicktime_t *file, quicktime_atom_t *parent_atom)
{
}
#endif

void quicktime_init_odml(quicktime_t *file, quicktime_hdrl_t *hdrl)
{
	quicktime_atom_t list_atom, dmlh_atom;


// LIST 'odml'
	quicktime_atom_write_header(file, &list_atom, "LIST");
	quicktime_write_char32(file, "odml");
// 'dmlh'
	quicktime_atom_write_header(file, &dmlh_atom, "dmlh");

// Placeholder for total frames in all RIFF objects
	hdrl->total_frames_offset = quicktime_position(file);
	quicktime_write_int32_le(file, 0);

	quicktime_atom_write_footer(file, &dmlh_atom);
	quicktime_atom_write_footer(file, &list_atom);
}

void quicktime_finalize_odml(quicktime_t *file, quicktime_hdrl_t *hdrl)
  {
  // Get length in frames
  if(file->total_vtracks)
    {
    quicktime_set_position(file, hdrl->total_frames_offset);
    quicktime_write_int32_le(file, quicktime_track_samples(file, file->vtracks[0].track));
    }
  }
#if 0
void quicktime_dump_odml(quicktime_t *file, quicktime_hdrl_t *hdrl)
  {
  
  }
#endif

/*******************************************************************************
 videocodec.c

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
#include "videocodec.h"

/* Common functions used by most uncompressed video codecs */

void lqt_set_fiel_uncompressed(quicktime_t * file, int track)
  {
  quicktime_video_map_t *vtrack = &file->vtracks[track];
  quicktime_stsd_table_t *stsd;
  
  stsd = vtrack->track->mdia.minf.stbl.stsd.table;

  if(stsd->has_fiel)
    return;
  
  switch(vtrack->interlace_mode)
    {
    case LQT_INTERLACE_NONE:
      lqt_set_fiel(file, track, 1, 0);
      break;
    case LQT_INTERLACE_TOP_FIRST:
      lqt_set_fiel(file, track, 2, 9);
      break;
    case LQT_INTERLACE_BOTTOM_FIRST:
      lqt_set_fiel(file, track, 2, 14);
      break;
    }
  }

int lqt_set_colr_yuv_uncompressed(quicktime_t *file,int track) {
    quicktime_colr_t colr;
    
    if (file->vtracks[track].track->mdia.minf.stbl.stsd.table->has_colr) {
	/* The user has already provided a colr atom. */
	return 0;
    }
    
    colr.colorParamType=LQT_COLR_NCLC;
    colr.primaries=1;
    colr.transferFunction=1;
    colr.matrix=1;
    return lqt_set_colr(file,track,&colr);
}

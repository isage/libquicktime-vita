/*******************************************************************************
 avi_avih.c

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

/*
  uint32_t dwMicroSecPerFrame;
  uint32_t dwMaxBytesPerSec;
  uint32_t dwReserved1;
  uint32_t dwFlags;
  uint32_t dwTotalFrames;
  uint32_t dwInitialFrames;
  uint32_t dwStreams;
  uint32_t dwSuggestedBufferSize;
  uint32_t dwWidth;
  uint32_t dwHeight;
  uint32_t dwScale;
  uint32_t dwRate;
  uint32_t dwStart;
  uint32_t dwLength;
*/

void quicktime_read_avih(quicktime_t *file,
                         quicktime_avih_t *avih,
                         quicktime_atom_t *parent_atom)
  {
  avih->dwMicroSecPerFrame    = quicktime_read_int32_le(file);
  avih->dwMaxBytesPerSec      = quicktime_read_int32_le(file);
  avih->dwReserved1           = quicktime_read_int32_le(file);
  avih->dwFlags               = quicktime_read_int32_le(file);
  avih->dwTotalFrames         = quicktime_read_int32_le(file);
  avih->dwInitialFrames       = quicktime_read_int32_le(file);
  avih->dwStreams             = quicktime_read_int32_le(file);
  avih->dwSuggestedBufferSize = quicktime_read_int32_le(file);
  avih->dwWidth               = quicktime_read_int32_le(file);
  avih->dwHeight              = quicktime_read_int32_le(file);
  avih->dwScale               = quicktime_read_int32_le(file);
  avih->dwRate                = quicktime_read_int32_le(file);
  avih->dwStart               = quicktime_read_int32_le(file);
  avih->dwLength              = quicktime_read_int32_le(file);
  }

void quicktime_write_avih(quicktime_t *file,
                          quicktime_avih_t *avih)
  {
  quicktime_atom_t avih_atom;
  quicktime_atom_write_header(file, &avih_atom, "avih");

  quicktime_write_int32_le(file, avih->dwMicroSecPerFrame);
  quicktime_write_int32_le(file, avih->dwMaxBytesPerSec);
  quicktime_write_int32_le(file, avih->dwReserved1);
  quicktime_write_int32_le(file, avih->dwFlags);
  quicktime_write_int32_le(file, avih->dwTotalFrames);
  quicktime_write_int32_le(file, avih->dwInitialFrames);
  quicktime_write_int32_le(file, avih->dwStreams);
  quicktime_write_int32_le(file, avih->dwSuggestedBufferSize);
  quicktime_write_int32_le(file, avih->dwWidth);
  quicktime_write_int32_le(file, avih->dwHeight);
  quicktime_write_int32_le(file, avih->dwScale);
  quicktime_write_int32_le(file, avih->dwRate);
  quicktime_write_int32_le(file, avih->dwStart);
  quicktime_write_int32_le(file, avih->dwLength);
  quicktime_atom_write_footer(file, &avih_atom);
  }

void quicktime_avih_dump(quicktime_avih_t *avih)
  {
  lqt_dump("avih\n");
  lqt_dump("  dwMicroSecPerFrame: %d\n",    avih->dwMicroSecPerFrame);
  lqt_dump("  dwMaxBytesPerSec: %d\n",      avih->dwMaxBytesPerSec);
  lqt_dump("  dwReserved1: %d\n",           avih->dwReserved1);
  lqt_dump("  dwFlags: %d\n",               avih->dwFlags);
  lqt_dump("  dwTotalFrames: %d\n",         avih->dwTotalFrames);
  lqt_dump("  dwInitialFrames: %d\n",       avih->dwInitialFrames);
  lqt_dump("  dwStreams: %d\n",             avih->dwStreams);
  lqt_dump("  dwSuggestedBufferSize: %d\n", avih->dwSuggestedBufferSize);
  lqt_dump("  dwWidth: %d\n",               avih->dwWidth);
  lqt_dump("  dwHeight: %d\n",              avih->dwHeight);
  lqt_dump("  dwScale: %d\n",               avih->dwScale);
  lqt_dump("  dwRate: %d\n",                avih->dwRate);
  lqt_dump("  dwStart: %d\n",               avih->dwStart);
  lqt_dump("  dwLength: %d\n",              avih->dwLength);
  }

void quicktime_avih_init(quicktime_avih_t *avih, quicktime_t * file)
  {
  if(file->total_vtracks)
    avih->dwMicroSecPerFrame = (uint32_t)(1000000 / quicktime_frame_rate(file, 0));
  avih->dwFlags = AVI_HASINDEX | AVI_ISINTERLEAVED;
  avih->dwStreams = file->total_atracks + file->total_vtracks;
  //  avih->dwSuggestedBufferSize = 1024 * 1024;

  if(file->total_vtracks)
    {
    avih->dwWidth = file->vtracks[0].track->tkhd.track_width;
    avih->dwHeight = file->vtracks[0].track->tkhd.track_height;
    }
  }

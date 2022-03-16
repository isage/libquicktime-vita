/*******************************************************************************
 avi_strh.c

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
 *
  char fccType[4];
  char fccHandler[4];
  uint32_t dwFlags;
  uint32_t dwReserved1;
  uint32_t dwInitialFrames;
  uint32_t dwScale;
  uint32_t dwRate;
  uint32_t dwStart;
  uint32_t dwLength;
  uint32_t dwSuggestedBufferSize;
  uint32_t dwQuality;
  uint32_t dwSampleSize;
 
 *
 */

void quicktime_read_strh(quicktime_t *file,
                         quicktime_strh_t *strh,
                         quicktime_atom_t *parent_atom)
  {
  quicktime_read_data(file, (uint8_t*)(strh->fccType), 4);
  quicktime_read_data(file, (uint8_t*)(strh->fccHandler), 4);

  strh->dwFlags               = quicktime_read_int32_le(file);
  strh->dwReserved1           = quicktime_read_int32_le(file);
  strh->dwInitialFrames       = quicktime_read_int32_le(file);
  strh->dwScale               = quicktime_read_int32_le(file);
  strh->dwRate                = quicktime_read_int32_le(file);
  strh->dwStart               = quicktime_read_int32_le(file);
  strh->dwLength              = quicktime_read_int32_le(file);
  strh->dwSuggestedBufferSize = quicktime_read_int32_le(file);
  strh->dwQuality             = quicktime_read_int32_le(file);
  strh->dwSampleSize          = quicktime_read_int32_le(file);
  strh->rcFrame.left          = quicktime_read_int16_le(file);
  strh->rcFrame.top           = quicktime_read_int16_le(file);
  strh->rcFrame.right         = quicktime_read_int16_le(file);
  strh->rcFrame.bottom        = quicktime_read_int16_le(file);
  }

void quicktime_write_strh(quicktime_t *file,
                          quicktime_strh_t *strh)
  {
  quicktime_atom_t atom;
  quicktime_atom_write_header(file, &atom, "strh");
  quicktime_write_data(file, (uint8_t*)(strh->fccType), 4);
  quicktime_write_data(file, (uint8_t*)(strh->fccHandler), 4);
  
  quicktime_write_int32_le(file, strh->dwFlags);
  quicktime_write_int32_le(file, strh->dwReserved1);
  quicktime_write_int32_le(file, strh->dwInitialFrames);
  quicktime_write_int32_le(file, strh->dwScale);
  quicktime_write_int32_le(file, strh->dwRate);
  quicktime_write_int32_le(file, strh->dwStart);
  quicktime_write_int32_le(file, strh->dwLength);
  quicktime_write_int32_le(file, strh->dwSuggestedBufferSize);
  quicktime_write_int32_le(file, strh->dwQuality);
  quicktime_write_int32_le(file, strh->dwSampleSize);
  quicktime_write_int16_le(file, strh->rcFrame.left);
  quicktime_write_int16_le(file, strh->rcFrame.top);
  quicktime_write_int16_le(file, strh->rcFrame.right);
  quicktime_write_int16_le(file, strh->rcFrame.bottom);
  
  quicktime_atom_write_footer(file, &atom);
  
  }

void quicktime_strh_dump(quicktime_strh_t *strh)
  {
  lqt_dump("  strh\n");

  lqt_dump("    fccType:               %.4s\n",  strh->fccType);
  lqt_dump("    fccHandler:            %.4s\n",  strh->fccHandler);
  lqt_dump("    dwFlags:               %08x\n", strh->dwFlags);
  lqt_dump("    dwReserved1:           %08x\n", strh->dwReserved1);
  lqt_dump("    dwInitialFrames:       %d\n", strh->dwInitialFrames);
  lqt_dump("    dwScale:               %d\n", strh->dwScale);
  lqt_dump("    dwRate:                %d\n", strh->dwRate);
  lqt_dump("    dwStart:               %d\n", strh->dwStart);
  lqt_dump("    dwLength:              %d\n", strh->dwLength);
  lqt_dump("    dwSuggestedBufferSize: %d\n", strh->dwSuggestedBufferSize);
  lqt_dump("    dwQuality:             %d\n", strh->dwQuality);
  lqt_dump("    dwSampleSize:          %d\n", strh->dwSampleSize);
  lqt_dump("    rcFrame:               l: %d t: %d r: %d b: %d\n",
         strh->rcFrame.left, strh->rcFrame.top, strh->rcFrame.right,
         strh->rcFrame.bottom);
  }

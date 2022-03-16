/*******************************************************************************
 pano.c

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

int quicktime_pano_init(quicktime_pano_t *pano)
{
       pano->version = 0;
       pano->revision = 0;

       pano->STrack = 0;
       pano->LowResSTrack = 0;
       pano->HSTrack = 0;

       pano->HPanStart = 0;
       pano->HPanEnd = 360;
       pano->VPanStart = 30;
       pano->VPanEnd = -30;
       pano->MinZoom = 0;
       pano->MaxZoom = 0;

       pano->SHeight = 0;
       pano->SWidth = 0;
       pano->NumFrames = 1;
       pano->SNumFramesHeight = 1;
       pano->SNumFramesWidth = 1;
       pano->SDepth = 32;

       pano->HSHeight = 0;
       pano->HSWidth = 0;
       pano->HSNumFramesHeight = 0;
       pano->HSNumFramesWidth = 0;
       pano->HSDepth = 8;

       return 0;
}

int quicktime_pano_delete(quicktime_pano_t *pano)
{
    return 0;
}

void quicktime_pano_dump(quicktime_pano_t *pano)
{
       lqt_dump("       panorama (pano)\n");
       lqt_dump("        version %i\n", pano->version );
       lqt_dump("        revision %i\n", pano->revision );

       lqt_dump("        scene track %ld\n", pano->STrack );
       lqt_dump("        lowres scene track %ld\n", pano->LowResSTrack );
       lqt_dump("        hotspot track %ld\n", pano->HSTrack );

       lqt_dump("        horizontal start pan %f\n", pano->HPanStart );
       lqt_dump("        horizontal end pan %f\n", pano->HPanEnd );
       lqt_dump("        vertical start pan %f\n", pano->VPanStart );
       lqt_dump("        vertical end pan %f\n", pano->VPanEnd );
       lqt_dump("        minimum zoom %f\n", pano->MinZoom );
       lqt_dump("        maximum zoom %f\n", pano->MaxZoom );

       lqt_dump("        scene height %ld\n", pano->SHeight );
       lqt_dump("        scene width %ld\n", pano->SWidth );
       lqt_dump("        num frames %ld\n", pano->NumFrames );
       lqt_dump("        num frames(height) %i\n", pano->SNumFramesHeight );
       lqt_dump("        num frames(width) %i\n", pano->SNumFramesWidth );
       lqt_dump("        scene depth %i\n", pano->SDepth );

       lqt_dump("        hotspot height %ld\n", pano->HSHeight );
       lqt_dump("        hotspot width %ld\n", pano->HSWidth );
       lqt_dump("        num. hotspot frames (height) %i\n", pano->HSNumFramesHeight );
       lqt_dump("        num. hotspot frames (width) %i\n", pano->HSNumFramesWidth );
       lqt_dump("        hotspot depth %i\n", pano->HSDepth );
}

int quicktime_read_pano(quicktime_t *file, quicktime_pano_t *pano, quicktime_atom_t *pano_atom)
{
       pano->version = quicktime_read_int16(file);
       pano->revision = quicktime_read_int16(file);

       pano->STrack = quicktime_read_int32(file);
       pano->LowResSTrack = quicktime_read_int32(file);
       quicktime_read_data(file, (uint8_t *)pano->reserved3,  4 * 6);
       pano->HSTrack = quicktime_read_int32(file);
       quicktime_read_data(file, (uint8_t *)pano->reserved4, 4 * 9);

       pano->HPanStart = quicktime_read_fixed32(file);
       pano->HPanEnd = quicktime_read_fixed32(file);
       pano->VPanStart = quicktime_read_fixed32(file);
       pano->VPanEnd = quicktime_read_fixed32(file);
       pano->MinZoom = quicktime_read_fixed32(file);
       pano->MaxZoom = quicktime_read_fixed32(file);

       pano->SHeight = quicktime_read_int32(file);
       pano->SWidth = quicktime_read_int32(file);
       pano->NumFrames = quicktime_read_int32(file);
       pano->reserved5 = quicktime_read_int16(file);
       pano->SNumFramesHeight = quicktime_read_int16(file);
       pano->SNumFramesWidth = quicktime_read_int16(file);
       pano->SDepth = quicktime_read_int16(file);

       pano->HSHeight = quicktime_read_int32(file);
       pano->HSWidth = quicktime_read_int32(file);
       pano->reserved6 = quicktime_read_int16(file);
       pano->HSNumFramesHeight = quicktime_read_int16(file);
       pano->HSNumFramesWidth = quicktime_read_int16(file);
       pano->HSDepth = quicktime_read_int16(file);
       return 0;
}

void quicktime_write_pano(quicktime_t *file, quicktime_pano_t *pano)
{
       quicktime_write_int16(file, pano->version);
       quicktime_write_int16(file, pano->revision);

       quicktime_write_int32(file, pano->STrack);
       quicktime_write_int32(file, pano->LowResSTrack);
       quicktime_write_data(file, (uint8_t *)pano->reserved3, 4 * 6);
       quicktime_write_int32(file, pano->HSTrack);
       quicktime_write_data(file, (uint8_t *)pano->reserved4, 4 * 9);

       quicktime_write_fixed32(file, pano->HPanStart);
       quicktime_write_fixed32(file, pano->HPanEnd);
       quicktime_write_fixed32(file, pano->VPanStart);
       quicktime_write_fixed32(file, pano->VPanEnd);
       quicktime_write_fixed32(file, pano->MinZoom);
       quicktime_write_fixed32(file, pano->MaxZoom);

       quicktime_write_int32(file, pano->SHeight);
       quicktime_write_int32(file, pano->SWidth);
       quicktime_write_int32(file, pano->NumFrames);
       quicktime_write_int16(file, pano->reserved5);
       quicktime_write_int16(file, pano->SNumFramesHeight);
       quicktime_write_int16(file, pano->SNumFramesWidth);
       quicktime_write_int16(file, pano->SDepth);

       quicktime_write_int32(file, pano->HSHeight);
       quicktime_write_int32(file, pano->HSWidth);
       quicktime_write_int16(file, pano->reserved6);
       quicktime_write_int16(file, pano->HSNumFramesHeight);
       quicktime_write_int16(file, pano->HSNumFramesWidth);
       quicktime_write_int16(file, pano->HSDepth);
}


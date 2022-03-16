/*******************************************************************************
 pdat.c

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

int quicktime_pdat_init(quicktime_pdat_t *pdat)
{
       pdat->version = 2;
       pdat->revision = 0;
       pdat->imageRefTrackIndex = 1;
       pdat->hotSpotRefTrackIndex = 0;
       pdat->minPan = 0;
       pdat->maxPan = 360;
       pdat->defaultPan = 0;
       pdat->minTilt = 72;
       pdat->maxTilt = -72;
       pdat->defaultTilt = 0;
       pdat->minFOV = 0;
       pdat->maxFOV = 64;
       pdat->defaultFOV = 64;
       pdat->imageSizeX = 0;
       pdat->imageSizeY = 0;
       pdat->imageNumFramesX = 1;
       pdat->imageNumFramesY = 1;
       pdat->hotSpotSizeX = 0;
       pdat->hotSpotSizeY = 0;
       pdat->hotSpotNumFramesX = 0;
       pdat->hotSpotNumFramesY = 0;
       pdat->flags = 0;
       pdat->panoType[0] = '\0';
       pdat->panoType[1] = '\0';
       pdat->panoType[2] = '\0';
       pdat->panoType[3] = '\0';
       return 0;
}

int quicktime_pdat_delete(quicktime_pdat_t *pdat)
{
    return 0;
}

void quicktime_pdat_dump(quicktime_pdat_t *pdat)
{
       lqt_dump("object node (pdat)\n");
       lqt_dump(" version %i\n", pdat->version );
       lqt_dump(" revision %i\n", pdat->revision );

       lqt_dump(" image track index %ld\n", pdat->imageRefTrackIndex );
       lqt_dump(" hotspot track index %ld\n", pdat->hotSpotRefTrackIndex );       
       lqt_dump(" minimum pan %f\n", pdat->minPan );
       lqt_dump(" maximum pan %f\n", pdat->maxPan );
       lqt_dump(" minimum tilt %f\n", pdat->minTilt );
       lqt_dump(" maximum tilt %f\n", pdat->maxTilt );
       lqt_dump(" minimum fov %f\n", pdat->minFOV );
       lqt_dump(" fov %f\n", pdat->maxFOV );
       lqt_dump(" default pan %f\n", pdat->defaultPan );
       lqt_dump(" default tilt %f\n", pdat->defaultTilt );
       lqt_dump(" default fov %f\n", pdat->defaultFOV );
       lqt_dump(" image size x %ld\n", pdat->imageSizeX );
       lqt_dump(" image size y %ld\n", pdat->imageSizeY );
       lqt_dump(" image frames x %i\n", pdat->imageNumFramesX );
       lqt_dump(" image frames y %i\n", pdat->imageNumFramesY );
       lqt_dump(" hotspot size x %ld\n", pdat->hotSpotSizeX );
       lqt_dump(" hotspot size y %ld\n", pdat->hotSpotSizeY );
       lqt_dump(" hotspot frames x %i\n", pdat->hotSpotNumFramesX );
       lqt_dump(" hotspot frames y %i\n", pdat->hotSpotNumFramesY );
       lqt_dump(" flags %ld\n", pdat->flags );
       lqt_dump(" panorama type %c%c%c%c\n",  pdat->panoType[0], pdat->panoType[1], pdat->panoType[2], pdat->panoType[3]);

}

int quicktime_read_pdat(quicktime_t *file, quicktime_pdat_t *pdat)
{
	pdat->version = quicktime_read_int16(file);
	pdat->revision = quicktime_read_int16(file);
	pdat->imageRefTrackIndex = quicktime_read_int32(file);
	pdat->hotSpotRefTrackIndex = quicktime_read_int32(file);
	pdat->minPan = quicktime_read_float32(file);
	pdat->maxPan = quicktime_read_float32(file);
	pdat->minTilt = quicktime_read_float32(file);
	pdat->maxTilt = quicktime_read_float32(file);
	pdat->minFOV = quicktime_read_float32(file);
	pdat->maxFOV = quicktime_read_float32(file);
	pdat->defaultPan = quicktime_read_float32(file);
	pdat->defaultTilt = quicktime_read_float32(file);
	pdat->defaultFOV = quicktime_read_float32(file);
	pdat->imageSizeX = quicktime_read_int32(file);
	pdat->imageSizeY = quicktime_read_int32(file);
	pdat->imageNumFramesX = quicktime_read_int16(file);
	pdat->imageNumFramesY = quicktime_read_int16(file);
	pdat->hotSpotSizeX = quicktime_read_int32(file);
	pdat->hotSpotSizeY = quicktime_read_int32(file);
	pdat->hotSpotNumFramesX = quicktime_read_int16(file);
	pdat->hotSpotNumFramesY = quicktime_read_int16(file);
	pdat->flags = quicktime_read_int32(file);
	quicktime_read_char32(file, pdat->panoType);
	pdat->reserved = quicktime_read_int32(file);
	return 0;
}

void quicktime_write_pdat(quicktime_t *file, quicktime_pdat_t *pdat)
{
	quicktime_write_int16(file, pdat->version);
	quicktime_write_int16(file, pdat->revision);
	quicktime_write_int32(file, pdat->imageRefTrackIndex);
	quicktime_write_int32(file, pdat->hotSpotRefTrackIndex);
	quicktime_write_float32(file, pdat->minPan);
	quicktime_write_float32(file, pdat->maxPan);
	quicktime_write_float32(file, pdat->minTilt);
	quicktime_write_float32(file, pdat->maxTilt);
	quicktime_write_float32(file, pdat->minFOV);
	quicktime_write_float32(file, pdat->maxFOV);
	quicktime_write_float32(file, pdat->defaultPan);
	quicktime_write_float32(file, pdat->defaultPan);
	quicktime_write_float32(file, pdat->defaultPan);
	quicktime_write_int32(file, pdat->imageSizeX);
	quicktime_write_int32(file, pdat->imageSizeY);
	quicktime_write_int16(file, pdat->imageNumFramesX);
	quicktime_write_int16(file, pdat->imageNumFramesY);
	quicktime_write_int32(file, pdat->hotSpotSizeX);
	quicktime_write_int32(file, pdat->hotSpotSizeY);
	quicktime_write_int16(file, pdat->hotSpotNumFramesX);
	quicktime_write_int16(file, pdat->hotSpotNumFramesY);
	quicktime_write_int32(file, pdat->flags);
	quicktime_write_char32(file, pdat->panoType);
	quicktime_write_int32(file, pdat->reserved);	
	return;
}


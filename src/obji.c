/*******************************************************************************
 obji.c

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

int quicktime_obji_init(quicktime_obji_t *obji)
{
       obji->version = 2;
       obji->revision = 0;
       obji->movieType = 1;
       obji->viewStateCount = 1;
       obji->defaultViewState = 1;
       obji->mouseDownViewState = 1;
       obji->viewDuration = 600;
       obji->mouseMotionScale = 180;
       obji->minPan = 0;
       obji->maxPan = 360;
       obji->defaultPan = 0;
       obji->minTilt = 72;
       obji->maxTilt = -72;
       obji->defaultTilt = 0;
       obji->minFOV = 0;
       obji->FOV = 64;
       obji->defaultFOV = 64;
       obji->defaultViewCenterH = 120;
       obji->defaultViewCenterV = 160;
       obji->viewRate = 1;
       obji->frameRate = 1;
       obji->controlSettings = 69;

       return 0;
}

int quicktime_obji_delete(quicktime_obji_t *obji)
{
    return 0;
}

void quicktime_obji_dump(quicktime_obji_t *obji)
{
       lqt_dump("object node (obji)\n");
       lqt_dump(" version %i\n", obji->version );
       lqt_dump(" revision %i\n", obji->revision );

       lqt_dump(" movie type %i\n", obji->movieType );
       lqt_dump(" view state count %i\n", obji->viewStateCount );
       lqt_dump(" default viewstate %i\n", obji->defaultViewState );
       lqt_dump(" mousedown viewstate %i\n", obji->mouseDownViewState );
       lqt_dump(" view duration %ld\n", obji->viewDuration );
       lqt_dump(" columns %ld\n", obji->columns );
       lqt_dump(" rows %ld\n", obji->rows );
       
       lqt_dump(" mouse motion scale %f\n", obji->mouseMotionScale);
       lqt_dump(" minimum pan %f\n", obji->minPan );
       lqt_dump(" maximum pan %f\n", obji->maxPan );
       lqt_dump(" default pan %f\n", obji->defaultPan );
       lqt_dump(" minimum tilt %f\n", obji->minTilt );
       lqt_dump(" maximum tilt %f\n", obji->maxTilt );
       lqt_dump(" default tilt %f\n", obji->defaultTilt );
       lqt_dump(" minimum fov %f\n", obji->minFOV );
       lqt_dump(" fov %f\n", obji->FOV );
       lqt_dump(" default fov %f\n", obji->defaultFOV );
       lqt_dump(" default horizontal viewcenter %f\n", obji->defaultViewCenterH );
       lqt_dump(" default vertical viewcenter %f\n", obji->defaultViewCenterV );
       lqt_dump(" view rate %f\n", obji->viewRate );
       lqt_dump(" frame rate %f\n", obji->frameRate );
       
       lqt_dump(" animation settings %ld\n", obji->animSettings );
       lqt_dump(" control settings %ld\n", obji->controlSettings );
}

int quicktime_read_obji(quicktime_t *file, quicktime_obji_t *obji)
{
       obji->version = quicktime_read_int16(file);
       obji->revision = quicktime_read_int16(file);

       obji->movieType = quicktime_read_int16(file);
       obji->viewStateCount = quicktime_read_int16(file);
       obji->defaultViewState = quicktime_read_int16(file);
       obji->mouseDownViewState = quicktime_read_int16(file);
       
       obji->viewDuration = quicktime_read_int32(file);
       obji->columns = quicktime_read_int32(file);
       obji->rows = quicktime_read_int32(file);
       
       obji->mouseMotionScale = quicktime_read_float32(file);
       obji->minPan = quicktime_read_float32(file);
       obji->maxPan = quicktime_read_float32(file);
       obji->defaultPan = quicktime_read_float32(file);
       obji->minTilt = quicktime_read_float32(file);
       obji->maxTilt = quicktime_read_float32(file);
       obji->defaultTilt = quicktime_read_float32(file);
       obji->minFOV = quicktime_read_float32(file);
       obji->FOV = quicktime_read_float32(file);
       obji->defaultFOV = quicktime_read_float32(file);
       obji->defaultViewCenterH = quicktime_read_float32(file);
       obji->defaultViewCenterV = quicktime_read_float32(file);
       obji->viewRate = quicktime_read_float32(file);
       obji->frameRate = quicktime_read_float32(file);

       obji->animSettings = quicktime_read_int32(file);
       obji->controlSettings = quicktime_read_int32(file);

       return 0;
}

void quicktime_write_obji(quicktime_t *file, quicktime_obji_t *obji)
{
	quicktime_write_int16(file, obji->version);
	quicktime_write_int16(file, obji->revision);
	quicktime_write_int16(file, obji->movieType);
	
	quicktime_write_int16(file, obji->viewStateCount);
	quicktime_write_int16(file, obji->defaultViewState);
	quicktime_write_int16(file, obji->mouseDownViewState);
	
	quicktime_write_int32(file, obji->viewDuration);
	quicktime_write_int32(file, obji->columns);
	quicktime_write_int32(file, obji->rows);
	
	quicktime_write_float32(file, obji->mouseMotionScale);
	quicktime_write_float32(file, obji->minPan);
	quicktime_write_float32(file, obji->maxPan);
	quicktime_write_float32(file, obji->defaultPan);
	quicktime_write_float32(file, obji->minTilt);
	quicktime_write_float32(file, obji->maxTilt);
	quicktime_write_float32(file, obji->defaultTilt);
	quicktime_write_float32(file, obji->minFOV);
	quicktime_write_float32(file, obji->FOV);
	quicktime_write_float32(file, obji->defaultFOV);
	quicktime_write_float32(file, obji->defaultViewCenterH);
	quicktime_write_float32(file, obji->defaultViewCenterV);
	quicktime_write_float32(file, obji->viewRate);
	quicktime_write_float32(file, obji->frameRate);
	
	quicktime_write_int32(file, obji->animSettings);
	quicktime_write_int32(file, obji->controlSettings);
	return;
}


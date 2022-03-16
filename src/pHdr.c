/*******************************************************************************
 pHdr.c

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

int quicktime_pHdr_init(quicktime_pHdr_t *pHdr)
{

    pHdr->nodeID = 1;
    pHdr->defHPan = 0;
    pHdr->defVPan = 0;
    pHdr->defZoom = 0;

    // constraints for this node; use zero for default
    pHdr->minHPan = 0;
    pHdr->minVPan = 0;
    pHdr->minZoom = 0;
    pHdr->maxHPan = 0;
    pHdr->maxVPan = 0;
    pHdr->maxZoom = 0;

    pHdr->nameStrOffset = 0;        // offset into string table atom
    pHdr->commentStrOffset = 0;    // offset into string table atom

    return 0;
}

int quicktime_pHdr_delete(quicktime_pHdr_t *pHdr)
{
    return 0;
}

void quicktime_pHdr_dump(quicktime_pHdr_t *pHdr)
{

}

int quicktime_read_pHdr(quicktime_t *file, quicktime_pHdr_t *pHdr, quicktime_atom_t *pHdr_atom)
{

       return 0;
}

void quicktime_write_pHdr(quicktime_t *file, quicktime_pHdr_t *pHdr)
{

}


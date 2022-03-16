/*******************************************************************************
 impn.c

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

int quicktime_impn_init(quicktime_impn_t *impn)
{
	impn->version = 2;
	impn->revision = 0;
	impn->imagingMode = 2;
	impn->imagingValidFlags = 7;
	impn->correction = 2;
	impn->quality = 256;
	impn->directdraw = 1;
	return 0;
}

int quicktime_impn_delete(quicktime_impn_t *impn)
{
    return 0;
}

void quicktime_impn_dump(quicktime_impn_t *impn)
{
	lqt_dump("        Panorama Imaging Atom (impn)\n");
	lqt_dump("         Version %i\n",  impn->version);
	lqt_dump("         Revision %i\n",  impn->revision);
	lqt_dump("         imagingMode %d\n",  impn->imagingMode);
	lqt_dump("         imagingValidFlags %d\n", impn->imagingValidFlags);
	lqt_dump("         Correction %d\n", impn->correction);
	lqt_dump("         Quality %d\n", impn->quality);
	lqt_dump("         directdraw %d\n", impn->directdraw);
	lqt_dump("         Imaging Properties %d %d %d %d %d %d \n", impn->imagingProperties[0],
	                                                             impn->imagingProperties[1],
	                                                             impn->imagingProperties[2],
	                                                             impn->imagingProperties[3],
	                                                             impn->imagingProperties[4],
	                                                             impn->imagingProperties[5]);

}

int quicktime_read_impn(quicktime_t *file, quicktime_impn_t *impn, quicktime_qtatom_t *impn_atom)
{
	impn->version =  quicktime_read_int16(file);
	impn->revision = quicktime_read_int16(file);
	impn->imagingMode = quicktime_read_int32(file);
	impn->imagingValidFlags = quicktime_read_int32(file);
	impn->correction = quicktime_read_int32(file);
	impn->quality = quicktime_read_int32(file);
	impn->directdraw = quicktime_read_int32(file);
	impn->imagingProperties[0] = quicktime_read_int32(file);
	impn->imagingProperties[1] = quicktime_read_int32(file);
	impn->imagingProperties[2] = quicktime_read_int32(file);
	impn->imagingProperties[3] = quicktime_read_int32(file);
	impn->imagingProperties[4] = quicktime_read_int32(file);
	impn->imagingProperties[5] = quicktime_read_int32(file);
	impn->reserved1 = quicktime_read_int32(file);
	impn->reserved2 = quicktime_read_int32(file);
	return 0;
}

void quicktime_write_impn(quicktime_t *file, quicktime_impn_t *impn)
{
	quicktime_qtatom_t atom;
	quicktime_qtatom_write_header(file, &atom, "impn", 1);
	quicktime_write_int16(file, impn->version);
	quicktime_write_int16(file, impn->revision);
	quicktime_write_int32(file, impn->imagingMode);
	quicktime_write_int32(file, impn->imagingValidFlags);
	quicktime_write_int32(file, impn->correction);
	quicktime_write_int32(file, impn->quality);
	quicktime_write_int32(file, impn->directdraw);
	quicktime_write_int32(file, impn->imagingProperties[0]);
	quicktime_write_int32(file, impn->imagingProperties[1]);
	quicktime_write_int32(file, impn->imagingProperties[2]);
	quicktime_write_int32(file, impn->imagingProperties[3]);
	quicktime_write_int32(file, impn->imagingProperties[4]);
	quicktime_write_int32(file, impn->imagingProperties[5]);
	quicktime_write_int32(file, impn->reserved1);
	quicktime_write_int32(file, impn->reserved2);
	quicktime_qtatom_write_footer(file, &atom);
}


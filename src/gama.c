/*******************************************************************************
 gama.c

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
#include <string.h>

void quicktime_gama_init(quicktime_gama_t *gama)
{
	memset(gama, 0, sizeof (*gama));
}

void quicktime_gama_delete(quicktime_gama_t *gama) { }

void quicktime_gama_dump(quicktime_gama_t *gama)
{

        lqt_dump("     Gamma value (gama): %f\n", gama->gamma);
}

void quicktime_read_gama(quicktime_t *file, quicktime_gama_t *gama)
{
	gama->gamma = quicktime_read_fixed32(file);
}

void quicktime_write_gama(quicktime_t *file, quicktime_gama_t *gama)
{
	quicktime_atom_t atom;

	quicktime_atom_write_header(file, &atom, "gama");
	quicktime_write_fixed32(file, gama->gamma);
	quicktime_atom_write_footer(file, &atom);
}

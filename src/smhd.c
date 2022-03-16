/*******************************************************************************
 smhd.c

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

void quicktime_smhd_init(quicktime_smhd_t *smhd)
{
	smhd->version = 0;
	smhd->flags = 0;
	smhd->balance = 0;
	smhd->reserved = 0;
}

void quicktime_smhd_delete(quicktime_smhd_t *smhd)
{
}

void quicktime_smhd_dump(quicktime_smhd_t *smhd)
{
        lqt_dump("    sound media header (smhd)\n");
	lqt_dump("     version %d\n", smhd->version);
	lqt_dump("     flags %ld\n", smhd->flags);
	lqt_dump("     balance %d\n", smhd->balance);
	lqt_dump("     reserved %d\n", smhd->reserved);
}

void quicktime_read_smhd(quicktime_t *file, quicktime_smhd_t *smhd)
{
	smhd->version = quicktime_read_char(file);
	smhd->flags = quicktime_read_int24(file);
	smhd->balance = quicktime_read_int16(file);
	smhd->reserved = quicktime_read_int16(file);
}

void quicktime_write_smhd(quicktime_t *file, quicktime_smhd_t *smhd)
{
	quicktime_atom_t atom;
	quicktime_atom_write_header(file, &atom, "smhd");

	quicktime_write_char(file, smhd->version);
	quicktime_write_int24(file, smhd->flags);
	quicktime_write_int16(file, smhd->balance);
	quicktime_write_int16(file, smhd->reserved);

	quicktime_atom_write_footer(file, &atom);
}

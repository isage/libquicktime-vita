/*******************************************************************************
 matrix.c

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

void quicktime_matrix_init(quicktime_matrix_t *matrix)
{
	int i;
	for(i = 0; i < 9; i++) matrix->values[i] = 0;
	matrix->values[0] = matrix->values[4] = 1;
	matrix->values[8] = 16384;
}

void quicktime_matrix_delete(quicktime_matrix_t *matrix)
{
}

void quicktime_read_matrix(quicktime_t *file, quicktime_matrix_t *matrix)
{
	int i = 0;
	for(i = 0; i < 9; i++)
	{
		matrix->values[i] = quicktime_read_fixed32(file);
	}
}

void quicktime_matrix_dump(quicktime_matrix_t *matrix)
{
	int i;
	lqt_dump("   matrix");
	for(i = 0; i < 9; i++) lqt_dump(" %f", matrix->values[i]);
	lqt_dump("\n");
}

void quicktime_write_matrix(quicktime_t *file, quicktime_matrix_t *matrix)
{
	int i;
	for(i = 0; i < 9; i++)
	{
		quicktime_write_fixed32(file, matrix->values[i]);
	}
}

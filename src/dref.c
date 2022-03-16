/*******************************************************************************
 dref.c

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
#include <stdlib.h>
#include <string.h>

void quicktime_dref_table_init(quicktime_dref_table_t *table, lqt_file_type_t type)
{
	table->size = 0;
        if(IS_MP4(type))
          {
          table->type[0] = 'u';
          table->type[1] = 'r';
          table->type[2] = 'l';
          table->type[3] = ' ';
          }
        else
          {
          table->type[0] = 'a';
          table->type[1] = 'l';
          table->type[2] = 'i';
          table->type[3] = 's';
          }
	table->version = 0;
	table->flags = 0x0001;
	table->data_reference = malloc(256);
	table->data_reference[0] = 0;
}

void quicktime_dref_table_delete(quicktime_dref_table_t *table)
{
	if(table->data_reference) free(table->data_reference);
	table->data_reference = 0;
}

void quicktime_read_dref_table(quicktime_t *file, quicktime_dref_table_t *table)
{
	table->size = quicktime_read_int32(file);
	quicktime_read_char32(file, table->type);
	table->version = quicktime_read_char(file);
	table->flags = quicktime_read_int24(file);
	if(table->data_reference) free(table->data_reference);

	table->data_reference = malloc(table->size);
	if(table->size > 12)
          quicktime_read_data(file, (uint8_t*)(table->data_reference), table->size - 12);
	table->data_reference[table->size - 12] = 0;
}

void quicktime_write_dref_table(quicktime_t *file, quicktime_dref_table_t *table)
{
	int len = strlen(table->data_reference);
	quicktime_write_int32(file, 12 + len);
	quicktime_write_char32(file, table->type);
	quicktime_write_char(file, table->version);
	quicktime_write_int24(file, table->flags);
	if(len)
          quicktime_write_data(file, (uint8_t*)(table->data_reference), len);
}

void quicktime_dref_table_dump(quicktime_dref_table_t *table)
{
	lqt_dump("      data reference table (dref)\n");
	lqt_dump("       type %c%c%c%c\n", table->type[0], table->type[1], table->type[2], table->type[3]);
	lqt_dump("       version %d\n", table->version);
	lqt_dump("       flags %ld\n", table->flags);
	lqt_dump("       data %s\n", table->data_reference);
}


void quicktime_dref_init(quicktime_dref_t *dref)
{
	dref->version = 0;
	dref->flags = 0;
	dref->total_entries = 0;
	dref->table = 0;
}

void quicktime_dref_init_all(quicktime_dref_t *dref, lqt_file_type_t type)
{
	if(!dref->total_entries)
	{
		dref->total_entries = 1;
		dref->table = (quicktime_dref_table_t *)malloc(sizeof(quicktime_dref_table_t) * dref->total_entries);
		quicktime_dref_table_init(&dref->table[0], type);
	}
}

void quicktime_dref_delete(quicktime_dref_t *dref)
{
	if(dref->table)
	{
		int i;
		for(i = 0; i < dref->total_entries; i++)
			quicktime_dref_table_delete(&dref->table[i]);
		free(dref->table);
	}
	dref->total_entries = 0;
}

void quicktime_dref_dump(quicktime_dref_t *dref)
{
	int i;
	
	lqt_dump("     data reference (dref)\n");
	lqt_dump("      version %d\n", dref->version);
	lqt_dump("      flags %ld\n", dref->flags);
	for(i = 0; i < dref->total_entries; i++)
	{
		quicktime_dref_table_dump(&dref->table[i]);
	}
}

void quicktime_read_dref(quicktime_t *file, quicktime_dref_t *dref)
{
	int i;

	dref->version = quicktime_read_char(file);
	dref->flags = quicktime_read_int24(file);
	dref->total_entries = quicktime_read_int32(file);
	dref->table = (quicktime_dref_table_t*)malloc(sizeof(quicktime_dref_table_t) * dref->total_entries);
	for(i = 0; i < dref->total_entries; i++)
	{
		quicktime_dref_table_init(&dref->table[i], file->file_type);
		quicktime_read_dref_table(file, &dref->table[i]);
	}
}

void quicktime_write_dref(quicktime_t *file, quicktime_dref_t *dref)
{
	int i;
	quicktime_atom_t atom;
	quicktime_atom_write_header(file, &atom, "dref");

	quicktime_write_char(file, dref->version);
	quicktime_write_int24(file, dref->flags);
	quicktime_write_int32(file, dref->total_entries);

	for(i = 0; i < dref->total_entries; i++)
	{
		quicktime_write_dref_table(file, &dref->table[i]);
	}
	quicktime_atom_write_footer(file, &atom);
}

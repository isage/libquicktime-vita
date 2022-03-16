/*******************************************************************************
 vrni.c

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

int quicktime_vrni_init(quicktime_vrni_t *vrnp)
{
	vrnp->ID = 1;
	quicktime_nloc_init(&vrnp->nloc);
	return 0;
}

int quicktime_vrni_delete(quicktime_vrni_t *vrni)
{
	return 0;
}

void quicktime_vrni_dump(quicktime_vrni_t *vrni)
{
	lqt_dump("         node id (vrni)\n");
	lqt_dump("          id %i\n", vrni->ID);
	quicktime_nloc_dump(&vrni->nloc);
}

int quicktime_read_vrni(quicktime_t *file, quicktime_vrni_t *vrni, quicktime_qtatom_t *vrni_atom)
{
	quicktime_qtatom_t leaf_atom;
	int result = 0;
	
	quicktime_qtatom_read_header(file, &leaf_atom);
	
	quicktime_read_nloc(file, &vrni->nloc, &leaf_atom);
	
	return result;
}

void quicktime_write_vrni(quicktime_t *file, quicktime_vrni_t *vrni )
{
	quicktime_qtatom_t atom;
	quicktime_qtatom_write_header(file, &atom, "vrni", 1);
	quicktime_write_nloc(file, &vrni->nloc);
	atom.child_count = 1;
	quicktime_qtatom_write_footer(file, &atom);

}


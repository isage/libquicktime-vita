/*******************************************************************************
 vrnp.c

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

int quicktime_vrnp_init(quicktime_vrnp_t *vrnp)
{
	vrnp->children = 0;
	quicktime_vrni_init(&vrnp->vrni[0]);
	return 0;
}

int quicktime_vrnp_delete(quicktime_vrnp_t *vrnp)
{
	return 0;
}

void quicktime_vrnp_dump(quicktime_vrnp_t *vrnp)
{
    	int i;
	lqt_dump("        node parent (vrnp)\n");
	lqt_dump("         nodes %i\n", vrnp->children);
	for (i = 0; i < vrnp->children; i++)
	{
	    quicktime_vrni_dump(&vrnp->vrni[i]);
	}
}

int quicktime_read_vrnp(quicktime_t *file, quicktime_vrnp_t *vrnp, quicktime_qtatom_t *vrnp_atom)
{
	quicktime_qtatom_t leaf_atom;
	int result = 0;
	int i;
	
	quicktime_qtatom_read_header(file, &leaf_atom);
	for (i = 0; i < vrnp->children; i++ )
	{
	   	vrnp->vrni[i].ID = leaf_atom.ID;
		result += quicktime_read_vrni(file, &vrnp->vrni[i], &leaf_atom);
	}
	
	return result;
}

void quicktime_write_vrnp(quicktime_t *file, quicktime_vrnp_t *vrnp )
{
	quicktime_qtatom_t atom;
	quicktime_qtatom_write_header(file, &atom, "vrnp", 1);
	atom.child_count = 1;
    	quicktime_write_vrni(file, &vrnp->vrni[0]);
	quicktime_qtatom_write_footer(file, &atom);

}


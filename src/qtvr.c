/*******************************************************************************
 qtvr.c

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

int quicktime_qtvr_init(quicktime_qtvr_t *qtvr)
{
	memset(qtvr, 0, sizeof(*qtvr));
	quicktime_imgp_init(&qtvr->imgp);
	quicktime_vrsc_init(&qtvr->vrsc);
	quicktime_vrnp_init(&qtvr->vrnp);
	return 0;
}

int quicktime_qtvr_delete(quicktime_qtvr_t *qtvr)
{
	return 0;
}

void quicktime_qtvr_dump(quicktime_qtvr_t *qtvr)
{
	lqt_dump("       qtvr world\n");
	quicktime_vrsc_dump(&qtvr->vrsc);
	quicktime_imgp_dump(&qtvr->imgp);
	quicktime_vrnp_dump(&qtvr->vrnp);
}

int quicktime_read_qtvr(quicktime_t *file, quicktime_qtvr_t *qtvr, quicktime_atom_t *qtvr_atom)
{
	quicktime_qtatom_t leaf_atom, root_atom;
	int result = 0;
	
	quicktime_qtatom_read_container_header(file);
	quicktime_qtatom_read_header(file, &root_atom);
	do
	{
	    	quicktime_qtatom_read_header(file, &leaf_atom);
		if(quicktime_qtatom_is(&leaf_atom, "vrsc"))
		{
		    	result += quicktime_read_vrsc(file, &qtvr->vrsc, &leaf_atom);
		} else
		if(quicktime_qtatom_is(&leaf_atom, "imgp"))
		{
		    	result += quicktime_read_imgp(file, &qtvr->imgp, &leaf_atom);
		} else
		if(quicktime_qtatom_is(&leaf_atom, "vrnp"))
		{
		    	qtvr->vrnp.children = leaf_atom.child_count;
		    	result += quicktime_read_vrnp(file, &qtvr->vrnp, &leaf_atom);
		} else
			quicktime_qtatom_skip(file, &leaf_atom);
	}while(quicktime_position(file) < root_atom.end);
	
	return result;
}

void quicktime_write_qtvr(quicktime_t *file, quicktime_qtvr_t *qtvr )
{
	quicktime_qtatom_t subatom;

	quicktime_qtatom_write_container_header(file);
	quicktime_qtatom_write_header(file, &subatom, "sean", 1);

   	subatom.child_count = 3;
	quicktime_write_vrsc(file, &qtvr->vrsc);
	quicktime_write_imgp(file, &qtvr->imgp);
	quicktime_write_vrnp(file, &qtvr->vrnp);

	quicktime_qtatom_write_footer(file, &subatom);

}


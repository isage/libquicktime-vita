/*******************************************************************************
 imgp.c

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

int quicktime_imgp_init(quicktime_imgp_t *imgp)
{
    	quicktime_impn_init(&imgp->impn);
	return 0;
}

int quicktime_imgp_delete(quicktime_imgp_t *imgp)
{
	return 0;
}

void quicktime_imgp_dump(quicktime_imgp_t *imgp)
{
	lqt_dump("        Imaging Parent (imgp)\n");
	quicktime_impn_dump(&imgp->impn);

}

int quicktime_read_imgp(quicktime_t *file, quicktime_imgp_t *imgp, quicktime_qtatom_t *imgp_atom)
{
	quicktime_qtatom_t leaf_atom;
	int result = 0;
	
	do
	{
	    	quicktime_qtatom_read_header(file, &leaf_atom);
		if(quicktime_qtatom_is(&leaf_atom, "impn"))
		{
		    	result += quicktime_read_impn(file, &imgp->impn, &leaf_atom);
		} else
			quicktime_qtatom_skip(file, &leaf_atom);
	}while(quicktime_position(file) < imgp_atom->end);
	
	return result;
}

void quicktime_write_imgp(quicktime_t *file, quicktime_imgp_t *imgp )
{
	quicktime_qtatom_t atom;
	quicktime_qtatom_write_header(file, &atom, "imgp", 1);
	quicktime_write_impn(file, &imgp->impn);

	atom.child_count = 1;
	
	quicktime_qtatom_write_footer(file, &atom);
}


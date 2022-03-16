/*******************************************************************************
 charset.h

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

#ifndef __LQT_CHARSET
#define __LQT_CHARSET

#define LQT_UTF_8_16 "lqt_utf_8_16"

typedef struct lqt_charset_converter_s lqt_charset_converter_t;

lqt_charset_converter_t *
lqt_charset_converter_create(quicktime_t * file, const char * src_charset,
                             const char * dst_charset);

/* We convert all strings "in place" */

void lqt_charset_convert(lqt_charset_converter_t * cnv,
                         char ** string, int in_len, int * out_len);

/* Convert string and realloc result */

void lqt_charset_convert_realloc(lqt_charset_converter_t * cnv,
                                 const char * in_str, int in_len,
                                 char ** out_str, int * out_alloc, int * out_len);


void lqt_charset_converter_destroy(lqt_charset_converter_t *);

#endif

/*******************************************************************************
 lqt_version.h

 libquicktime - A library for reading and writing quicktime/avi/mp4 files.
 http://libquicktime.sourceforge.net

 Copyright (C) 2002 Heroine Virtual Ltd.
 Copyright (C) 2002-2007 Members of the libquicktime project.

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

#ifndef __LQT_VERSION_H_
#define __LQT_VERSION_H_

/*
 *  Codec API version
 *  This should be increased (by one or so) after the codec API
 *  changes. This will happen sometimes during the development
 *  cycle but is hopefully constant afterwards
 */

#define LQT_CODEC_API_VERSION 12

/*
 * libquicktime library version
 */

#define LQT_VERSION "1.2.4"

#define LQT_VERSION_MAJOR 1
#define LQT_VERSION_MINOR 2
#define LQT_VERSION_MICRO 4

#define LQT_MAKE_BUILD(a,b,c) ((a << 16) + (b << 8) + c)

#define LQT_BUILD \
LQT_MAKE_BUILD(LQT_VERSION_MAJOR, LQT_VERSION_MINOR, LQT_VERSION_MICRO)

#endif /* __LQT_VERSION_H_ */

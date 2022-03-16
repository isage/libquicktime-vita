/*******************************************************************************
 lqt_qtvr.h

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

#ifndef _LQT_QTVR_H_
#define _LQT_QTVR_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#pragma GCC visibility push(default)
  
/* QTVR stuff */
#define QTVR_PANO_HORZ 1
#define QTVR_PANO_VERT 2

#define QTVR_QTVR_OBJ 1
#define QTVR_QTVR_PAN 2
#define QTVR_OBJ 3
#define QTVR_PAN 4

/* check if the file is a qtvr file */
/* return values:                                                             */
/* QTVR_OBJ  = file is object movie                                           */
/* QTVR_PAN  = file is panorama                                               */
int lqt_is_qtvr(quicktime_t  *file);


/* initialize qtvr atoms/track */
/* type: QTVR_OBJ == object movie                                             */
/*       QTVR_PAN == panorama movie                                           */
int lqt_qtvr_set_type(quicktime_t  *file,
		      int type,
		      int width,
		      int height,
		      int duration,
		      int time_scale,
		      int scene_track);


/* return full dimensions of the movie*/
int lqt_qtvr_get_width(quicktime_t  *file);
int lqt_qtvr_get_height(quicktime_t  *file);

/* get depth of the movie */
int lqt_qtvr_get_depth(quicktime_t  *file);

/* get number of rows/columns */
int lqt_qtvr_get_rows(quicktime_t  *file);
int lqt_qtvr_get_columns(quicktime_t  *file);

/* set number of columns/rows */
int lqt_qtvr_set_rows(quicktime_t  *file, int rows);
int lqt_qtvr_set_columns(quicktime_t  *file, int columns);

/* get initial frame to display */
int lqt_qtvr_get_initial_position(quicktime_t  *file);

/* panning, tilting, fov, movietype */
/* optional parameters that define player behavior                            */

/* Object movies:                                                             */
/* get/set the panning/zoom settings                                          */
/* starthpan == 0 and endhpan == 360 means continous horiz. panning           */
/* Most (all?) object movies use these values:                                */
/* starthpan = 0 (angle of thehorizontal starting position)                   */
/* endhpan = 360                                                              */
/* startvpan = 90                                                             */
/* endvpan = -90                                                              */
/* minzoom/maxzoom are ignored                                                */
/*                                                                            */
/* Note: In Apples Player they affect the mouse sensivity.                    */

/* Panoramas:                                                                 */
/* starthpan = 0                                                              */
/* endhpan = 360                                                              */
/* startvpan = ???                                                            */
/* endvpan = ???                                                              */
/* minzoom/maxzoom: restrict zoom levels (Panorama only)                      */
/*                                                                            */
/* Note: For panoramas endvpan/startvpan have to be adjusted for every movie  */

/* To ignore a parameter set it to NULL                                       */

// TODO: update above

/* get/set panning */
void lqt_qtvr_get_pan(quicktime_t  *file, float *minpan, float *maxpan, float *defpan);
void lqt_qtvr_set_pan(quicktime_t  *file, float minpan, float maxpan, float defpan);

/* get/set tilting */
void lqt_qtvr_get_tilt(quicktime_t  *file, float *mintilt, float *maxtilt, float *deftilt);
void lqt_qtvr_set_tilt(quicktime_t  *file, float mintilt, float maxtilt, float deftilt);

/* get/set fov */
void lqt_qtvr_get_fov(quicktime_t  *file, float *minfov, float *maxfov, float *deffov);
void lqt_qtvr_set_fov(quicktime_t  *file, float minfov, float maxfov, float deffov);

/* get number of loop frames */
int lqt_qtvr_get_loop_frames(quicktime_t  *file);

/* movietype
 * get/set the controller type
 * These controllers are possible:
 * QTVR_GRABBER_SCROLLER_UI hand cursor, view rotates in movement direction
 *                          rotation arrows
 * QTVR_OLD_JOYSTICK_UI "Joystick interface", rotation against movement
 *                      direction
 * QTVR_JOYSTICK_UI "Joystick Interface", rotation in movement direction
 * QTVR_GRABBER_UI hand cursor. no rotation arrows
 * QTVR_ABSOLUTE_UI absolute interface
 */
int lqt_qtvr_get_movietype(quicktime_t  *file);
int lqt_qtvr_set_movietype(quicktime_t  *file, int movietype);

/* get object track*/
int lqt_qtvr_get_object_track(quicktime_t  *file);


/* get qtvr track*/
int lqt_qtvr_get_qtvr_track(quicktime_t  *file);

/* Panorama specific */

/* get/set the dimensions of the player window */
int lqt_qtvr_set_display_width(quicktime_t  *file, int width);
int lqt_qtvr_set_display_height(quicktime_t  *file, int height);
int lqt_qtvr_get_display_width(quicktime_t  *file);
int lqt_qtvr_get_display_height(quicktime_t  *file);

/* get panorama track */
int lqt_qtvr_get_panorama_track(quicktime_t  *file);

/* get the image track */
int lqt_qtvr_get_image_track(quicktime_t  *file);

/* set the scene track */
int lqt_qtvr_set_image_track(quicktime_t  *file, int track);

int lqt_qtvr_add_panorama_node(quicktime_t *file);
int lqt_qtvr_add_object_node(quicktime_t *file);

int lqt_qtvr_get_panotype(quicktime_t *file);

#pragma GCC visibility pop
  
#ifdef __cplusplus
}
#endif /* __cplusplus */


  
#endif

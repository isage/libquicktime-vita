/*******************************************************************************
 lqt_qtvr.c

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

#define LOG_DOMAIN "qtvr"

/* util */

static int deg2frame(quicktime_t *file, float hdeg, float vdeg)
{
    return ((hdeg * abs(vdeg-90)) / 64800) * 
           lqt_qtvr_get_columns(file) *
           lqt_qtvr_get_rows(file) * 
           file->moov.udta.navg.loop_frames;
}

int lqt_qtvr_add_object_node(quicktime_t *file)
{
    quicktime_qtatom_t root_atom, leaf_atom;
    quicktime_trak_t *trak;
    
    quicktime_ndhd_init(&file->qtvr_node[0].ndhd);
    trak = file->moov.trak[lqt_qtvr_get_qtvr_track(file)];
    
    if (quicktime_track_samples(file, trak) > 0) {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN,
                "lqt_qtvr_add_node only single node movies supported.");
      return -1;
    }
    quicktime_write_chunk_header(file, trak);
    quicktime_qtatom_write_container_header(file);
    quicktime_qtatom_write_header(file, &root_atom, "sean", 1);
    root_atom.child_count = 1;
    quicktime_qtatom_write_header(file, &leaf_atom, "ndhd", 1);
    quicktime_write_ndhd(file, &file->qtvr_node[0].ndhd);
    quicktime_qtatom_write_footer(file, &leaf_atom);
    quicktime_qtatom_write_footer(file, &root_atom);
    trak->chunk_samples = 1;
    quicktime_write_chunk_footer(file, trak);
    trak->mdia.minf.stbl.stsd.table->qtvr.vrnp.children++;
    trak->mdia.minf.stbl.stsd.table->qtvr.vrnp.vrni->nloc.nodeType[0] = 'o';
    trak->mdia.minf.stbl.stsd.table->qtvr.vrnp.vrni->nloc.nodeType[1] = 'b';
    trak->mdia.minf.stbl.stsd.table->qtvr.vrnp.vrni->nloc.nodeType[2] = 'j';
    trak->mdia.minf.stbl.stsd.table->qtvr.vrnp.vrni->nloc.nodeType[3] = 'e';

    trak->mdia.minf.stbl.stts.table[0].sample_count = 1;
    
    /* set duration to duration of image track */
    /* works with 1 too but this seems to be more correct */
    trak->mdia.minf.stbl.stts.table[0].sample_duration = 
    	file->qtvr_node[0].obji.rows * file->qtvr_node[0].obji.columns;
    trak->tref.references[0].tracks[0] =
      file->moov.trak[lqt_qtvr_get_object_track(file)]->tkhd.track_id;
    
    trak = file->moov.trak[lqt_qtvr_get_object_track(file)];

    quicktime_write_chunk_header(file, trak);
    quicktime_qtatom_write_container_header(file);
    quicktime_qtatom_write_header(file, &root_atom, "sean", 1);
    root_atom.child_count = 1;
    quicktime_qtatom_write_header(file, &leaf_atom, "obji", 1);
    leaf_atom.child_count = 0;
    quicktime_write_obji(file, &file->qtvr_node[0].obji);

    quicktime_qtatom_write_footer(file, &leaf_atom);
    quicktime_qtatom_write_footer(file, &root_atom);
    trak->chunk_samples = 1;
    quicktime_write_chunk_footer(file, trak);
    trak->mdia.minf.stbl.stts.table[0].sample_count = 1;
    /* set duration to duration of image track */
    trak->mdia.minf.stbl.stts.table[0].sample_duration = file->qtvr_node[0].obji.rows * file->qtvr_node[0].obji.columns;
    return 0;
}

int lqt_qtvr_add_panorama_node(quicktime_t *file)
{
    quicktime_qtatom_t root_atom, leaf_atom;
    quicktime_trak_t *trak;
    
    quicktime_ndhd_init(&file->qtvr_node[0].ndhd);
    file->qtvr_node[0].ndhd.nodeType[0] = 'p';
    file->qtvr_node[0].ndhd.nodeType[1] = 'a';
    file->qtvr_node[0].ndhd.nodeType[2] = 'n';
    file->qtvr_node[0].ndhd.nodeType[3] = 'o';

    file->qtvr_node[0].pdat.imageSizeX = quicktime_video_width(file, 0); // could be a different track
    file->qtvr_node[0].pdat.imageSizeY = quicktime_video_height(file, 0) *  file->qtvr_node[0].pdat.imageNumFramesY;
    
    
    trak = file->moov.trak[lqt_qtvr_get_qtvr_track(file)];
    
    if (quicktime_track_samples(file, trak) > 0) {
      lqt_log(file, LQT_LOG_ERROR, LOG_DOMAIN,
                "lqt_qtvr_add_node only single node movies supported.");
      return -1;
    }
    quicktime_write_chunk_header(file, trak);
    quicktime_qtatom_write_container_header(file);
    quicktime_qtatom_write_header(file, &root_atom, "sean", 1);
    root_atom.child_count = 1;
    quicktime_qtatom_write_header(file, &leaf_atom, "ndhd", 1);
    quicktime_write_ndhd(file, &file->qtvr_node[0].ndhd);
    quicktime_qtatom_write_footer(file, &leaf_atom);
    quicktime_qtatom_write_footer(file, &root_atom);
    trak->chunk_samples = 1;
    quicktime_write_chunk_footer(file, trak);
    trak->mdia.minf.stbl.stsd.table->qtvr.vrnp.children++;
    trak->mdia.minf.stbl.stsd.table->qtvr.vrnp.vrni->nloc.nodeType[0] = 'p';
    trak->mdia.minf.stbl.stsd.table->qtvr.vrnp.vrni->nloc.nodeType[1] = 'a';
    trak->mdia.minf.stbl.stsd.table->qtvr.vrnp.vrni->nloc.nodeType[2] = 'n';
    trak->mdia.minf.stbl.stsd.table->qtvr.vrnp.vrni->nloc.nodeType[3] = 'o';
    trak->mdia.minf.stbl.stts.table[0].sample_count = 1;
    
    /* set duration to duration of image track */
    /* works with 1 too but this seems more correct */
    trak->mdia.minf.stbl.stts.table[0].sample_duration = 
   	file->qtvr_node[0].pdat.imageNumFramesX * 
    file->qtvr_node[0].pdat.imageNumFramesX;
    trak->tref.references[0].tracks[0] =
    file->moov.trak[lqt_qtvr_get_panorama_track(file)]->tkhd.track_id;

    trak = file->moov.trak[lqt_qtvr_get_panorama_track(file)];

    quicktime_write_chunk_header(file, trak);
    quicktime_qtatom_write_container_header(file);
    quicktime_qtatom_write_header(file, &root_atom, "sean", 1);
    root_atom.child_count = 1;
    quicktime_qtatom_write_header(file, &leaf_atom, "pdat", 1);
    leaf_atom.child_count = 0;
    quicktime_write_pdat(file, &file->qtvr_node[0].pdat);

    quicktime_qtatom_write_footer(file, &leaf_atom);
    quicktime_qtatom_write_footer(file, &root_atom);
    trak->chunk_samples = 1;
    quicktime_write_chunk_footer(file, trak);


    trak->mdia.minf.stbl.stts.table[0].sample_count = 1;
    /* set duration to duration of image track */
    trak->mdia.minf.stbl.stts.table[0].sample_duration =
    file->qtvr_node[0].pdat.imageNumFramesX * 
    file->qtvr_node[0].pdat.imageNumFramesX;

    return 0;
}


/* type of file */
int lqt_is_qtvr(quicktime_t  *file)
{
    if (file->moov.udta.is_qtvr) {
        if (quicktime_match_32(file->moov.udta.ctyp, "stna") ||
            (lqt_qtvr_get_object_track(file) >= 0)) 
            return QTVR_OBJ;

        if (quicktime_match_32(file->moov.udta.ctyp, "STpn") ||
            (lqt_qtvr_get_panorama_track(file) >= 0))
            return QTVR_PAN;
    }
    return 0;
}


/* type of qtvr to write */
int lqt_qtvr_set_type(quicktime_t  *file, int type, int width, int height, int duration, int time_scale, int scene_track)
{
    quicktime_trak_t *trak;

    if (time_scale < 1 || 
        duration < 1 ||
        scene_track >= file->total_vtracks || 
        scene_track < 0)
        return -1;

    if (type == QTVR_OBJ) {
        file->moov.udta.ctyp[0] = 'q';
        file->moov.udta.ctyp[1] = 't';
        file->moov.udta.ctyp[2] = 'v';
        file->moov.udta.ctyp[3] = 'r';
        file->moov.udta.is_qtvr = 1;

        trak = quicktime_add_track(file);
        quicktime_trak_init_qtvr(file, trak, QTVR_OBJ, width, height, duration, time_scale);
        quicktime_obji_init(&file->qtvr_node[0].obji);
        file->qtvr_node[0].obji.viewDuration = duration;

        trak = quicktime_add_track(file);
        quicktime_trak_init_qtvr(file, trak, QTVR_QTVR_OBJ, width, height, duration, time_scale);

        lqt_qtvr_set_display_width(file, width);
        lqt_qtvr_set_display_height(file, height);
        
        lqt_qtvr_set_image_track(file, scene_track);

        return 0;
    }

    if (type == QTVR_PAN) {
        file->moov.udta.ctyp[0] = 'q';
        file->moov.udta.ctyp[1] = 't';
        file->moov.udta.ctyp[2] = 'v';
        file->moov.udta.ctyp[3] = 'r';
        file->moov.udta.is_qtvr = 1;

        trak = quicktime_add_track(file);
        quicktime_trak_init_qtvr(file, trak, QTVR_PAN, width, height, duration, time_scale);
        quicktime_pdat_init(&file->qtvr_node[0].pdat);

        trak = quicktime_add_track(file);
        quicktime_trak_init_qtvr(file, trak, QTVR_QTVR_PAN, width, height, duration, time_scale);

        /* set initial scene track*/
        lqt_qtvr_set_image_track(file, scene_track);

        return 0;
    }

    file->moov.udta.is_qtvr = 0;
    return -1;
}

/* Number of frames in loop */
int lqt_qtvr_get_loop_frames(quicktime_t  *file)
{
    if (lqt_qtvr_get_object_track(file) >= 0) {
        return file->qtvr_node[0].obji.viewDuration / file->moov.trak[lqt_track_from_id(file, lqt_qtvr_get_image_track(file))]->mdia.minf.stbl.stts.table[0].sample_duration;
    }
    else {
        return file->moov.udta.navg.loop_frames;
    }
}

/* Number of rows */
int lqt_qtvr_get_rows(quicktime_t  *file)
{
    if (lqt_is_qtvr(file) == QTVR_OBJ) {
        if (lqt_qtvr_get_object_track(file) >= 0) {
            return file->qtvr_node[0].obji.rows;
        }
        else {
            return file->moov.udta.navg.rows;
        }
    }
    else
    if (lqt_is_qtvr(file) == QTVR_PAN) {
        if (lqt_qtvr_get_qtvr_track(file) >= 0) {
            if (lqt_qtvr_get_panotype(file) == QTVR_PANO_HORZ) { 
                return file->qtvr_node[0].pdat.imageNumFramesY;
            }
            else {
                return file->qtvr_node[0].pdat.imageNumFramesX;
            }
        }
        else {
            return file->moov.trak[lqt_qtvr_get_panorama_track(file)]->mdia.minf.stbl.stsd.table->pano.SNumFramesHeight;
        }
    }
    return -1;
}

/* Number of columns */
int lqt_qtvr_get_columns(quicktime_t  *file)
{
    if (lqt_is_qtvr(file) == QTVR_OBJ) {
        if (lqt_qtvr_get_object_track(file) >= 0) {
            return file->qtvr_node[0].obji.columns;
        }
        else {
            return file->moov.udta.navg.columns;
        }
    }
    else if (lqt_is_qtvr(file) == QTVR_PAN) {
        if (lqt_qtvr_get_qtvr_track(file) >= 0) {
            if (lqt_qtvr_get_panotype(file) == QTVR_PANO_HORZ) { 
                return file->qtvr_node[0].pdat.imageNumFramesX;
            }
            else {
                return file->qtvr_node[0].pdat.imageNumFramesY;
            }
        }
        else {
            return file->moov.trak[lqt_qtvr_get_panorama_track(file)]->mdia.minf.stbl.stsd.table->pano.SNumFramesWidth;
        }
    }

    return -1;
}

/* pan  (H) */
void lqt_qtvr_get_pan(quicktime_t  *file, float *minpan, float *maxpan, float *defpan)
{
    if (lqt_qtvr_get_object_track(file) >= 0) {
        if (minpan != NULL ) *minpan = file->qtvr_node[0].obji.minPan;
        if (maxpan != NULL ) *maxpan = file->qtvr_node[0].obji.maxPan;
        if (defpan != NULL ) *maxpan = file->qtvr_node[0].obji.defaultPan;
    }
    else {
        if (minpan != NULL ) *minpan = file->moov.udta.navg.startHPan;
        if (maxpan != NULL ) *maxpan = file->moov.udta.navg.endHPan;
        if (defpan != NULL ) *defpan = file->moov.udta.navg.initialHPan;
    }
}

void lqt_qtvr_set_pan(quicktime_t  *file, float minpan, float maxpan, float defpan)
{
    if (lqt_qtvr_get_object_track(file) >= 0) {
        file->qtvr_node[0].obji.minPan = minpan;
        file->qtvr_node[0].obji.maxPan = maxpan;
        file->qtvr_node[0].obji.defaultPan = defpan;
    }
    else {
        file->moov.udta.navg.startHPan = minpan;
        file->moov.udta.navg.endHPan = maxpan;
        file->moov.udta.navg.initialHPan = defpan;
    }
}

/* tilt  (V) */
void lqt_qtvr_get_tilt(quicktime_t  *file, float *mintilt, float *maxtilt, float *deftilt)
{
    if (lqt_qtvr_get_object_track(file) >= 0) {
        if (mintilt != NULL ) *mintilt = file->qtvr_node[0].obji.minTilt;
        if (maxtilt != NULL ) *maxtilt = file->qtvr_node[0].obji.maxTilt;
        if (deftilt != NULL ) *maxtilt = file->qtvr_node[0].obji.defaultTilt;
    }
    else {
        if (mintilt != NULL ) *mintilt = file->moov.udta.navg.startVPan;
        if (maxtilt != NULL ) *maxtilt = file->moov.udta.navg.endVPan;
        if (deftilt != NULL ) *deftilt = file->moov.udta.navg.initialVPan;
    }
}

void lqt_qtvr_set_tilt(quicktime_t  *file, float mintilt, float maxtilt, float deftilt)
{
    if (lqt_qtvr_get_object_track(file) >= 0) {
        file->qtvr_node[0].obji.minTilt = mintilt;
        file->qtvr_node[0].obji.maxTilt = maxtilt;
        file->qtvr_node[0].obji.defaultTilt = deftilt;
    }
    else {
        file->moov.udta.navg.startHPan = mintilt;
        file->moov.udta.navg.endHPan = maxtilt;
        file->moov.udta.navg.initialHPan = deftilt;
    }
}

/* FOV  "Zoom" */
void lqt_qtvr_get_fov(quicktime_t  *file, float *minfov, float *maxfov, float *deffov)
{
    if (lqt_qtvr_get_qtvr_track(file) >= 0 ) {
        if (lqt_qtvr_get_object_track(file) >= 0) {
            if (minfov != NULL ) *minfov = file->qtvr_node[0].obji.minFOV;
            if (maxfov != NULL ) *maxfov = file->qtvr_node[0].obji.FOV;
            if (deffov != NULL ) *deffov = file->qtvr_node[0].obji.defaultFOV;
        }
        else if (lqt_qtvr_get_panorama_track(file) >= 0) {
            if (minfov != NULL) *minfov = file->qtvr_node[0].pdat.minFOV;
            if (maxfov != NULL) *maxfov = file->qtvr_node[0].pdat.maxFOV;
            if (deffov != NULL) *deffov = file->qtvr_node[0].pdat.defaultFOV;
        }
    }
    else {
        if (lqt_is_qtvr(file) == QTVR_OBJ) {
            if (minfov != NULL ) *minfov = file->moov.udta.navg.fieldofview;
            if (maxfov != NULL ) *maxfov = file->moov.udta.navg.fieldofview;
            if (deffov != NULL ) *deffov = file->moov.udta.navg.fieldofview;
        }
        else if (lqt_is_qtvr(file) == QTVR_PAN) {
            if (minfov != NULL ) *minfov = file->moov.trak[lqt_qtvr_get_panorama_track(file)]->mdia.minf.stbl.stsd.table->pano.MinZoom;
            if (maxfov != NULL ) *maxfov = file->moov.trak[lqt_qtvr_get_panorama_track(file)]->mdia.minf.stbl.stsd.table->pano.MaxZoom;
            if (deffov != NULL ) *deffov = file->moov.trak[lqt_qtvr_get_panorama_track(file)]->mdia.minf.stbl.stsd.table->pano.MinZoom;
        }
    }
}

void lqt_qtvr_set_fov(quicktime_t  *file, float minfov, float maxfov, float deffov)
{
    if (lqt_qtvr_get_object_track(file) >= 0) {
        file->qtvr_node[0].obji.minFOV = minfov;
        file->qtvr_node[0].obji.FOV = maxfov;
        file->qtvr_node[0].obji.defaultFOV = deffov;
    }
    else {
        file->moov.trak[lqt_qtvr_get_panorama_track(file)]->mdia.minf.stbl.stsd.table->pano.MinZoom = minfov;
        file->moov.trak[lqt_qtvr_get_panorama_track(file)]->mdia.minf.stbl.stsd.table->pano.MaxZoom = maxfov;
    }
}

/* frame to start at */
int lqt_qtvr_get_initial_position(quicktime_t  *file)
{
    if (lqt_qtvr_get_object_track(file) >= 0) {
        return deg2frame(file, file->qtvr_node[0].obji.defaultPan, file->qtvr_node[0].obji.defaultTilt);
    }
    else {
        return deg2frame(file, file->moov.udta.navg.initialHPan, file->moov.udta.navg.initialVPan);
    }

    return -1;
}

/* movietype */
int lqt_qtvr_get_movietype(quicktime_t  *file)
{
    if (lqt_qtvr_get_object_track(file) >= 0) {
        return file->qtvr_node[0].obji.movieType;
    }
    else {
        return file->moov.udta.navg.movietype;
    }
}

int lqt_qtvr_set_movietype(quicktime_t  *file, int movietype)
{
    if (movietype == QTVR_GRABBER_SCROLLER_UI || 
        movietype == QTVR_OLD_JOYSTICK_UI || 
        movietype == QTVR_JOYSTICK_UI || 
        movietype == QTVR_GRABBER_UI || 
        movietype == QTVR_ABSOLUTE_UI) {
        if (lqt_qtvr_get_object_track(file) >= 0) {
            file->qtvr_node[0].obji.movieType = movietype;
        }
        else {
            file->moov.udta.navg.movietype = movietype;
        }
        return 0;
    }

    return -1;
}

/* Set number of rows */
int lqt_qtvr_set_rows(quicktime_t  *file, int rows)
{
    if (rows > 0) {
        if (lqt_is_qtvr(file) == QTVR_OBJ) {
            file->qtvr_node[0].obji.rows = rows;
            return 0;
        }
        else if (lqt_is_qtvr(file) == QTVR_PAN) {
            file->qtvr_node[0].pdat.imageNumFramesX = rows;
            return 0;
        }
    }
    
    return -1;
}

/* Set number of columns */
int lqt_qtvr_set_columns(quicktime_t  *file, int columns)
{
    if (columns > 0) {
        if (lqt_is_qtvr(file) == QTVR_OBJ) {
            file->qtvr_node[0].obji.columns = columns;
            return 0;
        }
        else if (lqt_is_qtvr(file) == QTVR_PAN) {
            file->qtvr_node[0].pdat.imageNumFramesY = columns;
            return 0;
        }
    }
    
    return -1;
}

/* dimensions of the player window */
int lqt_qtvr_get_display_width(quicktime_t  *file)
{
    if (lqt_is_qtvr(file) == QTVR_OBJ) {
        return quicktime_video_width(file, 0);
    }
    else if (lqt_is_qtvr(file) == QTVR_PAN) {
        return (int)file->moov.trak[lqt_qtvr_get_panorama_track(file)]->tkhd.track_width;
    }

    return -1;
}

int lqt_qtvr_set_display_width(quicktime_t  *file, int width)
{
    if (lqt_is_qtvr(file) == QTVR_OBJ) {
        file->moov.trak[lqt_qtvr_get_qtvr_track(file)]->tkhd.track_width = (float)width;
        file->moov.trak[lqt_qtvr_get_object_track(file)]->tkhd.track_width = (float)width;
    }
    else if (lqt_is_qtvr(file) == QTVR_PAN) {
        file->moov.trak[lqt_qtvr_get_panorama_track(file)]->tkhd.track_width = (float)width;
    }

    return -1;
}

int lqt_qtvr_get_display_height(quicktime_t  *file)
{
    if (lqt_is_qtvr(file) == QTVR_OBJ) {
        return quicktime_video_height(file, 0);
    }
    else if (lqt_is_qtvr(file) == QTVR_PAN) {
        return (int)file->moov.trak[lqt_qtvr_get_panorama_track(file)]->tkhd.track_height;
    }

    return -1;
}


int lqt_qtvr_set_display_height(quicktime_t *file, int height)
{
    if (lqt_is_qtvr(file) == QTVR_OBJ) {
        file->moov.trak[lqt_qtvr_get_qtvr_track(file)]->tkhd.track_height = height;
        file->moov.trak[lqt_qtvr_get_object_track(file)]->tkhd.track_height = height;
    }
    else if (lqt_is_qtvr(file) == QTVR_PAN) {
        file->moov.trak[lqt_qtvr_get_panorama_track(file)]->tkhd.track_height = height;
    }

    return -1;
}

/* get depth */
int lqt_qtvr_get_depth(quicktime_t  *file)
{
    if (lqt_is_qtvr(file) == QTVR_OBJ) {
        return quicktime_video_depth(file, 0);
    }
    else if (lqt_is_qtvr(file) == QTVR_PAN) { // add v2
        return (int)file->moov.trak[lqt_qtvr_get_panorama_track(file)]->mdia.minf.stbl.stsd.table->pano.SDepth;
    }

    return -1;
}

/* get/set full pano dimensions */
int lqt_qtvr_get_width(quicktime_t  *file)
{
    if (lqt_is_qtvr(file) == QTVR_OBJ) {
        return quicktime_video_width(file, 0);
    }
    else
    if (lqt_is_qtvr(file) == QTVR_PAN) {
        if (lqt_qtvr_get_qtvr_track(file) >= 0) {
            if (lqt_qtvr_get_panotype(file) == QTVR_PANO_HORZ) { 
                return file->qtvr_node[0].pdat.imageSizeX;
            }
            else {
                return file->qtvr_node[0].pdat.imageSizeY;
            }
        }
        else {
            return file->moov.trak[lqt_qtvr_get_panorama_track(file)]->mdia.minf.stbl.stsd.table->pano.SWidth;
        }
    }

    return -1;
}

int lqt_qtvr_get_height(quicktime_t  *file)
{
    if (lqt_is_qtvr(file) == QTVR_OBJ) {
        return quicktime_video_height(file, 0);
    }
    else if (lqt_is_qtvr(file) == QTVR_PAN) {
        if (lqt_qtvr_get_qtvr_track(file) >= 0) {
            if (lqt_qtvr_get_panotype(file) == QTVR_PANO_HORZ) { 
                return file->qtvr_node[0].pdat.imageSizeY;
            }
            else {
                return file->qtvr_node[0].pdat.imageSizeX;
            }
        }
        else {
            return file->moov.trak[lqt_qtvr_get_panorama_track(file)]->mdia.minf.stbl.stsd.table->pano.SHeight;
        }
    }

    return -1;
}


/* Panorama specific */
int lqt_qtvr_get_panorama_track(quicktime_t  *file)
{
    int i;
    for(i = 0; i < file->moov.total_tracks; i++) {
        if (quicktime_match_32(file->moov.trak[i]->mdia.hdlr.component_subtype, "STpn")) {
            return i;
        }
        if (quicktime_match_32(file->moov.trak[i]->mdia.hdlr.component_subtype, "pano")) {
            return i;
        }
    }
    return -1;
}


int lqt_qtvr_get_panotype(quicktime_t  *file)
{
    if (lqt_is_qtvr(file) == QTVR_PAN) {
        if (lqt_qtvr_get_qtvr_track(file) >= 0 &&
            quicktime_match_32(file->qtvr_node[0].pdat.panoType, "hcyl")) {
            return QTVR_PANO_HORZ;
        }
        return QTVR_PANO_VERT;
    }

    return -1;
}


/* object specific */
int lqt_qtvr_get_object_track(quicktime_t  *file)
{
    int i;
    for(i = 0; i < file->moov.total_tracks; i++) {
        if (quicktime_match_32(file->moov.trak[i]->mdia.hdlr.component_subtype, "obje")) {
            return i;
        }
    }

    return -1;
}

/* get qtvr track */
int lqt_qtvr_get_qtvr_track(quicktime_t  *file)
{
    int i;
    for(i = 0; i < file->moov.total_tracks; i++) {
        if (quicktime_match_32(file->moov.trak[i]->mdia.hdlr.component_subtype, "qtvr")) {
            return i;
        }
    }

    return -1;
}

/* set/get image track */
int lqt_qtvr_set_image_track(quicktime_t  *file, int track)
{
    if (lqt_qtvr_get_object_track(file) >= 0) {
        int otrack = lqt_qtvr_get_object_track(file);
        if (otrack != -1 && file->total_vtracks > track) {	    
            file->moov.trak[otrack]->tref.references[0].tracks[0] =
                  file->vtracks[track].track->tkhd.track_id;
            return 0;
        }
    }
    else {
        int ptrack = lqt_qtvr_get_panorama_track(file);
        if (ptrack != -1 && file->total_vtracks > track) {
            /* reset the prevous vtracks flags if required */
            if (file->moov.trak[ptrack]->mdia.minf.stbl.stsd.table->pano.STrack != 0) 
                file->moov.trak[lqt_track_from_id(file, file->moov.trak[ptrack]->mdia.minf.stbl.stsd.table->pano.STrack)]->tkhd.flags = 15;

            file->moov.trak[ptrack]->mdia.minf.stbl.stsd.table->pano.STrack = file->vtracks[track].track->tkhd.track_id;
            file->vtracks[track].track->tkhd.flags = 14; /* deactivate this vtrack */
            return 0;
        }
    }

    return -1;
}

// This will not work correctly with movies that contain panos and obj movies in the same file
// possibly replace with _get_object_image_track() and _get_pano_image_track()
int lqt_qtvr_get_image_track(quicktime_t  *file)
{
    if (lqt_qtvr_get_qtvr_track(file) >= 0) {
        if (lqt_qtvr_get_object_track(file) != -1) {
            return file->moov.trak[lqt_qtvr_get_object_track(file)]->tref.references[0].tracks[0];
        } 
        else if (lqt_qtvr_get_panorama_track(file) != -1) {
            return file->qtvr_node[0].pdat.imageRefTrackIndex;
        }
    }
    else {
        if (lqt_qtvr_get_panorama_track(file) != -1) {
            return file->moov.trak[lqt_qtvr_get_panorama_track(file)]->mdia.minf.stbl.stsd.table->pano.STrack;
        }
    }

    return -1;
}

/* get track by track id */
int lqt_track_from_id(quicktime_t *file, int track_id)
{
    int i;
    for(i = 0; i < file->moov.total_tracks; i++) {
        if (file->moov.trak[i]->tkhd.track_id == track_id ) return i;
    }
    return -1;
}




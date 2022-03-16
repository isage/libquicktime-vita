/*******************************************************************************
 lqt.h

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

#ifndef _LQT_H_
#define _LQT_H_

#include "quicktime.h"
#include "lqt_atoms.h"
#include "compression.h"
#include "lqt_codecinfo.h"
#include "lqt_qtvr.h"

#pragma GCC visibility push(default)


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file lqt.h
 * Public api header for libquicktime.
 */
  
void *lqt_bufalloc(size_t size);


  
/** \ingroup log
 *  \brief Set callback for global logging
 *  \param cb Callback function
 *  \param data Application supplied data
 */

void lqt_set_log_callback(lqt_log_callback_t cb, void * data);
                           
  
/** \ingroup general
 *
 * \brief Return the raw filedescriptor associated with the file
 * \param file A quicktime handle
 * \returns The filesecriptor
 *
 * Use this of you want to call some low-level functions of the file.
 * Note, that this routine should be used with care, since it's easy
 * to screw things up.
 */
  
int lqt_fileno(quicktime_t *file);

/** \ingroup audio
 *  \brief Set a codec parameter for an audio track
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param key Short name of the parameter
 *  \param value Parameter value.
 *
 *  For integer parameters, value must be of the type int*. For string parameters,
 *  use char*.  For floating-point parameters, use float*.
 */
  
void lqt_set_audio_parameter(quicktime_t *file,int track, const char *key,const void *value);

/** \ingroup video
 *  \brief Set a codec parameter for a video track
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param key Short name of the parameter
 *  \param value Parameter value.
 *
 *  For integer parameters, value must be of the type int*. For string parameters,
 *  use char*.  For floating-point parameters, use float*.
 */

void lqt_set_video_parameter(quicktime_t *file,int track, const char *key,const void *value);

/** \ingroup video_decode
 *  \brief Get the pixel aspect ratio of a video track
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param pixel_width returns the pixel width
 *  \param pixel_height returns the pixel height
 *  \returns 1 if the call was successful, 0 if there is no such track
 */


int lqt_get_pixel_aspect(quicktime_t *file, int track, int * pixel_width,
                         int * pixel_height);

/** \ingroup video_encode
 *  \brief Set the pixel aspect ratio of a video track
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param pixel_width Pixel width
 *  \param pixel_height Pixel height
 *  \returns 1 if the call was successful, 0 if there is no such track
 *
 */

int lqt_set_pixel_aspect(quicktime_t *file, int track, int pixel_width,
                         int pixel_height);

/** \ingroup video_decode
    \brief Get the interlace mode
    \param file A quicktime handle
    \param track Track index (starting with 0)
    \returns The interlace mode.

    The interlace mode is stored in the fiel atom, which is used
    by default. If there is no fiel atom and the interlace mode is
    not implied by
    the codec, \ref LQT_INTERLACE_NONE is returned, which might be wrong.
*/

lqt_interlace_mode_t lqt_get_interlace_mode(quicktime_t * file, int track);

/** \ingroup video_encode
 *  \brief Set the interlace mode
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param mode The interlace mode.
 *  \returns 1 if the call was successful, 0 if there is no such track
 *
 *  This will in most cases store the correct interlacing information
 *  in the file (e.g. in the fiel atom). For tweaking the fiel atom
 *  directly, advanced users might look at \ref lqt_set_fiel.
 */

int lqt_set_interlace_mode(quicktime_t * file, int track,
                           lqt_interlace_mode_t mode);

  
/** \ingroup video
    \brief Convert an interlace mode to a human readable string
    \param mode An interlace mode
    \returns A description of the interlace mode
*/
const char * lqt_interlace_mode_to_string(lqt_interlace_mode_t mode);

/** \ingroup video_decode
    \brief Get the chroma placement
    \param file A quicktime handle
    \param track Track index (starting with 0)
    \returns The chroma placement

    The chroma placement is implied by the codec and makes only sense
    for YUV420 formats.
*/
  
lqt_chroma_placement_t lqt_get_chroma_placement(quicktime_t * file, int track);

/** \ingroup video
    \brief Convert a chroma placement to a human readable string
    \param chroma_placement A chroma placement
    \returns A description of the chroma placement
*/

const char * lqt_chroma_placement_to_string(lqt_chroma_placement_t chroma_placement);

/** \ingroup general
    \brief Get the codec API version.
    \returns The codec API version, libquicktime was compiled with

    Normally you don't need this function. It is used internally to
    detect codec modules, which were compiled with an incompatible
    libquicktime version.
    
*/
  

/** \ingroup multichannel
 *  \brief Convert a channel identifier to a human readnable string
 *  \param ch A channel
 *  \returns A string describing the channel.
 */
  
const char * lqt_channel_to_string(lqt_channel_t ch);

/** \ingroup multichannel
 *  \brief Set a channel setup for an audio track
 *  \param file A quicktime handle
 *  \param track index (starting with 0)
 *  \param ch Channel array
 *
 *  Set the desired channel setup for a file. Note, that libquicktime
 *  may reorder the channels. It is therefore necessary to call
 *  \ref lqt_get_channel_setup after to know the final truth.
 */

void lqt_set_channel_setup(quicktime_t * file, int track, lqt_channel_t * ch);

/** \ingroup multichannel
 *  \brief Get a channel setup from a file
 *  \param file A quicktime handle
 *  \param track index (starting with 0)
 *  \returns The channel array
 *
 *  Get the channel setup from a quicktime file. Works for en- and decoding.
 *  The return value is owned by libquicktime and must not be freed.
 */
  
const lqt_channel_t * lqt_get_channel_setup(quicktime_t * file, int track);


/** \defgroup text Text
    \brief Text related definitions and functions

    Libquicktime supports reading and writing of text tracks from/to
    Quicktime and mp4 files. Text tracks contain samples, which
    are simple text strings. Libquicktime tries to handle character set issues
    internally, all string you pass to/from libquicktime can assumed to be UTF-8.

    A text track can either be used for subtitles (which is the default), or for
    chapters. In the latter case, each text sample denotes the title of the chapter,
    the corresponding timestamp is the start time of the chapter.

    For subtitles, there is only a start time defined for each sample, not the duration,
    after which the subtitle will be hidden. To make subtitles disappear at a specified
    time, insert an empty subtitle with the right timestamp.

    What's not supported are text attributes, font selection etc. Feel free to make a
    proposal how to support these things.
*/
  
/** \defgroup text_encode Writing text
    \ingroup text
    \brief Encode text

    @{
*/

/** \brief Add a text track
 *  \param file A quicktime handle
 *  \param timescale The timescale, in which timestamps will be given.
 */
  
int lqt_add_text_track(quicktime_t * file, int timescale);

/** \brief Set the language for a text track
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param language ISO-639 Language code
 *
 *  The language code is a 3-character code, English is "eng",
 *  Japanese is "jpn".
 */

void lqt_set_text_language(quicktime_t * file, int track, const char * language);

/** \brief Make a text track a chapter track
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *
 *  By default, text tracks are subtitles. By calling this function,
 *  you tell libquicktime, that the text track should be a chapter track.
 */


  
void lqt_set_chapter_track(quicktime_t * file, int track);

/** \brief Set the text box of a text track
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param top Top border
 *  \param left Left border
 *  \param bottom Bottom border
 *  \param right Right border
 */
  
void lqt_set_text_box(quicktime_t * file, int track,
                      uint16_t top, uint16_t left,
                      uint16_t bottom, uint16_t right);

/** \brief Set the foreground color of a text track
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param r Red
 *  \param g Green
 *  \param b Blue
 *  \param a Alpha
 *
 *  Color values are between 0x0000 and 0xffff. The alpha value
 *  is only used for mp4.
 */
  
void lqt_set_text_fg_color(quicktime_t * file, int track,
                           uint16_t r, uint16_t g,
                           uint16_t b, uint16_t a);

/** \brief Set the background color of a text track
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param r Red
 *  \param g Green
 *  \param b Blue
 *  \param a Alpha
 *
 *  Color values are between 0x0000 and 0xffff. The alpha value
 *  is only used for mp4. For Quicktime, the text box is set to transparent
 *  if the alpha value is below 0x8000.
 */
  
void lqt_set_text_bg_color(quicktime_t * file, int track,
                           uint16_t r, uint16_t g,
                           uint16_t b, uint16_t a);

  
  
  
/** \brief Write a text sample
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param text A null-terminated UTF-8 string
 *  \param duration The duration associated with this sample
 *  \returns 0 if a the text sample could be written to the file, nonzero else
 */
  
int lqt_write_text(quicktime_t * file, int track, const char * text, int64_t duration);
  
/**
   @}
*/
  
/** \defgroup text_decode Reading text
    \ingroup text
    \brief Decode text

    @{

*/

/** \brief Get the number of text tracks
 *  \param file A quicktime handle
 *  \returns The number of text tracks found in the file
 */
  
int lqt_text_tracks(quicktime_t * file);

/** \brief Get the text language
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param language Returns ISO-639 Language code
 *  \returns 1 on success, 0 on error.
 *
 *  The language code is a 3-character code, English is "eng",
 *  Japanese is "jpn".
 */

int lqt_get_text_language(quicktime_t * file, int track, char * language);

/** \brief Get the timescale for a text track
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \returns The timescale of the track.
 */

int lqt_text_time_scale(quicktime_t * file, int track);

/** \brief Set the text box of a text track
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param top Returns the top border
 *  \param left Returns the left border
 *  \param bottom Returns the bottom border
 *  \param right Returns the right border
 */
  
void lqt_get_text_box(quicktime_t * file, int track,
                      uint16_t * top, uint16_t * left,
                      uint16_t * bottom, uint16_t * right);

  
/** \brief Read a text sample
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param text Address of a buffer
 *  \param text_alloc Allocated bytes for this buffer (will be changed)
 *  \param timestamp Returns the timestamp of the sample
 *  \param duration Returns the duration  of the sample
 *  \returns 1 if a sample was decoded, 0 if the track is finished
 *
 *  This funtion calls realloc() to make sure there is enough space for
 *  for the text. It's a good idea to always pass the same buffer to this
 *  function (set to NULL initially) and free it after the file is closed.
 */
  
int lqt_read_text(quicktime_t * file, int track, char ** text, int * text_alloc,
                  int64_t * timestamp, int64_t * duration);

/** \brief Check if a track is a chapter track
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \returns 1 if the text track is a chapter track, 0 else
 */
     
  
int lqt_is_chapter_track(quicktime_t * file, int track);

/** \brief Get the total number of text samples
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \returns The number of samples
 */

int64_t lqt_text_samples(quicktime_t * file, int track);

/** \brief Go to a specific sample
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param position The sample position (starting with 0)
 */

void lqt_set_text_position(quicktime_t * file, int track, int64_t position);

/** \brief Go to a specific time
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param time Time
 *
 *  This wil reposition the text track such, that the next call
 *  to \ref lqt_read_text will return a sample with at least the
 *  timestamp you specified.
 */

void lqt_set_text_time(quicktime_t * file, int track, int64_t time);

/** \brief Get the foreground color of a text track
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param r Returns red
 *  \param g Returns green
 *  \param b Returns blue
 *  \param a Returns alpha
 *
 *  Color values are between 0x0000 and 0xffff.
 */
  
void lqt_get_text_fg_color(quicktime_t * file, int track,
                           uint16_t * r, uint16_t * g,
                           uint16_t * b, uint16_t * a);

/** \brief Get the background color of a text track
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param r Returns red
 *  \param g Returns green
 *  \param b Returns blue
 *  \param a Returns alpha
 *
 *  Color values are between 0x0000 and 0xffff.
 */
  
void lqt_get_text_bg_color(quicktime_t * file, int track,
                           uint16_t * r, uint16_t * g,
                           uint16_t * b, uint16_t * a);

/**
   @}
*/

/** \defgroup timecodes Timecodes
    \brief Timecode support

    Timecodes are passed to/from libquicktime in the same format as they are stored in the
    file: 32 bit unsigned integers. For the meaning of the bits, see the section "Timecode Sample Data"
    in the Quicktime file format specification.
    
    @{

*/
  
#define LQT_TIMECODE_DROP    0x0001 //!< Indicates whether the timecode is drop frame
#define LQT_TIMECODE_24HMAX  0x0002 //!< Indicates whether the timecode wraps after 24 hours
#define LQT_TIMECODE_NEG_OK  0x0004 //!< Indicates whether negative time values are allowed
#define LQT_TIMECODE_COUNTER 0x0008 //!< Indicates whether the time value corresponds to a tape counter value

/** \brief Attach a timecode track to a video track
 *  \param file A quicktime handle
 *  \param track Video track index (starting with 0)
 *  \param flags Zero or more of the LQT_TIMECODE_* flags
 *  \param framerate The integer framerate
 *
 *  If the format (e.g. AVI) doesn't support timecode, this
 *  function emits a warning.
 *
 *  Since 1.1.0
 */

void lqt_add_timecode_track(quicktime_t * file, int track,
                            uint32_t flags, int framerate);

/** \brief Write a timecode for the next video frame to be encoded
 *  \param file A quicktime handle
 *  \param track Video track index (starting with 0)
 *
 *  Call this function @b before encoding the actual video frame.
 *
 *  Since 1.1.0
 */
  
void lqt_write_timecode(quicktime_t * file, int track,
                        uint32_t timecode);

/** \brief Check, if a video track has timecodes
 *  \param file A quicktime handle
 *  \param track Video track index (starting with 0)
 *  \param flags If non NULL returns zero or more of the LQT_TIMECODE_* flags
 *  \param framerate If non NULL returns the integer framerate
 *  \returns 1 if timecodes are available, 0 else
 *
 *  Since 1.1.0
 */
  
int lqt_has_timecode_track(quicktime_t * file, int track,
                           uint32_t * flags, int * framerate);

/** \brief Read the timecode for the next frame to be decoded
 *  \param file A quicktime handle
 *  \param track Video track index (starting with 0)
 *  \param timecode Returns the timecode if available
 *  \returns 1 if a timecode is available for this frame, 0 else
 *
 *  For frames, which have no timecode attached, you can increment the
 *  last timecode accordingly in your application.
 *  Call this function @b before decoding the actual video frame.
 *
 *  Since 1.1.0
 */

  
int lqt_read_timecode(quicktime_t * file, int track,
                      uint32_t * timecode);

/** \brief Get the tape name stored in a timecode track
 *  \param file A quicktime handle
 *  \param track Video track index (starting with 0)
 *  \returns pointer to tape name (might be NULL if no tape name)
 *
 *  Returned pointer should remain valid as long as the file is open.
 *  Do not free it.
 *
 *  Since 1.1.0
 */
  
const char * lqt_get_timecode_tape_name(quicktime_t * file, int track);

/** \brief Set the tapename for a timecode track
 *  \param file A quicktime handle
 *  \param track Video track index (starting with 0)
 *  \param tapename Tape name string
 *
 *  A copy is made of the string passed in
 *
 *  Since 1.1.0
 */
    
void lqt_set_timecode_tape_name(quicktime_t * file, int track,
				const char * tapename);

/** \brief Get the enabled flag of a timecode track
 *  \param file A quicktime handle
 *  \param track Video track index (starting with 0)
 *  \returns 1 if enabled, 0 if not enabled
 *
 *  Since 1.1.0
 */

int lqt_get_timecode_track_enabled(quicktime_t * file, int track);

/** \brief Enable or disable a timecode track
 *  \param file A quicktime handle
 *  \param track Video track index (starting with 0)
 *  \param enabled 0=>disable otherwise enable
 *
 *  Since 1.1.0
 */

void lqt_set_timecode_track_enabled(quicktime_t * file, int track,
				    int enabled);

/**
   @}
*/

  
  
/***********************************************
 * Advanced colormodel handling.
 * (defined in lqt_color.c)
 ***********************************************/

/** \ingroup color
 *
 *  This value is used for termination of colormodel arrays
 */
  
#define LQT_COLORMODEL_NONE -1
  
/* Colormodel <-> string conversion (used by registry file routines) */

/** \ingroup color
    \brief Convert a colormodel to a human readable string
    \param colormodel A colormodel
    \returns A description of the colormodel
*/
  
const char * lqt_colormodel_to_string(int colormodel);

/** \ingroup color
    \brief Convert a description string to a colormodel
    \param str A colormodel description (as returned by \ref lqt_colormodel_to_string)
    \returns The corresponding colormodel or \ref LQT_COLORMODEL_NONE
*/
  
int lqt_string_to_colormodel(const char * str);

/* Query information about the colormodel */

/** \ingroup color
   \brief Check if a colormodel is planar
   \param colormodel A colormodel
   \returns 1 if the colormodel is planar, 0 else
*/

int lqt_colormodel_is_planar(int colormodel);

  /** \ingroup color
   \brief Check if a colormodel has an alpha (transperency) channel
   \param colormodel A colormodel
   \returns 1 if the colormodel has an alpha channel, 0 else
*/

int lqt_colormodel_has_alpha(int colormodel);

/** \ingroup color
   \brief Check, if a colormodel is RGB based
   \param colormodel A colormodel
   \returns 1 if the colormodel is RGB based, 0 else
*/
  
int lqt_colormodel_is_rgb(int colormodel);

/** \ingroup color
   \brief Check, if a colormodel is YUV based
   \param colormodel A colormodel
   \returns 1 if the colormodel is YUV based, 0 else
*/

int lqt_colormodel_is_yuv(int colormodel);

/** \ingroup color
   \brief Get the chroma subsampling factors
   \param colormodel A colormodel
   \param sub_h Returns the horizontal subsampling factor
   \param sub_v Returns the vertical subsampling factor
   
*/

void lqt_colormodel_get_chroma_sub(int colormodel, int * sub_h, int * sub_v);

/** \ingroup color
   \brief Get the default row span for a colormodel and an image width
   \param colormodel A colormodel
   \param width Image width
   \param rowspan Returns the rowspan for the luminance (Y) plane 
   \param rowspan_uv Returns the rowspan for the chrominance (U/V) planes 
   
   The rowspan is the byte offset between scanlines. It can be calculated
   from the colormodel and the image width. Some APIs however, padd the scanlines to
   certain boundaries, so the rowspans might become larger here (see \ref lqt_set_row_span and
   \ref lqt_set_row_span_uv).
*/

void lqt_get_default_rowspan(int colormodel, int width, int * rowspan, int * rowspan_uv);

/** \ingroup color
 * \brief Check if a colormodel conversion is supported by libquicktime
 * \param in_cmodel Input colormodel
 * \param out_cmodel Output colormodel
 * \returns 1 if the requested conversion is possible, 0 else
 *
 * As noted before, the colormodel converter is not complete, and this function
 * checks it. As a fallback, conversions from and to \ref BC_RGB888 are always supported.
 * If you need a converison, which is not present, contact the authors for hints how to
 * write it :)
 */
 
int lqt_colormodel_has_conversion(int in_cmodel, int out_cmodel);
  
/* Query supported colormodels */

/** \ingroup color
    \brief Get number of supported colormodels
    \returns The number of colormodels known to your version of libquicktime
*/
  
int lqt_num_colormodels();

/** \ingroup color
    \brief Get a colormodel string
    \param index Index of the colormodel (between 0 and the return value of \ref lqt_num_colormodels - 1)
    \returns A description of the colormodel according to index or NULL.
*/  
const char * lqt_get_colormodel_string(int index);

/** \ingroup color
    \brief Get a colormodel
    \param index Index of the colormodel (between 0 and the return value of \ref lqt_num_colormodels - 1)
    \returns The colormodel according to index or \ref LQT_COLORMODEL_NONE
*/  
  
int lqt_get_colormodel(int index);

/** \ingroup video_decode
 *  \brief Get the native colormodel of the decoder
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *
 *  This returns the colormodel, which is used by the stream natively. If your
 *  application can handle the colormodel, you can use \ref lqt_decode_video for
 *  decoding in the native colormodel. This will bypass all internal colormodel conversions.
 */

int lqt_get_decoder_colormodel(quicktime_t * file, int track);

/** \ingroup color
 *  \brief Select a source colormodel from a provided list that is best
 *         for conversion into a specified target colormodel.
 *  \param source_options An array of source colormodels to select from.
 *         Must be terminated with LQT_COLORMODEL_NODE.
 *  \param target The target colormodel.
 *  \return A colormodel from \p source_options or LQT_COLORMODEL_NONE,
 *          if none of the source colormodels can be converted to the
 *          target one.
 */

int lqt_get_best_source_colormodel(int const* source_options, int target);

/** \ingroup color
 *  \brief Select a target colormodel from a provided list that is best
 *         for conversion from a specified source colormodel.
 *  \param source The source colormodel.
 *  \param target_options An array of target colormodels to select from.
 *         Must be terminated with LQT_COLORMODEL_NODE.
 *  \return A colormodel from \p target_options or LQT_COLORMODEL_NONE,
 *          if the source colormodel can't be converted to any of the
 *          target ones.
 */

int lqt_get_best_target_colormodel(int source, int const* target_options);

/** \ingroup video
 *  \brief Get the best colormodel out of a list of supported colormodels
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param supported An array with supported colormodels.
 *  \returns The best colormodel
 *
 *  This is a convenience function for application developers:
 *  It takes an array with supported colormodels (Terminated with LQT_COLORMODEL_NONE)
 *  and returns the best colormodel. The decision is done according to the conversion
 *  overhead. i.e. you'll get the colormodel of your list, which is "closest" to the
 *  colormodel, the codec delivers. To make sure, that this function never fails, you
 *  should at least support \ref BC_RGB888 .
 *  This function works for en- and decoding.
 */

int lqt_get_best_colormodel(quicktime_t * file, int track, int * supported);

/** \ingroup video
 *  \brief Get the colormodel, which will be valid for the next en-/decode call.
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \returns The colormodel used for en-/decode functions.
 *
 *  By default, it will return the colormodel, which is used natively
 *  by the codec. It can be changed with \ref lqt_set_cmodel .
 */
  
int lqt_get_cmodel(quicktime_t * file, int track);
  
/** \ingroup video
 *  \brief Allocate a frame buffer for use with libquicktime
 *  \param width The width of the frame
 *  \param height The height of the frame
 *  \param colormodel The colormodel of the frame (see \ref color).
 *  \param rowspan Specifies the row span to use for the luma plane. Can be 0 to use default values. After the function call, it will contain the rowspan actually used.
 *  \param rowspan_uv Specifies the row span to use for the chroma planes. Can be 0 to use default values. After the function call, it will contain the rowspan actually used.
 *  \returns An array of pointers to be passed to any of the en-/decoding functions.
 *
 *  What is called "row_pointers" here is a bit misleading: For packed formats,
 *  the pointers point to the beginnings of scanlines. For planar formats, the pointers
 *  point to the beginning of planes. In either case, the byte offsets between scanlines are
 *  be specified by rowspan and rowspan_uv. To free the returned frame, call \ref lqt_rows_free
 */
  
uint8_t ** lqt_rows_alloc(int width, int height, int colormodel, int * rowspan, int * rowspan_uv);

/** \ingroup video
 * \brief Copy a video frame.
 * \param out_rows Destination frame
 * \param in_rows Source frame
 * \param width Width of the frame
 * \param height Height of the frame
 * \param in_rowspan Rowspan for the luma plane of the input frame
 * \param in_rowspan_uv Rowspan for the chroma planes of the input frame
 * \param out_rowspan Rowspan for the luma plane of the output frame
 * \param out_rowspan_uv Rowspan for the chroma planes of the output frame
 * \param colormodel The colormodel of the frames
 */
   
void lqt_rows_copy(uint8_t **out_rows, uint8_t **in_rows, int width, int height, int in_rowspan, int in_rowspan_uv,
                   int out_rowspan, int out_rowspan_uv, int colormodel);
  
/** \ingroup video
 * \brief Copy a subrectangle video frame.
 * \param out_rows Destination frame
 * \param in_rows Source frame
 * \param width Width of the frame
 * \param height Height of the frame
 * \param in_rowspan Rowspan for the luma plane of the input frame
 * \param in_rowspan_uv Rowspan for the chroma planes of the input frame
 * \param out_rowspan Rowspan for the luma plane of the output frame
 * \param out_rowspan_uv Rowspan for the chroma planes of the output frame
 * \param colormodel The colormodel of the frames
 * \param src_x X offset in the source frame
 * \param src_y Y offset in the source frame
 * \param dst_x X offset in the destination frame
 * \param dst_y Y offset in the destination frame
 *
 * Since 1.2.0
 */

void lqt_rows_copy_sub(uint8_t **out_rows, uint8_t **in_rows,
                       int width, int height, int in_rowspan,
                       int in_rowspan_uv, int out_rowspan,
                       int out_rowspan_uv, int colormodel, int src_x, int src_y, int dst_x, int dst_y);
  
/** \ingroup video
 * \brief Clear a video frame.
 * \param rows Frame
 * \param width Width of the frame
 * \param height Height of the frame
 * \param rowspan Rowspan for the luma plane of theframe
 * \param rowspan_uv Rowspan for the chroma planes of the frame
 * \param colormodel The colormodel of the frame
 *
 * This will set the colors to black and alpha (if available) to
 * completely transparent.
 *
 * Since 1.2.0
 * 
 */

void lqt_rows_clear(uint8_t **rows,
                    int width, int height, int rowspan, int rowspan_uv, int colormodel);

  
/** \ingroup video
 *  \brief Free a frame allocated by \ref lqt_rows_alloc
 *  \param rows The frame to be freed
 */
  
void lqt_rows_free(uint8_t ** rows);
  

/**************************************
 * Set streams for encoding
 **************************************/

/** \ingroup audio_encode
 *  \brief Set up audio tracks for encoding
 *  \param file A quicktime handle
 *  \param channels Number of channels
 *  \param sample_rate Samplerate
 *  \param bits Bits per sample (always 16)
 *  \param codec_info Codec to use (see \ref codec_registry )
 *
 *  This sets one audio track for encoding. Note that the bits argument
 *  should always be 16 since it's implicit to the codec in all cases.
 *  To add more than one audio track, use \ref lqt_add_audio_track .
 */
 
int lqt_set_audio(quicktime_t *file, int channels,
                  long sample_rate,  int bits,
                  lqt_codec_info_t * codec_info);

  
/** \ingroup video_encode
 *  \brief Set up video tracks for encoding
 *  \param file A quicktime handle
 *  \param tracks Number of video tracks
 *  \param frame_w Image width
 *  \param frame_h Image height
 *  \param frame_duration Duration of one frame. This can later be overridden
 *  \param timescale Timescale of the track
 *  \param codec_info Codec to use (see \ref codec_registry )
 *
 *  This sets one or more video tracks for encoding. The framerate is
 *  passed as a rational number (timescale/frame_duration). E.g. for an NTSC
 *  stream, you'll choose timescale = 30000 and frame_duration = 1001.
 *  To set up multiple video tracks with different formats and/or codecs,
 *  use \ref lqt_add_video_track .
 */

int lqt_set_video(quicktime_t *file, int tracks, 
                  int frame_w, int frame_h,
                  int frame_duration, int timescale,
                  lqt_codec_info_t * codec_info);

 
/** \ingroup audio_encode
 *  \brief Add an audio track for encoding
 *  \param file A quicktime handle
 *  \param channels Number of channels
 *  \param sample_rate Samplerate
 *  \param bits Bits per sample (always 16)
 *  \param codec_info Codec to use (see \ref codec_registry) or NULL
 *
 *  This sets adds a new audio track for encoding. Note that the bits argument
 *  should always be 16 since it's implicit to the codec in all cases.
 *  Call this function to subsequently to add as many tracks as you like.
 *
 *  If you passed NULL for the codec_info, you should call \ref lqt_set_audio_codec
 *  once you know which codec to use.
 */

int lqt_add_audio_track(quicktime_t *file,
                        int channels, long sample_rate, int bits,
                        lqt_codec_info_t * codec_info);

/** \ingroup audio_encode
 *  \brief Set a codec for an audio track
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param info The codec to be used for encoding
 */
  
int lqt_set_audio_codec(quicktime_t *file, int track,
                        lqt_codec_info_t * info);

  
/** \ingroup audio_encode
 *  \brief Set the audio language
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param language ISO-639 Language code
 *
 *  The language code is a 3-character code, English is "eng",
 *  Japanese is "jpn".
 */

void lqt_set_audio_language(quicktime_t * file, int track, const char * language);
  
/** \ingroup video_encode
 *  \brief Add a video track for encoding
 *  \param file A quicktime handle
 *  \param frame_w Image width
 *  \param frame_h Image height
 *  \param frame_duration Duration of one frame. This can later be overridden
 *  \param timescale Timescale of the track
 *  \param codec_info Codec to use (see \ref codec_registry ) or NULL
 *
 *  This sets one or more video tracks for encoding. The framerate is
 *  passed as a rational number (timescale/frame_duration). E.g. for an NTSC
 *  stream, you'll choose timescale = 30000 and frame_duration = 1001.
 *  Call this function to subsequently to add as many tracks as you like.
 *  
 *  If you passed NULL for the codec_info, you should call \ref lqt_set_video_codec
 *  once you know which codec to use.
 */
  
int lqt_add_video_track(quicktime_t *file,
                        int frame_w, int frame_h,
                        int frame_duration, int timescale,
                        lqt_codec_info_t * codec_info);

/** \ingroup video_encode
 *  \brief Set a codec for an audio track
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param info The codec to be used for encoding
 */
  
int lqt_set_video_codec(quicktime_t *file, int track,
                        lqt_codec_info_t * info);

  
/** \ingroup video_encode
 *  \brief Enable multipass encoding
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param pass The number of this pass (starting with 1)
 *  \param total_passes The total number of passes
 *  \param stats_file Statistics file
 *
 *  This is a purely optional function, which enables multipass encoding.
 *  Multipass encoding is done by repeatedly opening a quicktime file,
 *  encoding video and and closing it again. The stats_file parameter must
 *  always be the same for all passes.
 *
 *  Having more than 2 passes is not always useful. Audio encoding can be
 *  skipped for all passes until the last one.
 */

int lqt_set_video_pass(quicktime_t *file,
                       int pass, int total_passes, 
                       const char * stats_file, int track);

/** \ingroup video_decode
 *  \brief Get the timestamp of a given frame
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param frame The frame
 *  \returns The timestamp of the frame
 */
int64_t lqt_get_frame_time(quicktime_t * file, int track, int frame);

/** \ingroup video_decode
 *  \brief Get the timestamp of the next frame to be decoded
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \returns The timestamp of the next frame to be decoded
 *
 *  Call this BEFORE one of the decoding functions to get the
 *  timestamp of the next frame
 */
  
int64_t lqt_frame_time(quicktime_t * file, int track);

/** \ingroup video_decode
 *  \brief Decode one video frame
 *  \param file A quicktime handle
 *  \param row_pointers Frame (see \ref lqt_rows_alloc)
 *  \param track Track index (starting with 0)
 *
 * Decode one video frame and increment the interal frame pointer.
 * To get the presentation timestamp for this frame, call
 * \ref lqt_frame_time before.
 */
  

int lqt_decode_video(quicktime_t *file,
                     unsigned char **row_pointers, int track);

/** \ingroup video_decode
 *  \brief Read a compressed video frame
 *  \param file A quicktime handle
 *  \param buffer Buffer where the frame will be read to
 *  \param buffer_alloc Number of bytes allocated for the buffer
 *  \param frame Number of the frame to be read (starting with 0)
 *  \param time If non NULL, returns the timestamp in timescale tics
 *  \param track Track index (starting with 0)
 *  \returns The number of bytes in the frame or 0 if no frame could be read.
 *
 * Read one compressed video frame. This function calls realloc()
 * to ensure, that the buffer will be large enough. To use this
 * function, set buffer to NULL and buffer_alloc to zero before
 * the first call. After the last call, free the buffer with free().
 * This function is mainly used by video codecs.
 */

int lqt_read_video_frame(quicktime_t * file,
                         uint8_t ** buffer, int * buffer_alloc,
                         int64_t frame, int64_t * time, int track);
  
/** \ingroup video_encode
 *  \brief Encode one video frame
 *  \param file A quicktime handle
 *  \param row_pointers Frame (see \ref lqt_rows_alloc)
 *  \param track Track index (starting with 0)
 *  \param time Timestamp of the frame in timescale tics
 *  \returns 0 if the frame was encoded, 1 else.
 *
 * Encode one video frame. The presentation timestamp is in
 * timescale tics with the timescale you passed to
 * \ref lqt_add_video_track or \ref lqt_set_video . WARNING: AVI files
 * don't support arbitrary timestamps. For AVI files
 *  time is ignored, instead it's frame_number * frame_duration,
 */
  
int lqt_encode_video(quicktime_t *file, 
                     unsigned char **row_pointers, 
                     int track, int64_t time);

/** \ingroup video_encode
 *  \brief Encode one video frame
 *  \param file A quicktime handle
 *  \param row_pointers Frame (see \ref lqt_rows_alloc)
 *  \param track Track index (starting with 0)
 *  \param time Timestamp of the frame in timescale tics
 *  \param duration Duration of the frame
 *  \returns 0 if the frame was encoded, 1 else.
 *
 *  This is the same as \ref lqt_encode_video except that you can
 *  pass the duration along with the timestamp. This is really only
 *  important for the last picture, since all other durations are
 *  calculated from the timestamp differences.
 *
 *  Since 1.1.2
 */
  
int lqt_encode_video_d(quicktime_t *file, 
                       unsigned char **row_pointers, 
                       int track, int64_t time, int duration);
  
/** \ingroup video_decode
 *  \brief Get the duration of the NEXT frame to be decoded
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param constant If non NULL it will be set to 1 if the frame duration is constant throughout the whole track
 *  \returns The frame duration in timescale tics
 */

int lqt_frame_duration(quicktime_t * file, int track, int *constant);
  
/** \ingroup video_decode
 *  \brief Get the timescale of the track
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \returns The timescale of the track
 */
  
int lqt_video_time_scale(quicktime_t * file, int track);

/** \ingroup video_decode
 *  \brief Get the duration of a video track
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \returns The total duration of the track in timescale tics
 *
 *  Use this function if you want to support nonconstant framerates.
 */

int64_t lqt_video_duration(quicktime_t * file, int track);

/** \ingroup video
 * \brief Set the colormodel for en-/decoding
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param colormodel The colormodel to use.
 *
 *  Set colormodel of a video track. It's the colormodel, libquicktime
 *  will expect for the next call to \ref lqt_encode_video or
 *  \ref lqt_decode_video respectively. Before you should call this,
 *  you should verify, that this colormodel can be used with
 *  \ref quicktime_reads_cmodel (for reading), \ref quicktime_writes_cmodel
 *  (for writing) or \ref lqt_get_best_colormodel (for reading and writing).
 */

void lqt_set_cmodel(quicktime_t *file, int track, int colormodel);

/** \ingroup video_decode
 * \brief Get the number of video track edit segments
 *  \param file A quicktime handle
 *  \param track Video track index (starting with 0)
 */

long lqt_video_edit_list_total_entries(quicktime_t * file, int track);

/** \ingroup video_decode
 * \brief Get the duration of a video track edit segment
 *  \param file A quicktime handle
 *  \param track Video track index (starting with 0)
 *  \param entry_index Index into the edit segments
 */

long lqt_video_edit_duration(quicktime_t * file, int track, int entry_index);

/** \ingroup video_decode
 * \brief Get the time offset of a video track edit segment
 *  \param file A quicktime handle
 *  \param track Video track index (starting with 0)
 *  \param entry_index Index into the edit segments
 */

long lqt_video_edit_time(quicktime_t * file, int track, int entry_index);

/** \ingroup video_decode
 * \brief Get the rate of a video track edit segment
 *  \param file A quicktime handle
 *  \param track Video track index (starting with 0)
 *  \param entry_index Index into the edit segments
 */

float lqt_video_edit_rate(quicktime_t * file, int track, int entry_index);

/** \ingroup video
 * \brief Set the row span for the luma plane
 * \param file A quicktime handle
 * \param track Track index (starting with 0)
 * \param row_span The row span for the luma plane
 *
 * This sets the row_span, which will be used for the next en-/decode
 * calls (see \ref lqt_rows_alloc ).
 */

void lqt_set_row_span(quicktime_t *file, int track, int row_span);

/** \ingroup video
 * \brief Set the row span for the chroma planes
 * \param file A quicktime handle
 * \param track Track index (starting with 0)
 * \param row_span_uv The row span for the chroma planes
 *
 * This sets the row_span, which will be used for the next en-/decode
 * calls (see \ref lqt_rows_alloc ).
 */

void lqt_set_row_span_uv(quicktime_t *file, int track, int row_span_uv);
  
/** \ingroup audio_decode
 * \brief Decode all channels from all tracks at once
 * \param file A quicktime handle
 * \param output_i 16 bit integer output buffer (or NULL)
 * \param output_f floating point output buffer (or NULL)
 * \param samples How many samples to decode
 *
 * quicktime4linux hides the concept of multiple audio tracks from the user.
 * The idea was probably to put an arbitrary number of channels into a file
 * regardless of the codec (some codecs don't support more than 2 channels).
 * So in principle, this function does the same as \ref quicktime_decode_audio,
 * but it grabs all channels at once. Or if you want only some channels you can
 * leave the channels you don't want = NULL in the output array. The output
 * arrays must contain at least lqt_total_channels(file) elements.
 *
 * Libquicktime supports an arbitrary number of audio tracks, which can even have
 * completely different formats and codecs. Therefore you should always use
 * \ref lqt_decode_audio_track or \ref lqt_decode_audio_raw to decode audio from a
 * particular track.
 *
 * The number of actually decoded samples (and EOF) can be obtained with
 * \ref lqt_last_audio_position
 */

int lqt_decode_audio(quicktime_t *file, 
                     int16_t **output_i, 
                     float **output_f, 
                     long samples);
  
/** \ingroup audio_decode
 *  \brief Get the position of the last decoded sample.
 *  \param file A quicktime handle
 *  \param track index (starting with 0)
 *  \returns The position of the last decoded sample.
 * 
 * Returns the position of the last decoded sample. It is updated by the codec in each decode
 * call and resembles the true position of the stream. Therefore, after each decode call,
 * the last position should increment by the number of samples you decoded. If it's smaller,
 * it means, that no more samples are available and that the end of the track is reached.
 */
  
int64_t lqt_last_audio_position(quicktime_t * file, int track);
  
/** \ingroup audio_encode
 *  \brief Encode a number of audio samples for the first track
 *  \param file A quicktime handle
 *  \param output_i 16 bit integer output buffer (or NULL)
 *  \param output_f floating point output buffer (or NULL)
 *  \param samples Number of samples to decode
 *  \param track index (starting with 0)
 *
 * Same as \ref quicktime_encode_audio but with an additional track argument
 * for encoding files with more than one audio track. If you want to pass the full
 * resolution even for 24/32 bit audio, use \ref lqt_encode_audio_raw .
 */
  
int lqt_encode_audio_track(quicktime_t *file, 
                           int16_t **output_i, 
                           float **output_f, 
                           long samples,
                           int track);
  
/** \ingroup audio_decode
 *  \brief Decode a number of audio samples
 *  \param file A quicktime handle
 *  \param output_i 16 bit integer output buffer (or NULL)
 *  \param output_f floating point output buffer (or NULL)
 *  \param samples Number of samples to decode
 *  \param track index (starting with 0)
 *
 * Decode a number of samples from an audio track. All channels are decoded at once.
 * output_i and output_f point to noninterleaved arrays for each channel. Depending
 * on what you need, set either output_i or output_f to NULL. If you want the full resolution
 * also for 24/32 bits, use \ref lqt_decode_audio_raw .
 *
 * The number of actually decoded samples (and EOF) can be obtained with
 * \ref lqt_last_audio_position
 */
  
int lqt_decode_audio_track(quicktime_t *file, 
                           int16_t **output_i, 
                           float **output_f, 
                           long samples,
                           int track);

/*
 *  Support for "raw" audio en-/decoding: This bypasses all
 *  internal sampleformat conversions, and allows access to audio
 *  samples in a format, which is closest to the internal representation.
 */
  
/*
 *  Query the internal sample format. Works for decoding (call after quicktime_open)
 *  and encoding (call after lqt_add_audio_track, lqt_set_audio or quicktime_set_audio).
 */

/** \ingroup audio
 * \brief Get a human readable description for a sample format
 * \param sampleformat A sampleformat
 * \returns The description or NULL
 */
  
const char * lqt_sample_format_to_string(lqt_sample_format_t sampleformat);

/** \ingroup audio
 * \brief Return the sample format used natively by the codec.
 * \param file A quicktime handle
 * \param track Track index (starting with 0)
 * \returns The sampleformat
 *
 * Use this function if you want to use \ref lqt_decode_audio_raw or
 * \ref lqt_encode_audio_raw
 * to bypass libquicktimes internal sample format conversion routines.
 *
 * *Note*
 * Some codecs call 
 * 
 */
  
lqt_sample_format_t lqt_get_sample_format(quicktime_t * file, int track);

/* The following return the actual number of en-/decoded frames */

/** \ingroup audio_decode
 * \brief Decode audio in the native sampleformat of the codec
 * \param file A quicktime handle
 * \param output An array of interleaved samples
 * \param samples Number of samples to decode
 * \param track Track index (starting with 0)
 * \returns The number of actually decoded samples. 0 means end of track.
 *
 * This function bypasses all internal sampleformat conversion and allows
 * full resolution output for up to 32 bit integer and 32 bit float.
 * To check, which dataformat the samples will have, use \ref lqt_get_sample_format .
 *
 * The number of actually decoded samples (and EOF) can be obtained with
 * \ref lqt_last_audio_position
 */

int lqt_decode_audio_raw(quicktime_t *file, 
                         void * output, 
                         long samples,
                         int track);

/** \ingroup audio_decode
 *  \brief Get the audio language
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param language Returns ISO-639 Language code
 *  \returns 1 on success, 0 on error.
 *
 *  The language code is a 3-character code, English is "eng",
 *  Japanese is "jpn".
 */

int lqt_get_audio_language(quicktime_t * file, int track, char * language);

  
/** \ingroup audio_encode
 * \brief Encode audio in the native sampleformat of the codec
 * \param file A quicktime handle
 * \param input An array of interleaved samples
 * \param samples Number of samples to encode
 * \param track Track index (starting with 0)
 * \returns The number of encoded samples or 0 if an error occurred.
 *
 * This function bypasses all internal sampleformat conversion and allows
 * full resolution input for up to 32 bit integer and 32 bit float.
 * To check, which dataformat the samples will have, use \ref lqt_get_sample_format .
 */
  
int lqt_encode_audio_raw(quicktime_t *file, 
                         void * input, 
                         long samples,
                         int track);

/** \ingroup video_decode
 *  \brief Seek to a specific video time
 * \param file A quicktime handle
 * \param time The desired time of the next frame in timescale tics (starting with 0)
 * \param track index (starting with 0)
 *
 * Use this for seeking. During sequential decode calls, the position will be updated automatically.
 * Replacement of \ref quicktime_set_video_position
 * which also works for streams with nonconstant framerate
 */

void lqt_seek_video(quicktime_t * file, int track,
                    int64_t time);
  
/** \ingroup audio_decode
 * \brief Get the number of audio track edit segments
 *  \param file A quicktime handle
 *  \param track Audio track index (starting with 0)
 */

long lqt_audio_edit_list_total_entries(quicktime_t * file, int track);

/** \ingroup audio_decode
 * \brief Get the duration of a audio track edit segment
 *  \param file A quicktime handle
 *  \param track Audio track index (starting with 0)
 *  \param entry_index Index into the edit segments
 */

long lqt_audio_edit_duration(quicktime_t * file, int track, int entry_index);

/** \ingroup audio_decode
 * \brief Get the time offset of a audio track edit segment
 *  \param file A quicktime handle
 *  \param track Audio track index (starting with 0)
 *  \param entry_index Index into the edit segments
 */

long lqt_audio_edit_time(quicktime_t * file, int track, int entry_index);

/** \ingroup audio_decode
 * \brief Get the rate of a audio track edit segment
 *  \param file A quicktime handle
 *  \param track Audio track index (starting with 0)
 *  \param entry_index Index into the edit segments
 */

float lqt_audio_edit_rate(quicktime_t * file, int track, int entry_index);

/*
 *  AVI Specific stuff
 */

/** \ingroup general
 * \brief Query if the function is an AVI
 * \param file A quicktime handle
 * \returns 1 if the file is an AVI, 0 else
 */
  
int lqt_is_avi(quicktime_t *file);

/** \ingroup general
 * \brief Get the WAVE id of an audio track
 * \param file A quicktime handle
 * \param track index (starting with 0)
 * \returns The WAVE id of the file.
 *
 * This is the counterpart of \ref quicktime_audio_compressor for AVI files.
 * It returns the 16 bit compression ID of the track.
 */

int lqt_get_wav_id(quicktime_t *file, int track);
  
/** \ingroup audio_decode
 *  \brief Get the total number of audio channels across all tracks
 *  \param file A quicktime handle
 *  \returns The total channel count
 *
 * The result of this function is only meaningful in conjunction with
 * \ref lqt_decode_audio . In the general case, you should expect the audio tracks
 * in the file as completely independent. Use \ref quicktime_track_channels instead.
 */
  
int lqt_total_channels(quicktime_t *file);

/* Extended metadata support */

/** \ingroup metadata
    \brief Set the album for the file
    \param file A quicktime handle
    \param string The album
*/
  
void lqt_set_album(quicktime_t *file, char *string);

/** \ingroup metadata
    \brief Set the artist for the file
    \param file A quicktime handle
    \param string The artist
*/
  
void lqt_set_artist(quicktime_t *file, char *string);

/** \ingroup metadata
    \brief Set the genre for the file
    \param file A quicktime handle
    \param string The genre
*/

void lqt_set_genre(quicktime_t *file, char *string);

/** \ingroup metadata
    \brief Set the track number for the file
    \param file A quicktime handle
    \param string The track number (as string)
*/
  

void lqt_set_track(quicktime_t *file, char *string);

/** \ingroup metadata
    \brief Set the comment for the file
    \param file A quicktime handle
    \param string The comment
*/

void lqt_set_comment(quicktime_t *file, char *string);

/** \ingroup metadata
    \brief Set the author for the file
    \param file A quicktime handle
    \param string The author
*/

void lqt_set_author(quicktime_t *file, char *string);

/** \ingroup metadata
    \brief Set the creation time for the file
    \param file A quicktime handle
    \param time The creation time
*/

void lqt_set_creation_time(quicktime_t *file, unsigned long time);

  
/** \ingroup metadata
    \brief Get the album from the file
    \param file A quicktime handle
    \returns The album or NULL
*/

char * lqt_get_album(quicktime_t * file);
  
/** \ingroup metadata
    \brief Get the artist from the file
    \param file A quicktime handle
    \returns The artist or NULL
*/
char * lqt_get_artist(quicktime_t * file);

/** \ingroup metadata
    \brief Get the genre from the file
    \param file A quicktime handle
    \returns The genre or NULL
*/

char * lqt_get_genre(quicktime_t * file);

/** \ingroup metadata
    \brief Get the track number from the file
    \param file A quicktime handle
    \returns The track number (as string) or NULL
*/
char * lqt_get_track(quicktime_t * file);

/** \ingroup metadata
    \brief Get the comment from the file
    \param file A quicktime handle
    \returns The comment or NULL
*/

char * lqt_get_comment(quicktime_t *file);

/** \ingroup metadata
    \brief Get the author from the file
    \param file A quicktime handle
    \returns The author or NULL
*/
char * lqt_get_author(quicktime_t *file);

/** \ingroup metadata
    \brief Get the creation time from the file
    \param file A quicktime handle
    \returns The creation time
*/
unsigned long lqt_get_creation_time(quicktime_t * file);
  
/* get track number from track id */
int lqt_track_from_id(quicktime_t *file, int track_id);

/** \ingroup general
    \brief Get a human readable filetype
    \param type An lqt filetype
    \returns A human readable description of the fileformat
*/

const char * lqt_file_type_to_string(lqt_file_type_t type);

/** \ingroup general
    \brief Get the filetype
    \param file A quicktime handle
    \returns The file type
*/

lqt_file_type_t lqt_get_file_type(quicktime_t * file);
 
  
/** \ingroup general
    \brief Open a file for reading
    \param filename A path to a regular file
    \returns An initialized file handle or NULL if opening failed.
    
*/

quicktime_t * lqt_open_read(const char * filename);

/** \ingroup general
    \brief Open a file for reading
    \param filename A path to a regular file
    \param cb Log callback
    \param log_data Data for log callback
    \returns An initialized file handle or NULL if opening failed.
    
*/

  quicktime_t * lqt_open_read_with_log(const char * filename, lqt_log_callback_t cb, void * log_data);
  
/** \ingroup general
    \brief Open a file for writing
    \param filename A path to a regular file
    \param type The type of the file, you want to create
    \returns An initialized file handle or NULL if opening failed.
    
*/

quicktime_t * lqt_open_write(const char * filename, lqt_file_type_t type);

/** \ingroup general
    \brief Open a file for writing
    \param filename A path to a regular file
    \param type The type of the file, you want to create
    \param cb Log callback
    \param log_data Data for log callback
    \returns An initialized file handle or NULL if opening failed.
    
*/

quicktime_t * lqt_open_write_with_log(const char * filename, lqt_file_type_t type,
                                      lqt_log_callback_t cb, void * log_data);
  
/** \ingroup general
    \brief Set the segment size for ODML AVIs
    \param file A quicktime handle
    \param size Maximum size of one RIFF in megabytes

    The actual sizes of the RIFFs will be slightly larger,
    therefore, you shouldn't set this to exactly to 2 GB.
    The default is 1 GB, which is reasonable, so there might
    be no reason to call this function at all.
*/

void lqt_set_max_riff_size(quicktime_t * file, int size);


/** \ingroup audio_encode
 *  \brief Set an audio pts offset
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param offset PTS of the first audio sample (in samples)
 */

void lqt_set_audio_pts_offset(quicktime_t * file, int track, int64_t offset);
  
/** \ingroup audio_decode
 *  \brief Get an audio pts offset
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \returns PTS of the first audio sample (in samples)
 */

int64_t lqt_get_audio_pts_offset(quicktime_t * file, int track);

/** \ingroup video_encode
 *  \brief Set an video pts offset
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param offset PTS of the first video frame (in timescale units)
 */

void lqt_set_video_pts_offset(quicktime_t * file, int track, int64_t offset);
  
/** \ingroup video_decode
 *  \brief Get an video pts offset
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \returns PTS of the first video frame (in timescale units)
 */

int64_t lqt_get_video_pts_offset(quicktime_t * file, int track);

/** \ingroup text_encode
 *  \brief Set an video pts offset
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \param offset PTS offset of the subtitles (in timescale units)
 */

void lqt_set_text_pts_offset(quicktime_t * file, int track, int64_t offset);
  
/** \ingroup text_decode
 *  \brief Get an video pts offset
 *  \param file A quicktime handle
 *  \param track Track index (starting with 0)
 *  \returns PTS offset of the subtitles (in timescale units)
 */

int64_t lqt_get_text_pts_offset(quicktime_t * file, int track);
  
#ifdef __cplusplus
}
#endif /* __cplusplus */

#pragma GCC visibility pop
  
#endif

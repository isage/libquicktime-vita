/*******************************************************************************
 lqt_codecinfo.c

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
#include "lqt_codecinfo_private.h"
#define LQT_LIBQUICKTIME
#include <quicktime/lqt_codecapi.h>
#include <sys/stat.h>
#include <pthread.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "codecs.h"

#define LOG_DOMAIN "codecinfo"

#ifndef PATH_MAX
 #ifdef MAX_PATH
  #define PATH_MAX MAX_PATH
 #else
  #define PATH_MAX 256
 #endif
#endif


/* Forward declaration */

static lqt_codec_info_t *
sort_codecs_internal(lqt_codec_info_t * original, char * names);

/*
 *  Quick and dirty strdup function for the case it's not there
 */

static char * __lqt_strdup(const char * string)
  {
  char * ret = malloc(strlen(string)+1);
  strcpy(ret, string);
  return ret;
  }

static char * __lqt_fourccdup(const char * fourcc)
  {
  char * ret = malloc(5);
  memcpy(ret, fourcc, 5);
  return ret;
  };

/*
 *  Codec Registry
 */

int lqt_num_audio_codecs = 0;
int lqt_num_video_codecs = 0;

lqt_codec_info_t * lqt_audio_codecs = (lqt_codec_info_t*)0;
lqt_codec_info_t * lqt_video_codecs = (lqt_codec_info_t*)0;

static int registry_init_done = 0;
pthread_mutex_t codecs_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 *  Lock and unlock the codec registry
 */

void lqt_registry_lock()
  {
  pthread_mutex_lock(&codecs_mutex);
  }

void lqt_registry_unlock()
  {
  pthread_mutex_unlock(&codecs_mutex);
  }

/* Free memory of parameter info */

static void destroy_parameter_info(lqt_parameter_info_t * p)
  {
  int i;
  if(p->name)
    free(p->name);
  if(p->real_name)
    free(p->real_name);
  if(p->help_string)
    free(p->help_string);

  switch(p->type)
    {
    case LQT_PARAMETER_STRING:
      if(p->val_default.val_string)
        free(p->val_default.val_string);
      break;
    case LQT_PARAMETER_STRINGLIST:
      if(p->val_default.val_string)
        free(p->val_default.val_string);
      if(p->stringlist_options)
        {
        for(i = 0; i < p->num_stringlist_options; i++)
          free(p->stringlist_options[i]);
        free(p->stringlist_options);
        }
      if(p->stringlist_labels)
        {
        for(i = 0; i < p->num_stringlist_options; i++)
          free(p->stringlist_labels[i]);
        free(p->stringlist_labels);
        }
      break;
    default: /* Keep gcc quiet */
      break;
    }
  }

/* Free memory of codec info (public) */

void lqt_codec_info_destroy_single(lqt_codec_info_t * ptr)
  {
  int i;

  if(ptr->fourccs)
    {
    for(i = 0; i < ptr->num_fourccs; i++)
      free(ptr->fourccs[i]);
    free(ptr->fourccs);
    }

  if(ptr->wav_ids)
    free(ptr->wav_ids);
  
  if(ptr->name)
    free(ptr->name);          /* Name of the codec              */
  if(ptr->long_name)          /* Long name of the codec         */
    free(ptr->long_name);
  if(ptr->description)        /* Long name of the codec         */
    free(ptr->description);
  if(ptr->module_filename)        /* Module filename       */
    free(ptr->module_filename);

  if(ptr->encoding_colormodels)
    free(ptr->encoding_colormodels);
  
  if(ptr->encoding_parameters)
    {
    for(i = 0; i < ptr->num_encoding_parameters; i++)
      destroy_parameter_info(&ptr->encoding_parameters[i]);
    free(ptr->encoding_parameters);
    }

  if(ptr->decoding_parameters)
    {
    for(i = 0; i < ptr->num_decoding_parameters; i++)
      destroy_parameter_info(&ptr->decoding_parameters[i]);
    free(ptr->decoding_parameters);
    }
  if(ptr->image_sizes)
    free(ptr->image_sizes);
  free(ptr);
  }

static void copy_parameter_value(lqt_parameter_value_t * dst,
                                 const lqt_parameter_value_t * src,
                                 lqt_parameter_type_t type)
  {
  switch(type)
    {
    case LQT_PARAMETER_INT:
      dst->val_int = src->val_int;
      break;
    case LQT_PARAMETER_FLOAT:
      dst->val_float = src->val_float;
      break;
    case LQT_PARAMETER_STRING:
    case LQT_PARAMETER_STRINGLIST: /* String with options */

      if(dst->val_string)
        free(dst->val_string);

      if(src->val_string)
        dst->val_string = __lqt_strdup(src->val_string);
      else
        dst->val_string = (char*)0;
      break;
    case LQT_PARAMETER_SECTION:
      break;
    }

  }

static void
copy_parameter_info(lqt_parameter_info_t * ret,
                    const lqt_parameter_info_t * info)
  {
  int i;
  
  if(info->name)
    ret->name = __lqt_strdup(info->name);
  if(info->real_name)
    ret->real_name = __lqt_strdup(info->real_name);
  if(info->help_string)
    ret->help_string = __lqt_strdup(info->help_string);
  
  ret->type = info->type;

  switch(ret->type)
    {
    case LQT_PARAMETER_INT:
      ret->val_min.val_int = info->val_min.val_int;
      ret->val_max.val_int = info->val_max.val_int;
      break;
    case LQT_PARAMETER_FLOAT:
      ret->val_min.val_float = info->val_min.val_float;
      ret->val_max.val_float = info->val_max.val_float;
      ret->num_digits = info->num_digits;
      break;
    case LQT_PARAMETER_STRING:
      break;
    case LQT_PARAMETER_STRINGLIST: /* String with options */
      ret->num_stringlist_options = info->num_stringlist_options;
      ret->stringlist_options = calloc(ret->num_stringlist_options,
                                       sizeof(char*));
      ret->stringlist_labels = calloc(ret->num_stringlist_options,
                                      sizeof(char*));
      
      for(i = 0; i < ret->num_stringlist_options; i++)
        {
        ret->stringlist_options[i] =
          __lqt_strdup(info->stringlist_options[i]);
        ret->stringlist_labels[i] =
          __lqt_strdup(info->stringlist_labels[i]);
        }

      break;
    case LQT_PARAMETER_SECTION: /* String with options */
      break;
    }

  copy_parameter_value(&ret->val_default,
                       &info->val_default,
                       info->type);

  }

/*
 *  Copy codec Info
 */

lqt_codec_info_t * lqt_codec_info_copy_single(const lqt_codec_info_t * info)
  {
  int i, len;
  lqt_codec_info_t * ret = calloc(1, sizeof(*ret));
//   fprintf(stderr, "lqt_codec_info_copy_single %s\n", info->name);
  ret->compatibility_flags = info->compatibility_flags;
    
  if(info->name)
    ret->name = __lqt_strdup(info->name);
  if(info->long_name)
    ret->long_name = __lqt_strdup(info->long_name);
  if(info->description)
    ret->description = __lqt_strdup(info->description);
  
  if(info->module_filename)
    ret->module_filename = __lqt_strdup(info->module_filename);

  ret->module_index = info->module_index;
  
  ret->type = info->type;
  ret->direction = info->direction;
  
  ret->num_fourccs = info->num_fourccs;
  if(ret->num_fourccs)
    {
    ret->fourccs = malloc(ret->num_fourccs * sizeof(char*));
    for(i = 0; i < ret->num_fourccs; i++)
      ret->fourccs[i] = __lqt_fourccdup(info->fourccs[i]);
    }

  ret->num_encoding_colormodels = info->num_encoding_colormodels;
  if(ret->num_encoding_colormodels)
    {
    /* +1 to copy the terminating LQT_COLORMODEL_NODE */
    len = (ret->num_encoding_colormodels + 1) * sizeof(*ret->encoding_colormodels);
    ret->encoding_colormodels = malloc(len);
    memcpy(ret->encoding_colormodels, info->encoding_colormodels, len);
    }
  
  ret->num_wav_ids = info->num_wav_ids;
  if(ret->num_wav_ids)
    {
    len = ret->num_wav_ids * sizeof(*ret->wav_ids);
    ret->wav_ids = malloc(len);
    memcpy(ret->wav_ids, info->wav_ids, len);
    }

  ret->num_image_sizes = info->num_image_sizes;
  if(ret->num_image_sizes)
    {
    len = ret->num_image_sizes * sizeof(*ret->image_sizes);
    ret->image_sizes = malloc(len);
    memcpy(ret->image_sizes, info->image_sizes, len);
    }
  
  ret->num_encoding_parameters = info->num_encoding_parameters;
  
  if(ret->num_encoding_parameters)
    {
    ret->encoding_parameters =
      calloc(ret->num_encoding_parameters+1, sizeof(lqt_parameter_info_t));

    for(i = 0; i < ret->num_encoding_parameters; i++)
      copy_parameter_info(&ret->encoding_parameters[i],
                          &info->encoding_parameters[i]);
    }

  ret->num_decoding_parameters = info->num_decoding_parameters;
  if(ret->num_decoding_parameters)
    {
    ret->decoding_parameters =
      calloc(ret->num_decoding_parameters, sizeof(lqt_parameter_info_t));

    for(i = 0; i < ret->num_decoding_parameters; i++)
      copy_parameter_info(&ret->decoding_parameters[i],
                          &info->decoding_parameters[i]);
    }
  ret->compression_id = info->compression_id;
  return ret;
  }


static lqt_codec_info_t * load_codec_info_from_audioplugin()
  {
  void * module;

  lqt_codec_info_t * ret_end;

  int i;

  int num_codecs;

  lqt_codec_info_t * ret = (lqt_codec_info_t*)0;

  num_codecs = audiocodec_get_num_codecs();
  if(!num_codecs)
    {
    lqt_log(NULL, LQT_LOG_ERROR, LOG_DOMAIN, "No codecs found in audiocodec");
    return ret;
    }

  ret = lqt_create_codec_info(audiocodec_get_codec_info(0));


  ret_end = ret;
  ret_end->module_index = 0;
  ret_end->module_filename = __lqt_strdup("audio");

  for(i = 1; i < num_codecs; i++)
    {
    ret_end->next = lqt_create_codec_info(audiocodec_get_codec_info(i));
    ret_end = ret_end->next;
    ret_end->module_index = i;
    ret_end->module_filename = __lqt_strdup("audio");
    }
  ret_end->next = (lqt_codec_info_t*)0;
  return ret;
  }

static lqt_codec_info_t * load_codec_info_from_videoplugin()
  {
  void * module;

  lqt_codec_info_t * ret_end;

  int i;

  int num_codecs;

  lqt_codec_info_t * ret = (lqt_codec_info_t*)0;

  num_codecs = videocodec_get_num_codecs();
  if(!num_codecs)
    {
    lqt_log(NULL, LQT_LOG_ERROR, LOG_DOMAIN, "No codecs found in videocodec");
    return ret;
    }

  ret = lqt_create_codec_info(videocodec_get_codec_info(0));


  ret_end = ret;
  ret_end->module_index = 0;
  ret_end->module_filename = __lqt_strdup("video");

  for(i = 1; i < num_codecs; i++)
    {
    ret_end->next = lqt_create_codec_info(videocodec_get_codec_info(i));
    ret_end = ret_end->next;
    ret_end->module_index = i;
    ret_end->module_filename = __lqt_strdup("video");
    }
  ret_end->next = (lqt_codec_info_t*)0;
  return ret;
  }

static lqt_codec_info_t * load_codec_info_from_ffmpegplugin()
  {
  void * module;

  lqt_codec_info_t * ret_end;

  int i;

  int num_codecs;

  lqt_codec_info_t * ret = (lqt_codec_info_t*)0;

  num_codecs = ffmpegcodec_get_num_codecs();
  if(!num_codecs)
    {
    lqt_log(NULL, LQT_LOG_ERROR, LOG_DOMAIN, "No codecs found in ffmpegcodec");
    return ret;
    }

  ret = lqt_create_codec_info(ffmpegcodec_get_codec_info(0));


  ret_end = ret;
  ret_end->module_index = 0;
  ret_end->module_filename = __lqt_strdup("ffmpeg");

  for(i = 1; i < num_codecs; i++)
    {
    ret_end->next = lqt_create_codec_info(ffmpegcodec_get_codec_info(i));
    ret_end = ret_end->next;
    ret_end->module_index = i;
    ret_end->module_filename = __lqt_strdup("ffmpeg");
    }
  ret_end->next = (lqt_codec_info_t*)0;
  return ret;
  }



/*
 *   Register all codecs found in list
 */

static void register_codecs(lqt_codec_info_t * list,
                            lqt_codec_info_t ** audio_codecs_end,
                            lqt_codec_info_t ** video_codecs_end)
  {
  lqt_codec_info_t * tmp_ptr;
  while(list)
    {
    if(list->type == LQT_CODEC_AUDIO)
      {
      if(*audio_codecs_end)
        {
        (*audio_codecs_end)->next = list;
        *audio_codecs_end = (*audio_codecs_end)->next;
        }
      else
        {
        lqt_audio_codecs = list;
            (*audio_codecs_end) = lqt_audio_codecs;
        }
      lqt_num_audio_codecs++;
      }
    if(list->type == LQT_CODEC_VIDEO)
      {
      if((*video_codecs_end))
        {
        (*video_codecs_end)->next = list;
        (*video_codecs_end) = (*video_codecs_end)->next;
        }
      else
        {
        lqt_video_codecs = list;
        (*video_codecs_end) = lqt_video_codecs;
        }
      lqt_num_video_codecs++;
      }
    tmp_ptr = list;
    list = list->next;
    tmp_ptr->next = (lqt_codec_info_t*)0;
    }
  }

static int scan_for_plugins()
  {
  lqt_codec_info_t * codecs;

  lqt_codec_info_t * video_codecs_end;
  lqt_codec_info_t * audio_codecs_end;

  audio_codecs_end = lqt_audio_codecs;

  if(audio_codecs_end)
    while(audio_codecs_end->next)
      audio_codecs_end = audio_codecs_end->next;

  video_codecs_end = lqt_video_codecs;

  if(video_codecs_end)
    while(video_codecs_end->next)
      video_codecs_end = video_codecs_end->next;

  codecs = load_codec_info_from_audioplugin();
  register_codecs(codecs, &audio_codecs_end, &video_codecs_end);

  codecs = load_codec_info_from_audioplugin();
  register_codecs(codecs, &audio_codecs_end, &video_codecs_end);

  codecs = load_codec_info_from_ffmpegplugin();
  register_codecs(codecs, &audio_codecs_end, &video_codecs_end);
  return 1;
  }

void lqt_registry_destroy()
  {
  lqt_codec_info_t * tmp;

  while(lqt_audio_codecs)
    {
    tmp = lqt_audio_codecs->next;
    lqt_codec_info_destroy_single(lqt_audio_codecs);
    lqt_audio_codecs = tmp;
    }

  while(lqt_video_codecs)
    {
    tmp = lqt_video_codecs->next;
    lqt_codec_info_destroy_single(lqt_video_codecs);
    lqt_video_codecs = tmp;
    }

  lqt_num_video_codecs = 0;
  lqt_num_audio_codecs = 0;
  
  }

void lqt_registry_init()
  {
  int do_write = 0;
  char * audio_order = (char*)0;
  char * video_order = (char*)0;
  
  lqt_codec_info_t * file_codecs;
  lqt_codec_info_t * tmp_file_codecs;

  if(registry_init_done)
    {
    return;
    }

  registry_init_done = 1;

  if(lqt_audio_codecs || lqt_video_codecs)
    {
    return;
    }
  
  /* Scan for the plugins, use cached values if possible */

  scan_for_plugins();

  /*
   *  If there were codecs in the database, which have
   *  disappeared, they must be deleted now
   */
  
/*  while(file_codecs)
    {
    tmp_file_codecs = file_codecs;
    file_codecs = file_codecs->next;
    lqt_codec_info_destroy_single(tmp_file_codecs);
    }*/
  
  /*
   *  Write the file again, so we can use it the next time
   */

  /* Sort the codecs */

  if(audio_order)
    {
    lqt_audio_codecs =
      sort_codecs_internal(lqt_audio_codecs, audio_order);
    free(audio_order);
    }

  if(video_order)
    {
    lqt_video_codecs =
      sort_codecs_internal(lqt_video_codecs, video_order);
    free(video_order);
    }
  
  }

/*
 *  Get the numbers of codecs
 */

int lqt_get_num_audio_codecs() { return lqt_num_audio_codecs; }

int lqt_get_num_video_codecs() { return lqt_num_video_codecs; }

/*
 *   Get corresponding info structures
 *   These point to the original database entries,
 *   so they are returned as const here
 */

const lqt_codec_info_t * lqt_get_audio_codec_info(int index)
  {
  const lqt_codec_info_t * ret;
  int i;

  if((index < 0) || (index >= lqt_num_audio_codecs))
    return (lqt_codec_info_t *)0;

  ret = lqt_audio_codecs;
  
  for(i = 0; i < index; i++)
    ret = ret->next;
  
  return ret;
  }

const lqt_codec_info_t * lqt_get_video_codec_info(int index)
  {
  const lqt_codec_info_t * ret;
  int i;

  if((index < 0) || (index >= lqt_num_video_codecs))
    return (lqt_codec_info_t *)0;

  ret = lqt_video_codecs;
  
  for(i = 0; i < index; i++)
    ret = ret->next;
  
  return ret;
  }

/* Thread save methods of getting codec infos */

static void 
create_parameter_info(lqt_parameter_info_t * ret,
                    const lqt_parameter_info_static_t * info)
  {
  int i;
  ret->name = __lqt_strdup(info->name);           /* Parameter name  */
  ret->real_name = __lqt_strdup(info->real_name); /* Parameter name  */

  if(info->help_string)
    ret->help_string = __lqt_strdup(info->help_string);
  
  ret->type = info->type;

  switch(ret->type)
    {
    case LQT_PARAMETER_INT:
      ret->val_default.val_int = info->val_default.val_int;
      ret->val_min.val_int = info->val_min.val_int;
      ret->val_max.val_int = info->val_max.val_int;
      break;
    case LQT_PARAMETER_FLOAT:
      ret->val_default.val_float = info->val_default.val_float;
      ret->val_min.val_float = info->val_min.val_float;
      ret->val_max.val_float = info->val_max.val_float;
      ret->num_digits = info->num_digits;
      break;
    case LQT_PARAMETER_STRING:
      ret->val_default.val_string = __lqt_strdup(info->val_default.val_string);
      break;
    case LQT_PARAMETER_STRINGLIST:
      ret->val_default.val_string = __lqt_strdup(info->val_default.val_string);
      if(!info->stringlist_options)
        {
        lqt_log(NULL, LQT_LOG_ERROR, LOG_DOMAIN, "Stringlist parameter %s has NULL options",
                info->name);
        return;
        }

      /* Count the options */

      ret->num_stringlist_options = 0;
      
      while(1)
        {
        if(info->stringlist_options[ret->num_stringlist_options])
          ret->num_stringlist_options++;
        else
          break;
        }

      /* Now, copy them */

      ret->stringlist_options = malloc(ret->num_stringlist_options * sizeof(char *));
      for(i = 0; i < ret->num_stringlist_options; i++)
        {
          ret->stringlist_options[i] =
            __lqt_strdup(info->stringlist_options[i]);
        }

      /* Labels */
      ret->stringlist_labels = malloc(ret->num_stringlist_options * sizeof(char *));
      if(info->stringlist_labels)
        {
        for(i = 0; i < ret->num_stringlist_options; i++)
          {
          ret->stringlist_labels[i] =
            __lqt_strdup(info->stringlist_labels[i]);
          }
        }
      else
        {
        for(i = 0; i < ret->num_stringlist_options; i++)
          {
          ret->stringlist_labels[i] =
            __lqt_strdup(info->stringlist_options[i]);
          }
        }
      break;
    default:
      break;
    }
  
  }

lqt_codec_info_t *
lqt_create_codec_info(const lqt_codec_info_static_t * template)
  {
  int i;
  lqt_codec_info_t * ret;

  if(!template->fourccs)
    {
    lqt_log(NULL, LQT_LOG_ERROR, LOG_DOMAIN, "Codec %s has no fourccs defined", template->name);
    return (lqt_codec_info_t*)0;
    }
  
  ret = calloc(1, sizeof(lqt_codec_info_t));

  ret->compatibility_flags = template->compatibility_flags;

  ret->name =        __lqt_strdup(template->name);
  ret->long_name =   __lqt_strdup(template->long_name);
  ret->description = __lqt_strdup(template->description);

  ret->type      = template->type;
  ret->direction = template->direction;

  /* Copy fourccs */
  
  ret->num_fourccs = 0;
  
  while(1)
    {
    if(template->fourccs[ret->num_fourccs])
      ret->num_fourccs++;
    else
      break;
    }

  ret->fourccs = malloc(ret->num_fourccs * sizeof(char*));
  for(i = 0; i < ret->num_fourccs; i++)
    ret->fourccs[i] = __lqt_fourccdup(template->fourccs[i]);

  /* Copy supported encoding colormodels */

  ret->num_encoding_colormodels = 0;

  if(template->encoding_colormodels)
    {
    while(1)
      {
      if(template->encoding_colormodels[ret->num_encoding_colormodels] != LQT_COLORMODEL_NONE)
        ret->num_encoding_colormodels++;
      else
        break;
      }

    ret->encoding_colormodels =
      malloc((ret->num_encoding_colormodels+1) * sizeof(*ret->encoding_colormodels));
    for(i = 0; i < ret->num_encoding_colormodels; i++)
      ret->encoding_colormodels[i] = template->encoding_colormodels[i];
    ret->encoding_colormodels[ret->num_encoding_colormodels] =
      LQT_COLORMODEL_NONE;
    }
  
  /* Copy wav_ids */

  ret->num_wav_ids = 0;

  if(template->wav_ids)
    {
    while(1)
      {
      if(template->wav_ids[ret->num_wav_ids] != LQT_WAV_ID_NONE)
        ret->num_wav_ids++;
      else
        break;
      }
    ret->wav_ids = malloc(ret->num_wav_ids * sizeof(int));
    for(i = 0; i < ret->num_wav_ids; i++)
      ret->wav_ids[i] = template->wav_ids[i];
    }

  /* Copy image_sizes */

  ret->num_image_sizes = 0;

  if(template->image_sizes)
    {
    while(template->image_sizes[ret->num_image_sizes].width)
      ret->num_image_sizes++;

    if(ret->num_image_sizes)
      {
      ret->image_sizes = malloc(ret->num_image_sizes *
                                sizeof(*ret->image_sizes));
      for(i = 0; i < ret->num_image_sizes; i++)
        {
        ret->image_sizes[i].width = template->image_sizes[i].width;
        ret->image_sizes[i].height = template->image_sizes[i].height;
        }
      }
    }
  
  /* Copy encoding parameters */
  
  if(template->encoding_parameters)
    {
    ret->num_encoding_parameters = 0;
    while(1)
      {
      if(template->encoding_parameters[ret->num_encoding_parameters].name)
        ret->num_encoding_parameters++;
      else
        break;
      }
    }

  if(ret->num_encoding_parameters)
    {
    ret->encoding_parameters =
      calloc(ret->num_encoding_parameters,
             sizeof(lqt_parameter_info_t));
    for(i = 0; i < ret->num_encoding_parameters; i++)
      {
      /* Copy parameter info */
      create_parameter_info(&ret->encoding_parameters[i],
                            &template->encoding_parameters[i]);
      }
    }
  else
    {
    ret->encoding_parameters = (lqt_parameter_info_t*)0;
    }

  if(template->decoding_parameters)
    {
    ret->num_decoding_parameters = 0;
    while(1)
      {
      if(template->decoding_parameters[ret->num_decoding_parameters].name)
        ret->num_decoding_parameters++;
      else
        break;
      }
    }

  if(ret->num_decoding_parameters)
    {
    ret->decoding_parameters =
      calloc(ret->num_decoding_parameters,
             sizeof(lqt_parameter_info_t));
    for(i = 0; i < ret->num_decoding_parameters; i++)
      {
      /* Copy parameter info */
      create_parameter_info(&ret->decoding_parameters[i],
                            &template->decoding_parameters[i]);
      }
    }
  else
    {
    ret->decoding_parameters = (lqt_parameter_info_t*)0;
    }

  ret->compression_id = template->compression_id;
  
  return ret;
  }

static void dump_codec_parameter(lqt_parameter_info_t * p)
  {
  int i;
  lqt_dump("Parameter: %s (%s) ", p->name,
          p->real_name);
  lqt_dump("Type: ");
  switch(p->type)
    {
    case LQT_PARAMETER_INT:
      lqt_dump("Integer, Default Value: %d ",
              p->val_default.val_int);

      if(p->val_min.val_int < p->val_max.val_int)
        lqt_dump("(%d..%d)\n",
                p->val_min.val_int, p->val_max.val_int);
      else
        lqt_dump("(unlimited)\n");
      break;
    case LQT_PARAMETER_FLOAT:
      lqt_dump("Float, Default Value: %f ",
              p->val_default.val_float);

      if(p->val_min.val_float < p->val_max.val_float)
        lqt_dump("(%f..%f)\n",
                p->val_min.val_float, p->val_max.val_float);
      else
        lqt_dump("(unlimited)\n");
      break;
    case LQT_PARAMETER_STRING:
      lqt_dump("String, Default Value : %s\n",
              (p->val_default.val_string ? p->val_default.val_string : "NULL"));
      break;
    case LQT_PARAMETER_STRINGLIST:
      lqt_dump("Stringlist, Default Value : %s\n",
              (p->val_default.val_string ? p->val_default.val_string :
               "NULL"));
      lqt_dump("Options: ");
      for(i = 0; i < p->num_stringlist_options; i++)
        lqt_dump("%s ", p->stringlist_options[i]);
      lqt_dump("\n");
      break;
    case LQT_PARAMETER_SECTION:
      lqt_dump("Section");
    }
  if(p->help_string)
    lqt_dump("Help string: %s\n", p->help_string);
  }

void lqt_dump_codec_info(const lqt_codec_info_t * info)
  {
  int i;
  lqt_dump("Codec: %s (%s)\n", info->long_name, info->name);
  
  lqt_dump("Type: %s Direction: ",
          (info->type == LQT_CODEC_AUDIO ? "Audio, " : "Video, ") );
  switch(info->direction)
    {
    case LQT_DIRECTION_ENCODE:
      lqt_dump("Encode\n");
      break;
    case LQT_DIRECTION_DECODE:
      lqt_dump("Decode\n");
      break;
    case LQT_DIRECTION_BOTH:
      lqt_dump("Encode/Decode\n");
      break;
    }

  lqt_dump("Description:\n%s\n", info->description);

  lqt_dump("Four character codes: (fourccs)\n");
  for(i = 0; i < info->num_fourccs; i++)
    lqt_dump("%s (0x%08x)\n", info->fourccs[i],
            LQT_STRING_2_FOURCC(info->fourccs[i]));

  if(info->compression_id != LQT_COMPRESSION_NONE)
    lqt_dump("Compression ID: %s\n",
             lqt_compression_id_to_string(info->compression_id));
  
  if(!info->num_encoding_parameters)
    {
    lqt_dump("No settable parameters for encoding\n");
    }
  else
    {
    for(i = 0; i < info->num_encoding_parameters; i++)
      dump_codec_parameter(&info->encoding_parameters[i]);
    }

  if(!info->num_encoding_parameters)
    {
    lqt_dump("No settable parameters for decoding\n");
    }
  else
    {
    for(i = 0; i < info->num_decoding_parameters; i++)
      dump_codec_parameter(&info->decoding_parameters[i]);
    }
  lqt_dump("Module filename: %s\nIndex inside module: %d\n",
          info->module_filename, info->module_index);

  }

#define MATCH_FOURCC(a, b) \
( ( a[0]==b[0] ) && \
  ( a[1]==b[1] ) &&\
  ( a[2]==b[2] ) &&\
  ( a[3]==b[3] ) )

/* 
 *   Find codecs: These replace get_acodec_index() and get_vcodec_index()
 *   This returns a pointer to the codec info or NULL if there is none.
 */

lqt_codec_info_t ** lqt_find_audio_codec(char * fourcc, int encode)
  {
  int j;
  lqt_codec_info_t * tmp_ptr = (lqt_codec_info_t*)0;
  lqt_codec_info_t * ptr;

  lqt_codec_info_t ** ret = (lqt_codec_info_t **)0;

  /* also init registry */
  lqt_registry_init();
    
  lqt_registry_lock();
  
  ptr = lqt_audio_codecs;
  
  while(ptr)
    {
    for(j = 0; j < ptr->num_fourccs; j++)
      {
      if(MATCH_FOURCC(ptr->fourccs[j], fourcc))
        {
        if((encode && (ptr->direction != LQT_DIRECTION_DECODE)) ||
           (!encode && (ptr->direction != LQT_DIRECTION_ENCODE)))
          {
          tmp_ptr = ptr;
          break;
          }
        }
      }
    if(tmp_ptr)
      break;
    ptr = ptr->next;
    }
  if(tmp_ptr)
    {
    ret = calloc(2, sizeof(lqt_codec_info_t*));
    *ret = lqt_codec_info_copy_single(tmp_ptr);
    }
  lqt_registry_unlock();
  return ret;
  }

lqt_codec_info_t ** lqt_find_audio_codec_by_wav_id(int wav_id, int encode)
  {
  int j;
  lqt_codec_info_t * tmp_ptr = (lqt_codec_info_t*)0;
  lqt_codec_info_t * ptr;

  lqt_codec_info_t ** ret = (lqt_codec_info_t **)0;

  /* also init registry */
  lqt_registry_init();
  
  lqt_registry_lock();
  
  ptr = lqt_audio_codecs;
  
  while(ptr)
    {
    for(j = 0; j < ptr->num_wav_ids; j++)
      {
      if(ptr->wav_ids[j] == wav_id)
        {
        if((encode && (ptr->direction != LQT_DIRECTION_DECODE)) ||
           (!encode && (ptr->direction != LQT_DIRECTION_ENCODE)))
          {
          tmp_ptr = ptr;
          break;
          }
        }
      }
    if(tmp_ptr)
      break;
    ptr = ptr->next;
    }
  if(tmp_ptr)
    {
    ret = calloc(2, sizeof(lqt_codec_info_t*));
    *ret = lqt_codec_info_copy_single(tmp_ptr);
    }
  lqt_registry_unlock();
  return ret;
  }


lqt_codec_info_t ** lqt_find_video_codec(char * fourcc, int encode)
  {
  int j;
  lqt_codec_info_t * tmp_ptr = (lqt_codec_info_t*)0;
  lqt_codec_info_t * ptr;

  lqt_codec_info_t ** ret = (lqt_codec_info_t **)0;
  
  /* also init registry */
  lqt_registry_init();
  
  lqt_registry_lock();
  
  ptr = lqt_video_codecs;
  
  while(ptr)
    {
    for(j = 0; j < ptr->num_fourccs; j++)
      {
      if(MATCH_FOURCC(ptr->fourccs[j], fourcc))
        {
        if((encode && (ptr->direction != LQT_DIRECTION_DECODE)) ||
           (!encode && (ptr->direction != LQT_DIRECTION_ENCODE)))
          {
          tmp_ptr = ptr;
          break;
          }
        }
      }
    if(tmp_ptr)
      break;
    ptr = ptr->next;
    }
  if(tmp_ptr)
    {
    ret = calloc(2, sizeof(lqt_codec_info_t*));
    *ret = lqt_codec_info_copy_single(tmp_ptr);
    }
  lqt_registry_unlock();

  return ret;
  }

/*
 *  Query codec registry
 */

lqt_codec_info_t ** lqt_query_registry(int audio, int video,
                                       int encode, int decode)
  {
  lqt_codec_info_t ** ret;
  const lqt_codec_info_t * info;
  int num_codecs = 0, num_added = 0, i;

  /* also init registry */
  lqt_registry_init();

  lqt_registry_lock();

  if(audio)
    {
    for(i = 0; i < lqt_num_audio_codecs; i++)
      {
      info = lqt_get_audio_codec_info(i);
      if((encode && (info->direction != LQT_DIRECTION_DECODE)) ||
         (decode && (info->direction != LQT_DIRECTION_ENCODE)))
        num_codecs++;
      }
    }
  if(video)
    {
    for(i = 0; i < lqt_num_video_codecs; i++)
      {
      info = lqt_get_video_codec_info(i);
      if((encode && (info->direction != LQT_DIRECTION_DECODE)) ||
         (decode && (info->direction != LQT_DIRECTION_ENCODE)))
        num_codecs++;
      }
    }

  ret = calloc(num_codecs+1, sizeof(lqt_codec_info_t*));

  if(audio)
    {
    for(i = 0; i < lqt_num_audio_codecs; i++)
      {
      info = lqt_get_audio_codec_info(i);
      if((encode && (info->direction != LQT_DIRECTION_DECODE)) ||
         (decode && (info->direction != LQT_DIRECTION_ENCODE)))
        {
        ret[num_added] = lqt_codec_info_copy_single(info);
        num_added++;
        }
      }
    }
  if(video)
    {
    for(i = 0; i < lqt_num_video_codecs; i++)
      {
      info = lqt_get_video_codec_info(i);
      if((encode && (info->direction != LQT_DIRECTION_DECODE)) ||
         (decode && (info->direction != LQT_DIRECTION_ENCODE)))
        {
        ret[num_added] = lqt_codec_info_copy_single(info);
        num_added++;
        }
      }
    }
  lqt_registry_unlock();
  return ret;
  }

/*
 *  Find a codec by it's unique (short) name
 */

lqt_codec_info_t ** lqt_find_audio_codec_by_name(const char * name)
  {
  const lqt_codec_info_t * info;
  int i;
  lqt_codec_info_t ** ret = (lqt_codec_info_t**)0;

  if(!name)
    return ret;
  
   /* also init registry */
  lqt_registry_init();

  lqt_registry_lock();

  info = lqt_get_audio_codec_info(0);

  for(i = 0; i < lqt_num_audio_codecs; i++)
    {
    if(!strcmp(info->name, name))
      {
      ret = calloc(2, sizeof(lqt_codec_info_t*));
      *ret = lqt_codec_info_copy_single(info);
      break;
      }
    else
      info = info->next;
    }
  lqt_registry_unlock();


  return ret;
  }

lqt_codec_info_t ** lqt_find_video_codec_by_name(const char * name)
  {
  const lqt_codec_info_t * info;
  int i;
  lqt_codec_info_t ** ret = (lqt_codec_info_t**)0;

  if(!name)
    return ret;
  
  /* also init registry */
  lqt_registry_init();
  
  lqt_registry_lock();

  info = lqt_get_video_codec_info(0);

  for(i = 0; i < lqt_num_video_codecs; i++)
    {
    if(!strcmp(info->name, name))
      {
      ret = calloc(2, sizeof(lqt_codec_info_t*));
      *ret = lqt_codec_info_copy_single(info);
      break;
      }
    else
      info = info->next;
    }
  lqt_registry_unlock();


  return ret;
  }

/*
 *  Get infos about the codecs of a file
 *  To be called after quicktime_open() when reading
 *  or quicktime_set_audio()/quicktime_set_video() when writing
 */

lqt_codec_info_t ** lqt_audio_codec_from_file(quicktime_t * file, int track)
  {
  char * name = file->atracks[track].codec->info->name;
  return lqt_find_audio_codec_by_name(name);
  }

lqt_codec_info_t ** lqt_video_codec_from_file(quicktime_t * file, int track)
  {
  char * name = file->vtracks[track].codec->info->name;
  return lqt_find_video_codec_by_name(name);
  }

/*
 *  Destroys the codec info structure returned by the functions
 *  above
 */

void lqt_destroy_codec_info(lqt_codec_info_t ** info)
  {
  lqt_codec_info_t ** ptr = info;

  if(!ptr)
    return;
  
  while(*ptr)
    {
    lqt_codec_info_destroy_single(*ptr);
    ptr++;
    }
  free(info);
  }

void lqt_set_default_parameter(lqt_codec_type type, int encode,
                               const char * codec_name,
                               const char * parameter_name,
                               lqt_parameter_value_t * val)
  {
  int i, imax, parameter_found = 0;
  
  lqt_codec_info_t * codec_info;
  lqt_parameter_info_t * parameter_info;
  
  /* also init registry */
  lqt_registry_init();

  lqt_registry_lock();

  if(type == LQT_CODEC_AUDIO)
    codec_info = lqt_audio_codecs;
  else
    codec_info = lqt_video_codecs;

  /* Search codec */

  while(codec_info)
    {
    if(!strcmp(codec_name, codec_info->name))
      break;
    codec_info = codec_info->next;
    }

  if(!codec_info)
    {
    lqt_log(NULL, LQT_LOG_ERROR, LOG_DOMAIN, "lqt_set_default_parameter: No %s codec %s found",
            ((type == LQT_CODEC_AUDIO) ? "audio" : "video"),  codec_name);
    lqt_registry_unlock();
    return;
    }

  /* Search parameter */

  if(encode)
    {
    imax = codec_info->num_encoding_parameters;
    parameter_info = codec_info->encoding_parameters;
    }
  else
    {
    imax = codec_info->num_decoding_parameters;
    parameter_info = codec_info->decoding_parameters;
    }

  for(i = 0; i < imax; i++)
    {
    if(!strcmp(parameter_info[i].name, parameter_name))
      {
      parameter_found = 1;
      break;
      }
    }

  if(!parameter_found)
    {
    lqt_log(NULL, LQT_LOG_ERROR, LOG_DOMAIN,
            "lqt_set_default_parameter: No parameter %s for codec %s found",
            parameter_name,
            codec_name);
    lqt_registry_unlock();
    return;
    }

  /* Set the value */

  switch(parameter_info[i].type)
    {
    case LQT_PARAMETER_INT:
      parameter_info[i].val_default.val_int = val->val_int;
      break;
    case LQT_PARAMETER_FLOAT:
      parameter_info[i].val_default.val_float = val->val_float;
      break;
    case LQT_PARAMETER_STRING:
    case LQT_PARAMETER_STRINGLIST:
      if(parameter_info[i].val_default.val_string)
        free(parameter_info[i].val_default.val_string);
      parameter_info[i].val_default.val_string =
        __lqt_strdup(val->val_string);
      break;
    case LQT_PARAMETER_SECTION:
      break;
    }


  lqt_registry_unlock();
  return;
  }

/*
 *  I don't want to depend on one of the 1000s of MIN MAX macros out
 *  there
 */

#define __LQT_MIN(a, b) ((a<b)?a:b)

/*
 *  Load the module and restore default parameters.
 *  Parameters are only stored in the return value,
 *  NOT in the registry
 */

void lqt_restore_default_parameters(lqt_codec_info_t * codec_info,
                                    int encode, int decode)
  {
  lqt_codec_info_t * info_from_module;
  lqt_codec_info_static_t * (*get_codec_info)(int);

  int i, imax;
  
  info_from_module = lqt_create_codec_info(audiocodec_get_codec_info(codec_info->module_index));
  
  if(!info_from_module)
    {
    lqt_log(NULL, LQT_LOG_ERROR, LOG_DOMAIN, "Couldn't get codec info for %s from_module %s",
            codec_info->long_name, codec_info->module_filename);
    return;
    }

  if(encode)
    {
    imax = __LQT_MIN(info_from_module->num_encoding_parameters,
                     codec_info->num_encoding_parameters);
    
    for(i = 0; i < imax; i++)
      {
      /* Small check prevents evil bugs in ill conditioned applications */
      
      if(!strcmp(codec_info->encoding_parameters[i].name, 
                 info_from_module->encoding_parameters[i].name))
        {
        copy_parameter_value(&codec_info->encoding_parameters[i].val_default,
                             &info_from_module->encoding_parameters[i].val_default,
                             codec_info->encoding_parameters[i].type);

        }
      }
    }
  
  if(decode)
    {
    imax = __LQT_MIN(info_from_module->num_decoding_parameters,
                     codec_info->num_decoding_parameters);
    
    for(i = 0; i < imax; i++)
      {
      if(!strcmp(codec_info->decoding_parameters[i].name, 
                 info_from_module->decoding_parameters[i].name))
        {
        copy_parameter_value(&codec_info->decoding_parameters[i].val_default,
                             &info_from_module->decoding_parameters[i].val_default,
                             codec_info->decoding_parameters[i].type);

        }
      }
    }

  if(info_from_module)
    lqt_codec_info_destroy_single(info_from_module);
  
  }

/*
 *  Sort audio and video codecs
 */

/*
 * This is the actual sort function: It takes the original chained list
 * of the codecs and a string containing a comma separated list of the
 * codecs as arguments. It returns the newly sorted list.
 *
 * This string will also be saved in the codec file.
 */

static lqt_codec_info_t *
sort_codecs_internal(lqt_codec_info_t * original, char * names)
  {
  char * pos;
  char * end_pos;
  int len;
  lqt_codec_info_t * before;
  lqt_codec_info_t * ptr;
  lqt_codec_info_t * start = original;
  lqt_codec_info_t * ret = (lqt_codec_info_t*)0;
  lqt_codec_info_t * ret_end = (lqt_codec_info_t*)0;
  
  pos = names;

  end_pos = strchr(pos, ',');
  if(!end_pos)
    end_pos = pos + strlen(pos);

  
  while(1)
    {
    /* Seek the codec in the list */
    
    ptr = start;
    before = ptr;
    
    len = end_pos - pos;
        
    while(ptr)
      {
      if(!strncmp(pos, ptr->name, len)) /* Found the codec */
        break;
      before = ptr;
      ptr = ptr->next;
      }

    if(ptr)
      {
      /* Remove codec from the list */

      if(ptr == start)
        start = start->next;
      else
        before->next = ptr->next;
            
      ptr->next = (lqt_codec_info_t*)0;

      /* Append it to the returned list */

      if(!ret)
        {
        ret = ptr;
        ret_end = ret;
        }
      else
        {
        ret_end->next = ptr;
        ret_end = ret_end->next;
        }
      }

    /* Get the next codec name */

    pos = end_pos;

    if(*pos == '\0')
      break;

    pos++;
    end_pos = strchr(pos, ',');
    if(!end_pos)
      end_pos = pos + strlen(pos);
    }

  /* Append the rest of the list */

  if(start)
    ret_end->next = start;

  return ret;
  }

static char * create_seek_string(lqt_codec_info_t ** info)
  {
  int i;

  int num_codecs = 0;
  int string_length = 0;
  char * ret;
  
  while(info[num_codecs])
    {
    string_length += strlen(info[num_codecs]->name) + 1;
    num_codecs++;
    }

  ret = malloc(string_length);
  *ret = '\0';
  
  for(i = 0; i < num_codecs; i++)
    {
    strcat(ret, info[i]->name);
    if(i != num_codecs - 1)
      strcat(ret, ",");
    }
  return ret;
  }

void lqt_reorder_audio_codecs(lqt_codec_info_t ** info)
  {
  char * seek_string = create_seek_string(info);
  lqt_registry_lock();
  lqt_audio_codecs = sort_codecs_internal(lqt_audio_codecs, seek_string);
  lqt_registry_unlock();
  free(seek_string);
  }

void lqt_reorder_video_codecs(lqt_codec_info_t ** info)
  {
  char * seek_string = create_seek_string(info);
  lqt_registry_lock();
  lqt_video_codecs = sort_codecs_internal(lqt_video_codecs, seek_string);
  lqt_registry_unlock();
  free(seek_string);
  }

/***************************************************************
 * This will hopefully make the destruction for dynamic loading
 * (Trick comes from a 1995 version of the ELF Howto, so it
 * should work everywhere now)
 ***************************************************************/

#if defined(__GNUC__)

static void __lqt_cleanup_codecinfo() __attribute__ ((destructor));

static void __lqt_cleanup_codecinfo()
  {
  lqt_registry_destroy();
  }

#endif

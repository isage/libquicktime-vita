/*******************************************************************************
 log.c

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
#include <stdarg.h>
#include <stdio.h>

static struct
  {
  lqt_log_level_t level;
  const char * name;
  }
level_names[] =
  {
    { LQT_LOG_DEBUG,   "Debug" },
    { LQT_LOG_WARNING, "Warning" },
    { LQT_LOG_ERROR,   "Error" },
    { LQT_LOG_INFO,    "Info" },
    { 0,              (char*)0 }
  };


static const char * log_level_to_string(lqt_log_level_t level)
  {
  int index = 0;
  while(level_names[index].name)
    {
    if(level_names[index].level == level)
      return level_names[index].name;
    index++;
    }
  return (char*)0;
  }


static lqt_log_callback_t log_callback;
static void * log_data;
static int log_mask = LQT_LOG_ERROR | LQT_LOG_WARNING;

void lqt_set_log_callback(lqt_log_callback_t cb, void * data)
  {
  log_callback = cb;
  log_data = data;
  }

void lqt_logs(quicktime_t * file, lqt_log_level_t level,
              const char * domain, const char * msg_string)
  {

  if(!file || !file->log_callback)
    {
    if(log_callback)
      log_callback(level, domain, msg_string, log_data);
    else
      fprintf(stderr, "[%s] %s: %s\n",
              domain, log_level_to_string(level), msg_string);
    }
  else
    {
    file->log_callback(level, domain, msg_string, file->log_data);
    }

  }

void lqt_log(quicktime_t * file, lqt_log_level_t level,
             const char * domain, const char * format, ...)
  {
  char * msg_string;
  va_list argp; /* arg ptr */

#ifndef HAVE_VASPRINTF
  int len;
#endif

  if((!file || !file->log_callback) && !log_callback && !(level & log_mask))
    return;
  
  va_start( argp, format);

    
#ifndef HAVE_VASPRINTF
  len = vsnprintf((char*)0, 0, format, argp);
  msg_string = malloc(len+1);
  vsnprintf(msg_string, len+1, format, argp);
#else
  vasprintf(&msg_string, format, argp);
#endif

  va_end(argp);
  lqt_logs(file, level, domain, msg_string);
  free(msg_string);
  }

void lqt_dump(const char * format, ...)
  {
  va_list argp; /* arg ptr */
  va_start( argp, format);
  
  vfprintf(stdout, format, argp);
  
  va_end(argp);
  }

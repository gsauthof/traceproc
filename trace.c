/* {{{

    This file is part of libtraceproc - a library for tracing Pro*C/OCI calls

    Copyright (C) 2013 Georg Sauthoff <mail@georg.so>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

}}} */



#include "trace.h"

#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <time.h>

#include "timespec.h"

struct Trace_State {
  FILE *file;
  char format[32];
  bool last_was_nl;
};
typedef struct Trace_State Trace_State;

static Trace_State trace_state = {0};



void trace_set_file(FILE *file)
{
  trace_state.file = file;
  trace_state.last_was_nl = true;
}

void trace_set_ftime(const char *format)
{
  strncpy(trace_state.format, format, 31);
}


int tprintf(const char *format, ...)
{
  int code = 0;
  int ret_t = 0;
  if (trace_state.last_was_nl && *trace_state.format) {
    struct timespec ts = {0};
    int ret_clock = clock_gettime(CLOCK_REALTIME, &ts);
    char prefix[32] = {0};
    if (ret_clock) {
      fprintf(stderr, "clock_gettime: %s\n", strerror(errno));
      strcpy(prefix, "XXX ");
      code = -1;
    } else {
      size_t ret_strf = timespec_strftime(prefix, 32, trace_state.format, &ts);
      if (!ret_strf) {
        *prefix = 0;
        code = -1;
      }
    }
    ret_t = fprintf(trace_state.file, "%s", prefix);

  }
  va_list args;
  va_start(args, format);
  int ret_f = vfprintf(trace_state.file, format, args);
  va_end(args);

  size_t n = strlen(format);
  if (n)
    --n;
  trace_state.last_was_nl = format[n] == '\n';

  if (code >= 0)
    code = ret_t + ret_f;
  return code;
}

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


#ifndef TIMESPEC_H
#define TIMESPEC_H

#include <time.h>

#if defined(__sun)
#  define WRAP_CLOCK_ID CLOCK_HIGHRES
#elif defined(__linux__)
// this does not work well when forking is involved ...
//#  define WRAP_CLOCK_ID CLOCK_PROCESS_CPUTIME_ID
#  define WRAP_CLOCK_ID CLOCK_REALTIME
#else
#  define WRAP_CLOCK_ID CLOCK_REALTIME
#endif


int timespec_add(struct timespec *res,
    const struct timespec *a);

int timespec_minus(struct timespec *res,
    const struct timespec *a);


int timespec_add_intv(struct timespec *res,
    const struct timespec *begin, const struct timespec *end);

double timespec_cent(const struct timespec *part,
    const struct timespec *sum);

int timespec_pp_intv(const struct timespec *i,
    char *s, size_t n);

size_t timespec_strftime(char *s, size_t max, const char *format,
    const struct timespec *t)
#ifdef __GNUC__
  __attribute__ ((format(strftime, 3, 0)))
#endif
  ;

#endif

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


#ifndef TRACE_H
#define TRACE_H

#include <stdio.h>

void trace_set_file(FILE *file);
void trace_set_ftime(const char *format);

int tprintf(const char *format, ...)
#ifdef __GNUC__
  __attribute__ ((format(printf, 1, 2)))
#endif
  ;

#include <stdbool.h>

// to enable/disable tracing from client
// (when not preloaded)
void traceproc_set_intercept(bool b);
void traceproc_set_oci(bool b);

#endif

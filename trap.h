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


#ifndef TRACEPROC_TRAP_H
#define TRACEPROC_TRAP_H

#include <stdbool.h>

/*
 * Provide single entry-point for all intercepted OCI/Pro*C
 * functions.
 *
 * Is a null-op.
 *
 * Only for easy break pointing ...
 *
 */
void traceproc_trap(
    const char *method,
    const char *stmt,
    bool from_proc,
    bool before
    );

#endif

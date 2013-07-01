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



#ifndef RET_CHECK_H
#define RET_CHECK_H

#include <string.h>
#include <errno.h>
#include <stdio.h>

#define IFTRUERET(a, b, c) \
  do { \
    if (a) { \
      fprintf(stderr, "%s() %s:%d: Error %s\n", __func__, __FILE__, __LINE__, (b) ? (b) : ""); \
      return (c); \
    } \
  } while (0)

#define IFTRUEEXIT(a, b, c) \
  do { \
    if (a) { \
      fprintf(stderr, "%s() %s:%d: Error %s\n", __func__, __FILE__, __LINE__, (b) ? (b) : ""); \
      exit(c); \
    } \
  } while (0)

#define IFNULLRET(a, c) \
  do { \
    if (!a) { \
      fprintf(stderr, "%s() %s:%d: NULL returned\n", __func__, __FILE__, __LINE__); \
      return (c); \
    } \
  } while (0)

#define IFERRNOEXIT(a, b, c) \
  do { \
    if (a) { \
      fprintf(stderr, "%s() %s:%d: Error %s %s\n",\
          __func__, __FILE__, __LINE__,\
          strerror(errno), \
          (b) ? (b) : ""\
          ); \
      exit(c); \
    } \
  } while (0)

#define IFSQLRET(a, b) \
  do { \
    if (sqlca.sqlcode != 0) { \
      proc_error(sqlca.sqlcode, &oraca, (a), __func__, __FILE__, __LINE__); \
      return (b); \
    } \
  } while (0)

#define IFOCIRET(A, B, C) \
  do { \
    if (A) { \
      oci_err((A), (B), OCI_HTYPE_ERROR, __func__, __FILE__, __LINE__); \
      return (C); \
    } \
  } while (0)

#define IFOCIENVRET(A, B, C) \
  do { \
    if (A) { \
      oci_err((A), (B), OCI_HTYPE_ENV, __func__, __FILE__, __LINE__); \
      return (C); \
    } \
  } while (0)

#endif

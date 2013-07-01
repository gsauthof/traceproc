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


#ifndef INTERCEPT_H
#define INTERCEPT_H

#define INTERCEPT_SETUP(A) \
static int setup_ ## A() \
{ \
  /* clear existing error */ \
  dlerror(); \
  /* cast workaround, see linux man page example */ \
  *(void**) (&fn_table. A) = dlsym(RTLD_NEXT, #A); \
  const char *error = dlerror(); \
  if (error) { \
    fprintf(stderr, "dlsym failed: %s\n", error); \
    return -1; \
  } \
  return 0; \
}

  /*
     void *handle = dlopen("libclntsh.so", RTLD_LAZY);
     if (!handle) {
     fprintf(stderr, "dlopen failed: %s\n", dlerror());
     exit(23);
     }
     */

#endif

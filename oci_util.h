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


#ifndef OCI_UTIL_H
#define OCI_UTIL_H

void oci_err(int ret, void *handle, unsigned handle_type,
    const char *func, const char *filename, int line
    );

#include <ociap.h>

int oci_connect(
  OCIEnv **envh,
  OCISvcCtx **sh,
  OCIError **eh
    );
int oci_disconnect(
  OCIEnv *env_handle,
  OCISvcCtx *service_handle,
  OCIError *error_handle
    );

#endif

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


#include <stdio.h>
#include <stdbool.h>

#include <ociap.h>

#include "ret_check.h"
#include "ora_util.h"
#include "oci_util.h"


#include "ocierr.c"


void oci_err(int ret, void *handle, unsigned handle_type,
    const char *func, const char *filename, int line
    )
{
  const Oci_Err *e = oci_errs;
  for (; e->str; ++e) {
    if (e->val == ret) {
      fprintf(stderr, "%s() %s:%d: %s", func, filename, line, e->str);
      if (e->call_error_get) {
        // call multiple times (with increasing no) - until OCI_NO_DATA is returned
        int no = 1;
        int code = 0;
        char buffer[1024] = {0};
        int r = OCIErrorGet(handle, no, 0, &code,
            (OraText*) buffer, 1023, handle_type);
        fprintf(stderr, ": code %d, %s\n", code, buffer);
      } else
        fprintf(stderr, "\n");
      return;
    }
  }
  fprintf(stderr, "%s() %s:%d: Unknown OCI return code: %d",
      func, filename, line, ret);
}


int oci_connect(
  OCIEnv **envh,
  OCISvcCtx **sh,
  OCIError **eh
    )
{
  OCIEnv *oenv = 0;
  int ret = OCIEnvNlsCreate(
      &oenv,
      OCI_DEFAULT, // also bit-orable
      0, // memory callback context
      0, // memory alloc fn
      0, // memory realloc fn
      0, // memory free fn
      0, // user memory to be allocated
      0, // user memory ptr
      0, // use NLS_LANG
      0 // use NLS_CHAR
      );
  IFOCIENVRET(ret, oenv, -1);
  *envh = oenv;

  OCIError *error_handle = 0;
  ret = OCIHandleAlloc(oenv, (void**) &error_handle, OCI_HTYPE_ERROR, 0, 0);
  IFOCIRET(ret, error_handle, -1);
  *eh = error_handle;

  Ora_Connect c = {0};
  ret = ora_init_vars(&c);
  IFTRUERET(ret, 0, -1);

  OCISvcCtx *service_handle = 0;
  // done by Logon2
  //ret = OCIHandleAlloc(&oenv, &service_handle, OCI_HTYPE_SVCCTX, 0, 0);
  //IFOCIRET(ret, error_handle, -1);
  ret = OCILogon2(
      oenv,
      error_handle,
      &service_handle,
      (OraText*) c.username,
      strlen(c.username),
      (OraText*) c.password,
      strlen(c.password),
      (OraText*) c.dbspec,
      strlen(c.dbspec),
      OCI_DEFAULT // flags are also bit-orable
      );
  IFOCIRET(ret, error_handle, -1);
  *sh = service_handle;

  return 0;
}

int oci_disconnect(
  OCIEnv *env_handle,
  OCISvcCtx *service_handle,
  OCIError *error_handle
    )
{
  // not necessary because free of env should be hierachical
  //ret = OCIHandleFree(service_handle, OCI_HTYPE_SVCCTX);

  int ret = OCILogoff(service_handle, error_handle);
  IFOCIRET(ret, error_handle, -1);
  ret = OCIHandleFree(error_handle, OCI_HTYPE_ERROR);
  IFTRUERET(ret, 0, -1);
  ret = OCIHandleFree(env_handle, OCI_HTYPE_ENV);
  IFTRUERET(ret, 0, -1);
  ret = OCITerminate(OCI_DEFAULT);
  IFTRUERET(ret, 0, -1);
  return 0;
}

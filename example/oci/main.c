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



#include <ociap.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <ora_util.h>
#include <ret_check.h>

#include <oci_util.h>


//OCIEnvCreate() or OCIEnvNlsCreate() 
//old:  OCIInitialize() and OCIEnvInit()

// OCILogon2()
// old: ICILogin()
// OCILogoff
// OCIAttrSet
// OCIServerAttach
// OCISessionBegin

// Also OCIStmtPrepare
static int do_select(
  OCIEnv *env_handle,
  OCISvcCtx *service_handle,
  OCIError *error_handle
    )
{
  OCIStmt *stmt_handle = 0;
  const char stmt[] =  "SELECT "
    "to_char("
    //":idate"
    "SYSDATE"
    ", 'YYYY-MM-DD HH24:MI:SS')"
    " FROM DUAL";
  int ret = OCIStmtPrepare2(
      service_handle,
      &stmt_handle,
      error_handle,
      (const OraText*)stmt,
      strlen(stmt),
      0,
      0,
      OCI_NTV_SYNTAX,
      OCI_DEFAULT);
  IFOCIRET(ret, error_handle, -1);

  char value[32] = {0};

  OCIDefine *define_handle = 0; //  implicitly free'd?
  sb2 indicator = 0;
  ub2 length = 0;
  ret = OCIDefineByPos(stmt_handle, &define_handle,
      error_handle,
      1,
      value,
      32,
      SQLT_STR,
      &indicator,
      &length,
      0, // col level ret array
      OCI_DEFAULT
      );
  IFOCIRET(ret, error_handle, -1);

  ret = OCIStmtExecute(service_handle, stmt_handle, error_handle,
      1,
      0,
      0,
      0,
      OCI_DEFAULT
      );
  IFOCIRET(ret, error_handle, -1);

  fprintf(stderr, "XXX: |%s|=%zu, length=%u\n", value, strlen(value), length);


  ret = OCIStmtRelease(stmt_handle, error_handle, 0, 0, OCI_DEFAULT);
  IFOCIRET(ret, error_handle, -1);
  return 0;
}

// use native OCIDateTime type for result
static int do_select2(
  OCIEnv *env_handle,
  OCISvcCtx *service_handle,
  OCIError *error_handle
    )
{
  OCIStmt *stmt_handle = 0;
  const char stmt[] =  "SELECT "
    //"to_char("
    //":idate"
    "SYSDATE"
    //"to_date('1897-03-04 23:22:21', 'YYYY-MM-DD HH24:MI:SS') "
    //", 'YYYY-MM-DD HH24:MI:SS')"
    " FROM DUAL";
  int ret = OCIStmtPrepare2(
      service_handle,
      &stmt_handle,
      error_handle,
      (const OraText*)stmt,
      strlen(stmt),
      0,
      0,
      OCI_NTV_SYNTAX,
      OCI_DEFAULT);
  IFOCIRET(ret, error_handle, -1);

  OCIDateTime *value_date = 0;
  ret = OCIDescriptorAlloc(env_handle, (void**) &value_date, OCI_DTYPE_TIMESTAMP, 0, 0);
  IFOCIRET(ret, error_handle, -1);
  OCIDefine *define_handle = 0; //  implicitly free'd?
  sb2 indicator = 0;
  ub2 length = 0;
  ret = OCIDefineByPos(stmt_handle, &define_handle,
      error_handle,
      1,
      &value_date,
      sizeof(OCIDateTime*),
      SQLT_TIMESTAMP,
      &indicator,
      &length,
      0, // col level ret array
      OCI_DEFAULT
      );
  IFOCIRET(ret, error_handle, -1);

  ret = OCIStmtExecute(service_handle, stmt_handle, error_handle,
      1,
      0,
      0,
      0,
      OCI_DEFAULT
      );
  IFOCIRET(ret, error_handle, -1);

  char dbuffer[32] = {0};
  ub4 dbuffer_size = 31;
  // SQLT_DATE does not support HH:MI:SS !
  const char dformat[] = "YYYY-MM-DD HH24:MI:SS";
  //const char dformat[] = "YYYY-MM-DD";
  ret = OCIDateTimeToText(env_handle, error_handle,
      value_date,
      (const OraText*)dformat,
      strlen(dformat),
      0,
      0,
      0,
      &dbuffer_size,
      (OraText*)dbuffer);
  IFOCIRET(ret, error_handle, -1);
  fprintf(stderr, "DBUFFER: |%s|\n", dbuffer);

  OCIInterval *interval = 0;
  ret = OCIDescriptorAlloc(env_handle, (void**) &interval, OCI_DTYPE_INTERVAL_YM, 0, 0);
  IFOCIRET(ret, error_handle, -1);
  ub1 darray[32] = {0};
  ub4 darray_size = 31;
  ret = OCIDateTimeToArray(env_handle, error_handle,
      value_date,
      interval, //0,
      darray,
      &darray_size,
      0
      );
  IFOCIRET(ret, error_handle, -1);
  fprintf(stderr, "darray_size=%u\n", darray_size);
  for (unsigned i = 0; i<darray_size; ++i)
    fprintf(stderr, "%u ", darray[i]);
  fprintf(stderr, "\n");

  ret = OCIDescriptorFree(value_date, OCI_DTYPE_TIMESTAMP);
  IFOCIRET(ret, error_handle, -1);
  ret = OCIDescriptorFree(interval, OCI_DTYPE_INTERVAL_YM);
  IFOCIRET(ret, error_handle, -1);

  ret = OCIStmtRelease(stmt_handle, error_handle, 0, 0, OCI_DEFAULT);
  IFOCIRET(ret, error_handle, -1);
  return 0;
}

// use bind
static int do_select3(
  OCIEnv *env_handle,
  OCISvcCtx *service_handle,
  OCIError *error_handle
    )
{
  OCIStmt *stmt_handle = 0;
  const char stmt[] =  "SELECT "
    //"to_char("
    ":var1"
    //"SYSDATE"
    //"to_date('1897-03-04 23:22:21', 'YYYY-MM-DD HH24:MI:SS') "
    //", 'YYYY-MM-DD HH24:MI:SS')"
    " FROM DUAL";
  int ret = OCIStmtPrepare2(
      service_handle,
      &stmt_handle,
      error_handle,
      (const OraText*)stmt,
      strlen(stmt),
      0,
      0,
      OCI_NTV_SYNTAX,
      OCI_DEFAULT);
  IFOCIRET(ret, error_handle, -1);

  OCIBind *bind_handle = 0; // should be implicitly free'd by OCIStmtRelease
  OCIDateTime *date = 0;
  ret = OCIDescriptorAlloc(env_handle, (void**) &date, OCI_DTYPE_TIMESTAMP, 0, 0);
  IFOCIRET(ret, error_handle, -1);
  ret = OCIDateTimeConstruct(env_handle, error_handle,
      date, 2011, 11, 23, 13, 14, 15, 0, 0, 0);
  IFOCIRET(ret, error_handle, -1);
  sb2 date_ind = 0;
  ret = OCIBindByPos(
      stmt_handle,
      &bind_handle,
      error_handle,
      1,
      &date,
      sizeof(OCIDateTime*),
      SQLT_TIMESTAMP, //SQLT_STR, // SQLT_CHR, // not-null-term
      &date_ind, // (void*)0, // OCI_IND_NULL
      0, // len array
      0, // col level ret code array,
      0, // max array len
      0, // cur arr len
      OCI_DEFAULT
      );
  IFOCIRET(ret, error_handle, -1);
  char buffer[32] = {0};
  ub4 buffer_size = 31;
  const char dformat[] = "YYYY-MM-DD HH24:MI:SS";
  //const char dformat[] = "YYYY-MM-DD";
  ret = OCIDateTimeToText(env_handle, error_handle,
      date,
      (const OraText*)dformat,
      strlen(dformat),
      0,
      0,
      0,
      &buffer_size,
      (OraText*)buffer);
  IFOCIRET(ret, error_handle, -1);
  fprintf(stderr, "bind val: |%s|\n", buffer);

  OCIDateTime *value_date = 0;
  ret = OCIDescriptorAlloc(env_handle, (void**) &value_date, OCI_DTYPE_TIMESTAMP, 0, 0);
  IFOCIRET(ret, error_handle, -1);
  OCIDefine *define_handle = 0; //  implicitly free'd?
  sb2 indicator = 0;
  ub2 length = 0;
  ret = OCIDefineByPos(stmt_handle, &define_handle,
      error_handle,
      1,
      &value_date,
      sizeof(OCIDateTime*),
      SQLT_TIMESTAMP,
      &indicator,
      &length,
      0, // col level ret array
      OCI_DEFAULT
      );
  IFOCIRET(ret, error_handle, -1);

  ret = OCIStmtExecute(service_handle, stmt_handle, error_handle,
      1,
      0,
      0,
      0,
      OCI_DEFAULT
      );
  IFOCIRET(ret, error_handle, -1);

  char buffer2[32] = {0};
  ub4 buffer2_size = 31;
  // SQLT_DATE does not support HH:MI:SS !
  const char dformat2[] = "YYYY-MM-DD HH24:MI:SS";
  //const char dformat[] = "YYYY-MM-DD";
  ret = OCIDateTimeToText(env_handle, error_handle,
      value_date,
      (const OraText*)dformat2,
      strlen(dformat2),
      0,
      0,
      0,
      &buffer2_size,
      (OraText*)buffer2);
  IFOCIRET(ret, error_handle, -1);
  fprintf(stderr, "DBUFFER: |%s|\n", buffer2);

  OCIInterval *interval = 0;
  ret = OCIDescriptorAlloc(env_handle, (void**) &interval, OCI_DTYPE_INTERVAL_YM, 0, 0);
  IFOCIRET(ret, error_handle, -1);
  ub1 darray[32] = {0};
  ub4 darray_size = 31;
  ret = OCIDateTimeToArray(env_handle, error_handle,
      value_date,
      interval, //0,
      darray,
      &darray_size,
      0
      );
  IFOCIRET(ret, error_handle, -1);
  fprintf(stderr, "darray_size=%u\n", darray_size);
  for (unsigned i = 0; i<darray_size; ++i)
    fprintf(stderr, "%u ", darray[i]);
  fprintf(stderr, "\n");

  ret = OCIDescriptorFree(date, OCI_DTYPE_TIMESTAMP);
  IFOCIRET(ret, error_handle, -1);
  ret = OCIDescriptorFree(value_date, OCI_DTYPE_TIMESTAMP);
  IFOCIRET(ret, error_handle, -1);
  ret = OCIDescriptorFree(interval, OCI_DTYPE_INTERVAL_YM);
  IFOCIRET(ret, error_handle, -1);

  ret = OCIStmtRelease(stmt_handle, error_handle, 0, 0, OCI_DEFAULT);
  IFOCIRET(ret, error_handle, -1);
  return 0;
}

int main(int argc, char **argv)
{
  // XXX check if signal handler are installed by logon and friends
  OCIEnv *env_handle = 0;
  OCISvcCtx *service_handle = 0;
  OCIError *error_handle = 0;
  ora_print_signals("OCI before");
  int ret = oci_connect(&env_handle, &service_handle, &error_handle);
  IFTRUERET(ret, 0, 1);
  ora_print_signals("OCI after");
  ora_reset_signals();

  ret = do_select(env_handle, service_handle, error_handle);
  IFTRUERET(ret, 0, 1);
  ret = do_select2(env_handle, service_handle, error_handle);
  IFTRUERET(ret, 0, 1);
  ret = do_select3(env_handle, service_handle, error_handle);
  IFTRUERET(ret, 0, 1);

  ret = oci_disconnect(env_handle, service_handle, error_handle);
  IFTRUERET(ret, 0, 1);
  return 0;
}


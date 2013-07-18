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


#if defined(__linux__)
#define _GNU_SOURCE
#endif
#include <dlfcn.h>

#include "ocitrace.h"

#include <stdbool.h>
#include <stdlib.h>
#include <search.h>
#include <time.h>

#include <ociap.h>

#include "ret_check.h"
#include "intercept.h"
#include "trace.h"
#include "timespec.h"
#include "stats.h"
#include "trap.h"

// XXX change STATS_BEGIN such that counts are incremented before

static bool intercept = false;
static bool gory = false;
static bool sql = false;
static bool count = false;
static bool leak_check = false;

struct Fn_Table {
  sword (*OCIEnvInit)(OCIEnv **envp, ub4 mode,
                    size_t xtramem_sz, void  **usrmempp);
  sword (*OCIEnvCreate)(OCIEnv **envp, ub4 mode, void  *ctxp,
      void  *(*malocfp)(void  *ctxp, size_t size),
      void  *(*ralocfp)(void  *ctxp, void  *memptr, size_t newsize),
      void   (*mfreefp)(void  *ctxp, void  *memptr), size_t xtramem_sz,
      void  **usrmempp);
  sword (*OCIEnvNlsCreate)(OCIEnv **envp, ub4 mode, void  *ctxp,
      void  *(*malocfp)(void  *ctxp, size_t size),
      void  *(*ralocfp)(void  *ctxp, void  *memptr, size_t newsize),
      void   (*mfreefp)(void  *ctxp, void  *memptr), size_t xtramem_sz,
      void  **usrmempp, ub2 charset, ub2 ncharset);
  sword   (*OCILogon)(OCIEnv *envhp, OCIError *errhp, OCISvcCtx **svchp,
      const OraText *username, ub4 uname_len, const OraText *password,
      ub4 passwd_len, const OraText *dbname, ub4 dbname_len);
  sword  (*OCILogon2)(OCIEnv *envhp, OCIError *errhp, OCISvcCtx **svchp,
      const OraText *username, ub4 uname_len, const OraText *password,
      ub4 passwd_len, const OraText *dbname, ub4 dbname_len, ub4 mode);
  sword (*OCIStmtPrepare)(OCIStmt *stmtp, OCIError *errhp,
      const OraText *stmt, ub4 stmt_len, ub4 language, ub4 mode);
  sword (*OCIStmtPrepare2)(OCISvcCtx *svchp, OCIStmt **stmtp,
      OCIError *errhp, const OraText *stmt, ub4 stmt_len,
      const OraText *key, ub4 key_len, ub4 language, ub4 mode);
  sword (*OCIBindByPos)(OCIStmt *stmtp, OCIBind **bindp, OCIError *errhp,
      ub4 position, void  *valuep, sb4 value_sz,
      ub2 dty, void  *indp, ub2 *alenp, ub2 *rcodep,
      ub4 maxarr_len, ub4 *curelep, ub4 mode);
  sword (*OCIBindByName)(OCIStmt *stmtp, OCIBind **bindp, OCIError *errhp,
      const OraText *placeholder, sb4 placeh_len,
      void  *valuep, sb4 value_sz, ub2 dty,
      void  *indp, ub2 *alenp, ub2 *rcodep,
      ub4 maxarr_len, ub4 *curelep, ub4 mode);
  sword (*OCIDefineByPos)(OCIStmt *stmtp, OCIDefine **defnp,
      OCIError *errhp,
      ub4 position, void  *valuep, sb4 value_sz, ub2 dty,
      void  *indp, ub2 *rlenp, ub2 *rcodep, ub4 mode);
  sword (*OCITerminate)(ub4 mode);
  sword (*OCIStmtExecute)(OCISvcCtx *svchp, OCIStmt *stmtp,
      OCIError *errhp, ub4 iters, ub4 rowoff, const OCISnapshot *snap_in,
      OCISnapshot *snap_out, ub4 mode);
  sword (*OCIStmtFetch)(OCIStmt *stmtp, OCIError *errhp, ub4 nrows,
      ub2 orientation, ub4 mode);
  sword (*OCIStmtFetch2)(OCIStmt *stmtp, OCIError *errhp, ub4 nrows,
      ub2 orientation, sb4 scrollOffset, ub4 mode);
  sword (*OCITransCommit)(OCISvcCtx *svchp, OCIError *errhp, ub4 flags);
  sword (*OCIHandleAlloc)(const void  *parenth, void  **hndlpp,
      const ub4 type, const size_t xtramem_sz, void  **usrmempp);
  sword (*OCIHandleFree)(void  *hndlp, const ub4 type);
  sword (*OCIDescriptorAlloc)(const void  *parenth, void  **descpp,
      const ub4 type, const size_t xtramem_sz, void  **usrmempp);
  sword (*OCIDescriptorFree)(void  *descp, const ub4 type);
  sword (*OCIArrayDescriptorAlloc)(const void *parenth, void **descpp,
      const ub4 type, ub4 array_size,
      const size_t xtramem_sz, void  **usrmempp);
  sword (*OCIArrayDescriptorFree)(void **descp, const ub4 type);
  sword (*OCISessionBegin)(OCISvcCtx *svchp, OCIError *errhp,
      OCISession *usrhp, ub4 credt, ub4 mode);
  sword (*OCISessionEnd)(OCISvcCtx *svchp, OCIError *errhp,
      OCISession *usrhp, ub4 mode);
  sword (*OCIAttrGet)(const void  *trgthndlp, ub4 trghndltyp,
      void  *attributep, ub4 *sizep, ub4 attrtype,
      OCIError *errhp);
  sword (*OCIAttrSet)(void  *trgthndlp, ub4 trghndltyp, void  *attributep,
      ub4 size, ub4 attrtype, OCIError *errhp);
  sword (*OCIServerAttach)(OCIServer *srvhp, OCIError *errhp,
      const OraText *dblink, sb4 dblink_len, ub4 mode);
  sword (*OCIServerDetach)(OCIServer *srvhp, OCIError *errhp, ub4 mode);
  sword (*OCIInitialize)(ub4 mode, void  *ctxp, 
                 void  *(*malocfp)(void  *ctxp, size_t size),
                 void  *(*ralocfp)(void  *ctxp, void  *memptr, size_t newsize),
                 void   (*mfreefp)(void  *ctxp, void  *memptr) );
  sword (*OCILobWrite)(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,
      ub4 *amtp, ub4 offset, void  *bufp, ub4 buflen,
      ub1 piece,  void  *ctxp, OCICallbackLobWrite cbfp,
      ub2 csid, ub1 csfrm);
  sword (*OCIParamGet)(const void  *hndlp, ub4 htype, OCIError *errhp, 
      void  **parmdpp, ub4 pos);
  sword (* OCIParamSet)(void  *hdlp, ub4 htyp, OCIError *errhp, const void  *dscp,
      ub4 dtyp, ub4 pos);
  sword (*OCITransStart)(OCISvcCtx *svchp, OCIError *errhp, 
      uword timeout, ub4 flags );
  sword (*OCITransRollback)(OCISvcCtx *svchp, OCIError *errhp, ub4 flags);
  sword (*OCISessionPoolCreate)(OCIEnv *envhp, OCIError *errhp, OCISPool *spoolhp, 
        OraText **poolName, ub4 *poolNameLen, 
        const OraText *connStr, ub4 connStrLen,
        ub4 sessMin, ub4 sessMax, ub4 sessIncr,
        OraText *userid, ub4 useridLen,
        OraText *password, ub4 passwordLen,
        ub4 mode);
  sword (*OCISessionPoolDestroy)(OCISPool *spoolhp,
        OCIError *errhp,
        ub4 mode);
  sword (*OCISessionGet)(OCIEnv *envhp, OCIError *errhp, OCISvcCtx **svchp,
        OCIAuthInfo *authhp,
        OraText *poolName, ub4 poolName_len, 
        const OraText *tagInfo, ub4 tagInfo_len,
        OraText **retTagInfo, ub4 *retTagInfo_len,
        boolean *found, ub4 mode);
  sword (*OCISessionRelease)(OCISvcCtx *svchp, OCIError *errhp,
        OraText *tag, ub4 tag_len,
        ub4 mode);
};
typedef struct Fn_Table Fn_Table;

static Fn_Table fn_table = {0};

enum Func {
  FN_OCIENVINIT,
  FN_OCIENVCREATE,
  FN_OCIENVNLSCREATE,
  FN_OCILOGON,
  FN_OCILOGON2,
  FN_OCISTMTPREPARE,
  FN_OCISTMTPREPARE2,
  FN_OCIBINDBYPOS,
  FN_OCIBINDBYNAME,
  FN_OCIDEFINEBYPOS,
  FN_OCITERMINATE,
  FN_OCISTMTEXECUTE,
  FN_OCISTMTFETCH,
  FN_OCISTMTFETCH2,
  FN_OCITRANSCOMMIT,
  FN_OCIHANDLEALLOC,
  FN_OCIHANDLEFREE,
  FN_OCIDESCRIPTORALLOC,
  FN_OCIDESCRIPTORFREE,
  FN_OCIARRAYDESCRIPTORALLOC,
  FN_OCIARRAYDESCRIPTORFREE,
  FN_OCISESSIONBEGIN,
  FN_OCISESSIONEND,
  FN_OCIATTRGET,
  FN_OCIATTRSET,
  FN_OCISERVERATTACH,
  FN_OCISERVERDETACH,
  FN_OCIINITIALIZE,
  FN_OCILOBWRITE,
  FN_OCIPARAMGET,
  FN_OCIPARAMSET,
  FN_OCITRANSSTART,
  FN_OCITRANSROLLBACK,
  FN_OCISESSIONGET,
  FN_OCISESSIONPOOLCREATE,
  FN_OCISESSIONPOOLDESTROY,
  FN_OCISESSIONRELEASE,
  FN_SIZE
};
typedef enum Func Func;
static const char *func_str[] = {
  "OCIEnvInit",
  "OCIEnvCreate",
  "OCIEnvNlsCreate",
  "OCILogon",
  "OCILogon2",
  "OCIStmtPrepare",
  "OCIStmtPrepare2",
  "OCIBindByPos",
  "OCIBindByName",
  "OCIDefineByPos",
  "OCITerminate",
  "OCIStmtExecute",
  "OCIStmtFetch",
  "OCIStmtFetch2",
  "OCITransCommit",
  "OCIHandleAlloc",
  "OCIHandleFree",
  "OCIDescriptorAlloc",
  "OCIDescriptorFree",
  "OCIArrayDescriptorAlloc",
  "OCIArrayDescriptorFree",
  "OCISessionBegin",
  "OCISessionEnd",
  "OCIAttrGet",
  "OCIAttrSet",
  "OCIServerAttach",
  "OCIServerDetach",
  "OCIInitialize",
  "OCILobWrite",
  "OCIParamGet",
  "OCIParamSet",
  "OCITransStart",
  "OCITransRollback",
  "OCISessionEnd",
  "OCISessionGet",
  "OCISessionPoolCreate",
  "OCISessionPoolDestroy",
  "OCISessionRelease",
  0
};

static Stats stats = {0};

#include "ocierr.c"

static void print_retcode(int rc)
{
  if (!gory)
    return;
  for (const Oci_Err *e = oci_errs;e->str; ++e) {
    if (e->val == rc) {
      tprintf(" => %s (%d)\n", e->str, rc);
      return;
    }
  }
  tprintf(" => ERROR: unknown ret code (%d)\n", rc);
}

struct Flag_Str {
  unsigned flag;
  char *str;
};
typedef struct Flag_Str Flag_Str;

int flags2str(unsigned flags, const Flag_Str *a,
    char *buffer, size_t buffer_size)
{
  if (!buffer_size)
    return -1;
  if (flags == OCI_DEFAULT) {
    strncpy(buffer, "  OCI_DEFAULT", buffer_size-1);
    return 0;
  }
  size_t written = 0;
  for (const Flag_Str *x = a; x->str; ++x) {
    if (flags & x->flag) {
      written += strlen(x->str) + 3;
      if (written > buffer_size)
        return -2;
      strcat(buffer, "| ");
      strcat(buffer, x->str);
    }
  }
  return 0;
}

const char *flag2str(unsigned flag, const Flag_Str *a)
{
  for (const Flag_Str *x = a; x->str; ++x) {
    if (flag == x->flag) {
      return x->str;
    }
  }
  return "FLAGNOTFOUND";
}

sword OCIEnvInit(OCIEnv **envp, ub4 mode, size_t xtramem_sz,
    void  **usrmempp)
{
  char buffer[128] = {0};
  static const Flag_Str mode_str[] = {
    //{ OCI_DEFAULT, "OCI_DEFAULT" },
    { OCI_ENV_NO_MUTEX, "OCI_ENV_NO_MUTEX" },
    { OCI_ENV_NO_UCB, "OCI_ENV_NO_UCB" },
    { 0, 0}
  };
  if (gory) {
    flags2str(mode, mode_str, buffer, 128);
    tprintf("OCIEnvInit(%s)", buffer+2);
  }

  traceproc_trap("OCIEnvInit", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIEnvInit)(envp, mode, xtramem_sz, usrmempp);

  STATS_END(count, FN_OCIENVINIT, 0);
  traceproc_trap("OCIEnvInit", "", false, false);

  print_retcode(ret);
  return ret;
}

  static const Flag_Str env_mode_str[] = {
    //{ OCI_DEFAULT, "OCI_DEFAULT" },
    { OCI_THREADED, "OCI_THREADED" },
    { OCI_OBJECT, "OCI_OBJECT" },
    { OCI_EVENTS, "OCI_EVENTS" },
    { OCI_NO_UCB, "OCI_NO_UCB" },
    { OCI_NO_MUTEX, "OCI_NO_MUTEX" },
    { OCI_NEW_LENGTH_SEMANTICS, "OCI_NEW_LENGTH_SEMANTICS" },
    { OCI_SUPPRESS_NLS_VALIDATION, "OCI_SUPPRESS_NLS_VALIDATION" },
    { OCI_NCHAR_LITERAL_REPLACE_ON, "OCI_NCHAR_LITERAL_REPLACE_ON" },
    { OCI_NCHAR_LITERAL_REPLACE_OFF, "OCI_NCHAR_LITERAL_REPLACE_OFF" },
    { OCI_ENABLE_NLS_VALIDATION, "OCI_ENABLE_NLS_VALIDATION" },
    { 0, 0}
  };

sword OCIEnvCreate(OCIEnv **envp, ub4 mode, void  *ctxp,
    void  *(*malocfp)(void  *ctxp, size_t size),
    void  *(*ralocfp)(void  *ctxp, void  *memptr, size_t newsize),
    void   (*mfreefp)(void  *ctxp, void  *memptr), size_t xtramem_sz,
    void  **usrmempp)
{
  char buffer[128] = {0};
  if (gory) {
    flags2str(mode, env_mode_str, buffer, 128);
    tprintf("OCIEnvCreate(%s)", buffer+2);
  }

  traceproc_trap("OCIEnvCreate", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIEnvCreate)(envp, mode, ctxp,
      malocfp,
      ralocfp,
      mfreefp, xtramem_sz,
      usrmempp);

  STATS_END(count, FN_OCIENVCREATE, 0);
  traceproc_trap("OCIEnvCreate", "", false, false);

  print_retcode(ret);
  return ret;
}

sword OCIEnvNlsCreate(OCIEnv **envp, ub4 mode, void  *ctxp,
    void  *(*malocfp)(void  *ctxp, size_t size),
    void  *(*ralocfp)(void  *ctxp, void  *memptr, size_t newsize),
    void   (*mfreefp)(void  *ctxp, void  *memptr), size_t xtramem_sz,
    void  **usrmempp, ub2 charset, ub2 ncharset)
{
  char buffer[128] = {0};
  if (gory) {
    flags2str(mode, env_mode_str, buffer, 128);
    tprintf("OCIEnvCreate(%s)", buffer+2);
  }

  traceproc_trap("OCIEnvNlsCreate", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIEnvNlsCreate)(envp, mode, ctxp,
      malocfp,
      ralocfp,
      mfreefp, xtramem_sz,
      usrmempp, charset, ncharset);

  STATS_END(count, FN_OCIENVNLSCREATE, 0);
  traceproc_trap("OCIEnvNlsCreate", "", false, false);

  print_retcode(ret);
  return ret;
}

sword OCILogon(OCIEnv *envhp, OCIError *errhp, OCISvcCtx **svchp,
    const OraText *username, ub4 uname_len, const OraText *password,
    ub4 passwd_len, const OraText *dbname, ub4 dbname_len)
{
  if (gory) {
    tprintf("OCILogon(%.*s, %.*s, %.*s) ", uname_len, username,
        passwd_len, password, dbname_len, dbname);
  }

  traceproc_trap("OCILogon", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCILogon)(envhp, errhp, svchp,
      username, uname_len, password,
      passwd_len, dbname, dbname_len);

  STATS_END(count, FN_OCILOGON, 0);
  traceproc_trap("OCILogon", "", false, false);

  print_retcode(ret);
  return ret;
}

sword OCILogon2(OCIEnv *envhp, OCIError *errhp, OCISvcCtx **svchp,
    const OraText *username, ub4 uname_len, const OraText *password,
    ub4 passwd_len, const OraText *dbname, ub4 dbname_len, ub4 mode)
{
  if (gory) {
    tprintf("OCILogon2(%.*s, %.*s, %.*s) ", uname_len, username,
        passwd_len, password, dbname_len, dbname);
  }

  traceproc_trap("OCILogon2", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCILogon2)(envhp, errhp, svchp,
      username, uname_len, password,
      passwd_len, dbname, dbname_len, mode);

  STATS_END(count, FN_OCILOGON2, 0);
  traceproc_trap("OCILogon2", "", false, false);

  print_retcode(ret);
  return ret;
}

sword OCIStmtPrepare(OCIStmt *stmtp, OCIError *errhp, const OraText *stmt,
    ub4 stmt_len, ub4 language, ub4 mode)
{
  char buffer[128] = {0};
  if (gory) {
    flags2str(mode, env_mode_str, buffer, 128);
    tprintf("OCIStmtPrepare(%.*s)", stmt_len, stmt);
  }

  traceproc_trap("OCIStmtPrepare", (const char*)stmt, false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIStmtPrepare)(stmtp, errhp, stmt,
      stmt_len, language, mode);

  STATS_END(count, FN_OCISTMTPREPARE, 0);
  traceproc_trap("OCIStmtPrepare", (const char*)stmt, false, false);

  print_retcode(ret);
  return ret;
}

sword OCIStmtPrepare2(OCISvcCtx *svchp, OCIStmt **stmtp, OCIError *errhp,
    const OraText *stmt, ub4 stmt_len, const OraText *key, ub4 key_len,
    ub4 language, ub4 mode)
{
  char buffer[128] = {0};
  if (gory) {
    flags2str(mode, env_mode_str, buffer, 128);
    tprintf("OCIStmtPrepare2(%.*s, %s, key=%.*s)", stmt_len, stmt,
        buffer + 2,
        key ? key_len : 0,
        key? (const char*) key : ""
        );
  }

  traceproc_trap("OCIStmtPrepare2", (const char*)stmt, false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIStmtPrepare2)(svchp, stmtp, errhp,
      stmt, stmt_len, key, key_len,
      language, mode);

  STATS_END(count, FN_OCISTMTPREPARE2, 0);
  traceproc_trap("OCIStmtPrepare2", (const char*)stmt, false, false);

  print_retcode(ret);
  return ret;
}

  const Flag_Str bind_mode_str[] = {
    { OCI_DEFAULT, "OCI_DEFAULT" },
    { OCI_BIND_SOFT, "OCI_BIND_SOFT" },
    { OCI_DATA_AT_EXEC, "OCI_DATA_AT_EXEC" },
    { OCI_IOV, "OCI_IOV" },
    { 0, 0 }
  };

sword OCIBindByPos(OCIStmt *stmtp, OCIBind **bindp, OCIError *errhp,
    ub4 position, void  *valuep, sb4 value_sz, ub2 dty, void  *indp,
    ub2 *alenp, ub2 *rcodep, ub4 maxarr_len, ub4 *curelep, ub4 mode)
{
  if (gory)
    tprintf("OCIBindByPos(pos=%2u, %s)", position,
        flag2str(mode, bind_mode_str));

  traceproc_trap("OCIBindByPos", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIBindByPos)(stmtp, bindp, errhp,
      position, valuep, value_sz, dty, indp,
      alenp, rcodep, maxarr_len, curelep, mode);

  STATS_END(count, FN_OCIBINDBYPOS, 0);
  traceproc_trap("OCIBindByPos", "", false, false);

  print_retcode(ret);
  return ret;
}



sword OCIBindByName(OCIStmt *stmtp, OCIBind **bindp, OCIError *errhp,
    const OraText *placeholder, sb4 placeh_len, void  *valuep,
    sb4 value_sz, ub2 dty, void  *indp, ub2 *alenp, ub2 *rcodep,
    ub4 maxarr_len, ub4 *curelep, ub4 mode)
{
  if (gory)
    tprintf("OCIBindByName(name=%s, %s)", placeholder,
        flag2str(mode, bind_mode_str));

  traceproc_trap("OCIBindByName", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIBindByName)(stmtp, bindp, errhp,
      placeholder, placeh_len, valuep,
      value_sz, dty, indp, alenp, rcodep,
      maxarr_len, curelep, mode);

  STATS_END(count, FN_OCIBINDBYNAME, 0);
  traceproc_trap("OCIBindByName", "", false, false);

  print_retcode(ret);
  return ret;
}

sword OCIDefineByPos(OCIStmt *stmtp, OCIDefine **defnp, OCIError *errhp,
    ub4 position, void  *valuep, sb4 value_sz, ub2 dty,
    void  *indp, ub2 *rlenp, ub2 *rcodep, ub4 mode)
{
  static const Flag_Str mode_str[] = {
    { OCI_DEFAULT, "OCI_DEFAULT" },
    { OCI_DEFINE_SOFT, "OCI_DEFINE_SOFT" },
    { OCI_DYNAMIC_FETCH, "OCI_DYNAMIC_FETCH" },
    { OCI_IOV, "OCI_IOV" },
    { 0, 0 }
  };
  if (gory)
    tprintf("OCIDefineByPos(pos=%2u, %s)", position,
        flag2str(mode, mode_str)
        );

  traceproc_trap("OCIDefineByPos", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIDefineByPos)(stmtp, defnp, errhp,
      position, valuep, value_sz, dty,
      indp, rlenp, rcodep, mode);

  STATS_END(count, FN_OCIDEFINEBYPOS, 0);
  traceproc_trap("OCIDefineByPos", "", false, false);

  print_retcode(ret);
  return ret;
}

sword OCITerminate(ub4 mode)
{
  if (gory)
    tprintf("OCITerminate()");

  traceproc_trap("OCITerminate", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCITerminate)(mode);

  STATS_END(count, FN_OCITERMINATE, 0);
  traceproc_trap("OCITerminate", "", false, false);

  print_retcode(ret);
  return ret;
}

sword OCIStmtExecute(OCISvcCtx *svchp, OCIStmt *stmtp, OCIError *errhp, 
  ub4 iters, ub4 rowoff, const OCISnapshot *snap_in, 
  OCISnapshot *snap_out, ub4 mode)
{
  static const Flag_Str mode_str[] = {
    { OCI_BATCH_ERRORS, "OCI_BATCH_ERRORS" },
    { OCI_COMMIT_ON_SUCCESS, "OCI_COMMIT_ON_SUCCESS" },
    { OCI_DEFAULT, "OCI_DEFAULT" },
    { OCI_DESCRIBE_ONLY, "OCI_DESCRIBE_ONLY" },
    { OCI_EXACT_FETCH, "OCI_EXACT_FETCH" },
    { OCI_PARSE_ONLY, "OCI_PARSE_ONLY" },
    { OCI_STMT_SCROLLABLE_READONLY, "OCI_STMT_SCROLLABLE_READONLY" },
    { 0, 0 }
  };
  if (gory)
    tprintf("OCIStmtExecute(iters=%u, rowoff=%u, %s)", iters, rowoff,
        flag2str(mode, mode_str));

  traceproc_trap("OCIStmtExecute", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIStmtExecute)(svchp, stmtp, errhp,
        iters, rowoff, snap_in,
        snap_out, mode);

  STATS_END(count, FN_OCISTMTEXECUTE, 0);
  traceproc_trap("OCIStmtExecute", "", false, false);

  print_retcode(ret);
  return ret;
}

static const Flag_Str orient_str[] = {
    { OCI_DEFAULT, "OCI_DEFAULT" },
    { OCI_FETCH_CURRENT, "OCI_FETCH_CURRENT" },
    { OCI_FETCH_NEXT, "OCI_FETCH_NEXT" },
    { OCI_FETCH_FIRST, "OCI_FETCH_FIRST" },
    { OCI_FETCH_LAST, "OCI_FETCH_LAST" },
    { OCI_FETCH_PRIOR, "OCI_FETCH_PRIOR" },
    { OCI_FETCH_ABSOLUTE, "OCI_FETCH_ABSOLUTE" },
    { OCI_FETCH_RELATIVE, "OCI_FETCH_RELATIVE" },
    { 0, 0}
};

sword OCIStmtFetch(OCIStmt *stmtp, OCIError *errhp, ub4 nrows, 
    ub2 orientation, ub4 mode)
{
  if (gory)
    tprintf("OCIStmtFetch(rows=%u, %s)", nrows,
        flag2str(orientation, orient_str));

  traceproc_trap("OCIStmtFetch", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIStmtFetch)(stmtp, errhp, nrows,
        orientation, mode);

  STATS_END(count, FN_OCISTMTFETCH, 0);
  traceproc_trap("OCIStmtFetch", "", false, false);

  print_retcode(ret);
  return ret;
}

sword OCIStmtFetch2(OCIStmt *stmtp, OCIError *errhp, ub4 nrows, 
    ub2 orientation, sb4 scrollOffset, ub4 mode)
{
  if (gory)
    tprintf("OCIStmtFetch2(rows=%u, %s, scrolloffset=%u)",
        nrows,
        flag2str(orientation, orient_str),
        scrollOffset);

  traceproc_trap("OCIStmtFetch2", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIStmtFetch2)(stmtp, errhp, nrows,
        orientation, scrollOffset, mode);

  STATS_END(count, FN_OCISTMTFETCH2, 0);
  traceproc_trap("OCIStmtFetch2", "", false, false);

  print_retcode(ret);
  return ret;
}


sword OCITransCommit(OCISvcCtx *svchp, OCIError *errhp, ub4 flags)
{
  char buffer[1024] = {0};
  static const Flag_Str flag_str[] = {
    { OCI_TRANS_TWOPHASE, "OCI_TRANS_TWOPHASE" },
    { OCI_TRANS_WRITEIMMED, "OCI_TRANS_WRITEIMMED" },
    { OCI_TRANS_WRITEBATCH, "OCI_TRANS_WRITEBATCH" },
    { OCI_TRANS_WRITEWAIT, "OCI_TRANS_WRITEWAIT" },
    { OCI_TRANS_WRITENOWAIT, "OCI_TRANS_WRITENOWAIT" },
    { 0, 0}
  };
  flags2str(flags, flag_str, buffer, 1024);
  if (gory)
    tprintf("OCITransCommit(%s)", buffer+2);

  traceproc_trap("OCITransCommit", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCITransCommit)(svchp, errhp, flags);

  STATS_END(count, FN_OCITRANSCOMMIT, 0);
  traceproc_trap("OCITransCommit", "", false, false);

  print_retcode(ret);
  return ret;
}

static void *handle_tree = 0;
static size_t handle_count = 0;
struct Handle_Info {
  void *ptr;
  char name[32];
  bool freed;
  unsigned type;
};
typedef struct Handle_Info Handle_Info;
static int handle_tree_cmp(const void *a, const void *b)
{
  const Handle_Info *x = a;
  const Handle_Info *y = b;
  if (x->ptr < y->ptr)
    return -1;
  if (x->ptr > y->ptr)
    return 1;
  return 0;
}

void handle_tree_check_leaky(const void *nodep, const VISIT which, const int depth)
{
  if (!(which == postorder || which == leaf))
    return;
  const Handle_Info *info = *(const Handle_Info**) nodep;
  if (!info->freed)
    tprintf("ERROR: handle/desc leak - %s was not freed\n", info->name);
}

void handle_tree_check(void **tree, void **hndlpp,
    const char *kind, const char *fn_name)
{
  Handle_Info *e = 0;
  Handle_Info q = {0};
  if (intercept) {
    if (leak_check) {
      if (!hndlpp) {
        tprintf("ERROR: NULL %s pointer", kind);
      }
      if (hndlpp && *hndlpp) {
        q.ptr = *hndlpp;
        strcpy(q.name, "XXX");
        void *val = tfind(&q, tree, handle_tree_cmp);
        Handle_Info *info = val ? *(Handle_Info**)val : 0;
        if (info) {
          if (!info->freed)
            tprintf("ERROR: leak - %s %s was not freed yet\n",
                kind,
                info->name);
        }
      }
    }
    if (gory)
      tprintf("%s(", fn_name);
  }
}

void handle_tree_add(void **hndlpp, int ret, size_t *count, unsigned type,
    const char *kind, void **tree)
{
  if (intercept && hndlpp && *hndlpp && !ret) {
    if (leak_check) {
      Handle_Info q = {0};

      q.ptr = *hndlpp;
      strcpy(q.name, "XXX");
      void *val = tfind(&q, tree, handle_tree_cmp);
      Handle_Info *e = val ? (*(Handle_Info**)val) : 0;

      if (e) {
        if (e->freed) {
          e->freed = false;
        } else {
          tprintf("ERROR: %s %s was not freed but still returned by OCI %p info\n", kind, e->name, e->ptr);
        }
      } else {
        e = calloc(1, sizeof(Handle_Info));
        e->ptr = *hndlpp;
        tsearch(e, tree, handle_tree_cmp);
      }
      e->type = type;
      snprintf(e->name, 32, "%s_%03zu type=%u", kind, (*count)++, type);
      //*hndlpp);
      tprintf("%s", e->name);
    }
    tprintf(" %p)", *hndlpp);
  }
}

void handle_tree_check_free(void *hndlp, unsigned type, void **tree, const char *kind,
    const char *fn_name)
{
  if (!intercept)
    return;

  char name[64] = {0};
  if (leak_check) {
    Handle_Info q = {0};
    Handle_Info *info = 0;
    if (hndlp) {
      q.ptr = hndlp;
      void *val = tfind(&q, tree, handle_tree_cmp);
      if (val)
        info = *(Handle_Info**)val;
      if (info) {
        if (info->freed) {
          tprintf("ERROR: double-free of %s %s\n", kind, info->name);
        } else {
          info->freed = true;
          if (info->type != type) {
            tprintf("%s %s was allocated with a different type (%u) != %u\n",
                kind,
                info->name, info->type, type);
            snprintf(name, 64, "ERROR: %s type error", info->name);
          } else {
            snprintf(name, 64, "%s", info->name);
          }
        }
      } else {
        tprintf("WARN: Trying to free unknown %s\n", kind);
        snprintf(name, 64, "WARN: %s %p is unknown", kind, hndlp);
      }
    }
  }

  if (gory)
    tprintf("%s(%s %p)", fn_name, name, hndlp);

}

sword OCIHandleAlloc(const void  *parenth, void  **hndlpp, const ub4 type, 
                       const size_t xtramem_sz, void  **usrmempp)
{
  handle_tree_check(&handle_tree, hndlpp, "handle",
      "OCIHandleAlloc");


  traceproc_trap("OCIHandleAlloc", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIHandleAlloc)(parenth, hndlpp, type,
      xtramem_sz, usrmempp);

  STATS_END(count, FN_OCIHANDLEALLOC, 0);
  traceproc_trap("OCIHandleAlloc", "", false, false);

  handle_tree_add(hndlpp, ret, &handle_count, type, "HANDLE", &handle_tree);

  print_retcode(ret);
  return ret;
}

sword OCIHandleFree(void  *hndlp, const ub4 type)
{
  handle_tree_check_free(hndlp, type, &handle_tree, "handle", "OCIHandleFree");

  traceproc_trap("OCIHandleFree", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIHandleFree)(hndlp, type);

  STATS_END(count, FN_OCIHANDLEFREE, 0);
  traceproc_trap("OCIHandleFree", "", false, false);

  print_retcode(ret);
  return ret;
}

static void *descriptor_tree = 0;
static size_t descriptor_count = 0;

sword OCIDescriptorAlloc(const void  *parenth, void  **descpp,
    const ub4 type, const size_t xtramem_sz, void  **usrmempp)
{
  handle_tree_check(&descriptor_tree, descpp, "descriptor",
      "OCIDescriptorAlloc");

  traceproc_trap("OCIDescriptorAlloc", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIDescriptorAlloc)(parenth, descpp, type,
      xtramem_sz, usrmempp);

  STATS_END(count, FN_OCIDESCRIPTORALLOC, 0);
  traceproc_trap("OCIDescriptorAlloc", "", false, false);

  handle_tree_add(descpp, ret, &descriptor_count, type, "DESC", &descriptor_tree);

  print_retcode(ret);
  return ret;
}

sword OCIDescriptorFree(void  *descp, const ub4 type)
{
  handle_tree_check_free(descp, type, &descriptor_tree, "descriptor", "OCIDescriptorFree");

  traceproc_trap("OCIDescriptorFree", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIDescriptorFree)(descp, type);

  STATS_END(count, FN_OCIDESCRIPTORFREE, 0);
  traceproc_trap("OCIDescriptorFree", "", false, false);

  print_retcode(ret);
  return ret;
}

void *array_desc_tree = 0;
size_t array_desc_count = 0;

void res_tree_add(void **tree, size_t *count, void *ptr, unsigned type,
    const char *kind, char *name, size_t name_size)
{
  if (!intercept || !leak_check)
    return;
  if (!ptr) {
    tprintf("ERROR: NULL pointer used for obtaining a %s\n", kind);
    return;
  }

  Handle_Info q = {0};
  q.ptr = ptr;
  strcpy(q.name, "XXX");
  void *val = tfind(&q, tree, handle_tree_cmp);
  Handle_Info *e = val ? *(Handle_Info**)val : 0;

  if (e) {
    if (e->freed) {
      e->freed = false;
    } else {
      tprintf("ERROR: resource leak - %s was not freed %p\n", e->name, e->ptr);
    }
  } else {
    e = calloc(1, sizeof(Handle_Info));
    e->ptr = ptr;
    tsearch(e, tree, handle_tree_cmp);
  }
  e->type = type;
  snprintf(e->name, 32, "%s_%03zu type=%u %p", kind, (*count)++, type,
      ptr);
  if (!name_size)
    return;
  strncpy(name, e->name, name_size);
}

void res_tree_del(void **tree, void *ptr, unsigned type, const char *kind,
    char *name, size_t name_size)
{
  if (!intercept || !leak_check)
    return;
  if (!ptr) {
    tprintf("ERROR: NULL pointer used for freeing a %s\n", kind);
    return;
  }
  Handle_Info q = {0};
  q.ptr = ptr;
  void *val = tfind(&q, tree, handle_tree_cmp);
  Handle_Info *info = val ? *(Handle_Info**)val : 0;
  if (info) {
    if (info->freed) {
      tprintf("ERROR: double-free of %s %s\n", kind, info->name);
    } else {
      info->freed = true;
      if (info->type != type) {
        tprintf("ERROR: %s %s was allocated with a different type (%u) != %u\n",
            kind,
            info->name, info->type, type);
      } else {
        snprintf(name, name_size, "%s", info->name);
      }
    }
  } else {
    tprintf("WARN: Trying to free unknown %s %p\n", kind, ptr);
  }
}

sword OCIArrayDescriptorAlloc(const void *parenth, void **descpp,
    const ub4 type, ub4 array_size,
    const size_t xtramem_sz, void  **usrmempp)
{
  char name[32] = {0};
  res_tree_add(&array_desc_tree, &array_desc_count, descpp, type, "arr_desc", name, 32);

  if (gory)
    tprintf("OCIArrayDescriptorAlloc(n=%u, %s, type=%u, %p)",
        array_size, name, type, descpp);

  traceproc_trap("OCIArrayDescriptorAlloc", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIArrayDescriptorAlloc)(parenth, descpp,
      type, array_size,
      xtramem_sz, usrmempp);

  STATS_END(count, FN_OCIARRAYDESCRIPTORALLOC, 0);
  traceproc_trap("OCIArrayDescriptorAlloc", "", false, false);

  print_retcode(ret);
  return ret;
}

sword OCIArrayDescriptorFree(void **descp, const ub4 type)
{
  char name[64] = {0};
  res_tree_del(&array_desc_tree, descp, type, "arr_desc", name, 64);

  if (gory)
    tprintf("OCIArrayDescriptorFree(%s, type=%u, %p)",
        name, type, descp);

  traceproc_trap("OCIArrayDescriptorFree", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIArrayDescriptorFree)(descp, type);

  STATS_END(count, FN_OCIARRAYDESCRIPTORFREE, 0);
  traceproc_trap("OCIArrayDescriptorFree", "", false, false);

  print_retcode(ret);
  return ret;
}

sword OCISessionBegin(OCISvcCtx *svchp, OCIError *errhp,
    OCISession *usrhp, ub4 credt, ub4 mode)
{
  static const Flag_Str cred_str[] = {
    { OCI_CRED_RDBMS, "OCI_CRED_RDBMS" },
    { OCI_CRED_EXT, "OCI_CRED_EXT" }, 
    { 0, 0}
  };
  static const Flag_Str mod_str[] = {
    { OCI_DEFAULT, "OCI_DEFAULT" },
    { OCI_MIGRATE, "OCI_MIGRATE" },
    { OCI_SYSDBA, "OCI_SYSDBA" },
    { OCI_SYSOPER, "OCI_SYSOPER" },
    { OCI_PRELIM_AUTH, "OCI_PRELIM_AUTH" },
    { 0, 0}
  };
  if (gory)
    tprintf("OCISessionBegin(%s, %s)", flag2str(credt, cred_str),
        flag2str(mode, mod_str));

  traceproc_trap("OCISessionBegin", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCISessionBegin)(svchp, errhp,
      usrhp, credt, mode);

  STATS_END(count, FN_OCISESSIONBEGIN, 0);
  traceproc_trap("OCISessionBegin", "", false, false);

  print_retcode(ret);
  return ret;
}

sword OCISessionEnd(OCISvcCtx *svchp, OCIError *errhp,
    OCISession *usrhp, ub4 mode)
{
  if (gory)
    tprintf("OCISessionEnd()");

  traceproc_trap("OCISessionEnd", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCISessionEnd)(svchp, errhp,
      usrhp, mode);

  STATS_END(count, FN_OCISESSIONEND, 0);
  traceproc_trap("OCISessionEnd", "", false, false);

  print_retcode(ret);
  return ret;
}

#include "ociattr.c"
#include "sqlgls_str.c"

sword OCIAttrGet (const void  *trgthndlp, ub4 trghndltyp,
    void  *attributep, ub4 *sizep, ub4 attrtype,
    OCIError *errhp)
{
  if (gory) {
    tprintf("OCIAttrGet(%s", flag2str(attrtype, attr_str));

  }

  traceproc_trap("OCIAttrGet", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIAttrGet)(trgthndlp, trghndltyp,
      attributep, sizep, attrtype, errhp);

  STATS_END(count, FN_OCIATTRGET, 0);
  traceproc_trap("OCIAttrGet", "", false, false);

  if (gory) {
    switch (attrtype) {
      case OCI_ATTR_SQLFNCODE:
        {
          ub2 code = *(ub2*)attributep;
          if (code > sql_fn_code_str_size)
            tprintf(", UNK (%u)", code);
          else
            tprintf(", %s", sql_fn_code_str[code]);
        }
        break;
    }
    tprintf(")");
  }

  print_retcode(ret);
  return ret;
}

sword OCIAttrSet (void  *trgthndlp, ub4 trghndltyp, void  *attributep,
    ub4 size, ub4 attrtype, OCIError *errhp)
{
  if (gory) {
    tprintf("OCIAttrSet(%s", flag2str(attrtype, attr_str));

  switch (attrtype) {
    case OCI_ATTR_PASSWORD:
    case OCI_ATTR_USERNAME:
      tprintf(" %.*s", size, (const char*) attributep);
      break;
  }
  tprintf(")");
  }

  traceproc_trap("OCIAttrSet", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIAttrSet)(trgthndlp, trghndltyp,
      attributep, size, attrtype, errhp);

  STATS_END(count, FN_OCIATTRSET, 0);
  traceproc_trap("OCIAttrSet", "", false, false);

  print_retcode(ret);
  return ret;
}

sword OCIServerAttach(OCIServer *srvhp, OCIError *errhp,
    const OraText *dblink, sb4 dblink_len, ub4 mode)
{
  static const Flag_Str mode_str[] = {
    { OCI_DEFAULT, "OCI_DEFAULT" },
    { OCI_CPOOL, "OCI_CPOOL" },
    { 0, 0 }
  };
  if (gory)
    tprintf("OCIServerAttach(%.*s %s)", dblink_len, (const char*)dblink,
        flag2str(mode, mode_str));

  traceproc_trap("OCIServerAttach", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIServerAttach)(srvhp, errhp,
      dblink, dblink_len, mode);

  STATS_END(count, FN_OCISERVERATTACH, 0);
  traceproc_trap("OCIServerAttach", "", false, false);

  print_retcode(ret);
  return ret;
}

sword OCIServerDetach(OCIServer *srvhp, OCIError *errhp, ub4 mode)
{
  if (gory)
    tprintf("OCIServerDetach()");

  traceproc_trap("OCIDetach", "", false, false);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIServerDetach)(srvhp, errhp, mode);

  STATS_END(count, FN_OCISERVERDETACH, 0);
  traceproc_trap("OCIDetach", "", false, true);

  print_retcode(ret);
  return ret;
}

sword OCIInitialize(ub4 mode, void  *ctxp, 
    void  *(*malocfp)(void  *ctxp, size_t size),
    void  *(*ralocfp)(void  *ctxp, void  *memptr, size_t newsize),
    void   (*mfreefp)(void  *ctxp, void  *memptr) )
{
  static const Flag_Str mode_str[] = {
    //{ OCI_DEFAULT, "OCI_DEFAULT" },
    { OCI_THREADED, "OCI_THREADED" },
    { OCI_OBJECT, "OCI_OBJECT" },
    { OCI_EVENTS, "OCI_EVENTS" },
    { 0, 0}
  };
  char buffer[128] = {0};
  flags2str(mode, mode_str, buffer, 128);
  if (gory)
    tprintf("OCIInitialize(%s)", buffer+2);

  traceproc_trap("OCIInitialize", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIInitialize)(mode, ctxp, malocfp, ralocfp, mfreefp);

  STATS_END(count, FN_OCIINITIALIZE, 0);
  traceproc_trap("OCIInitialize", "", false, false);

  print_retcode(ret);
  return ret;
}

sword OCILobWrite(OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,
    ub4 *amtp, ub4 offset, void  *bufp, ub4 buflen,
    ub1 piece,  void  *ctxp, OCICallbackLobWrite cbfp,
    ub2 csid, ub1 csfrm)
{
  if (gory)
    tprintf("OCILobWrite()");

  traceproc_trap("OCILobWrite", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCILobWrite)(svchp, errhp, locp, amtp, offset, bufp, buflen, piece, ctxp, cbfp, csid, csfrm);

  STATS_END(count, FN_OCILOBWRITE, 0);
  traceproc_trap("OCILobWrite", "", false, false);

  print_retcode(ret);
  return ret;
}

sword OCIParamGet(const void  *hndlp, ub4 htype, OCIError *errhp, 
    void  **parmdpp, ub4 pos)
{
  if (gory)
    tprintf("OCIParamGet(pos=%2u)", pos);

  traceproc_trap("OCIParamGet", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIParamGet)(hndlp, htype, errhp, parmdpp, pos);

  STATS_END(count, FN_OCIPARAMGET, 0);
  traceproc_trap("OCIParamGet", "", false, false);

  print_retcode(ret);
  return ret;
}

sword OCIParamSet(void  *hdlp, ub4 htyp, OCIError *errhp, const void  *dscp,
    ub4 dtyp, ub4 pos)
{
  if (gory)
    tprintf("OCIParamSet(pos=%2u)", pos);

  traceproc_trap("OCIParamSet", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCIParamSet)(hdlp, htyp, errhp, dscp, dtyp, pos);

  STATS_END(count, FN_OCIPARAMSET, 0);
  traceproc_trap("OCIParamSet", "", false, false);

  print_retcode(ret);
  return ret;
}
sword OCITransStart(OCISvcCtx *svchp, OCIError *errhp, 
    uword timeout, ub4 flags )
{
  static const Flag_Str flag_str[] = {
    { OCI_TRANS_NEW, "OCI_TRANS_NEW" },
    { OCI_TRANS_TIGHT, "OCI_TRANS_TIGHT" },
    { OCI_TRANS_LOOSE, "OCI_TRANS_LOOSE" },
    { OCI_TRANS_RESUME, "OCI_TRANS_RESUME" },
    { OCI_TRANS_READONLY, "OCI_TRANS_READONLY" },
    { OCI_TRANS_SERIALIZABLE, "OCI_TRANS_SERIALIZABLE" },
    { OCI_TRANS_SEPARABLE, "OCI_TRANS_SEPARABLE" },
    { 0, 0}
  };
  char buffer[128] = {0};
  flags2str(flags, flag_str, buffer, 128);
  if (gory)
    tprintf("OCITransStart(timeout=%u, %s)", timeout, buffer+2);

  traceproc_trap("OCITransStart", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCITransStart)(svchp, errhp, timeout, flags);

  STATS_END(count, FN_OCITRANSSTART, 0);
  traceproc_trap("OCITransStart", "", false, false);

  print_retcode(ret);
  return ret;
}

sword   OCITransRollback  (OCISvcCtx *svchp, OCIError *errhp, ub4 flags)
{
  if (gory)
    tprintf("OCITransRollback()");

  traceproc_trap("OCITransRollback", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCITransRollback)(svchp, errhp, flags);

  STATS_END(count, FN_OCITRANSROLLBACK, 0);
  traceproc_trap("OCITransRollback", "", false, false);

  print_retcode(ret);
  return ret;
}


sword OCISessionPoolCreate (OCIEnv *envhp, OCIError *errhp, OCISPool *spoolhp, 
      OraText **poolName, ub4 *poolNameLen, 
      const OraText *connStr, ub4 connStrLen,
      ub4 sessMin, ub4 sessMax, ub4 sessIncr,
      OraText *userid, ub4 useridLen,
      OraText *password, ub4 passwordLen,
      ub4 mode)
{
  static const Flag_Str mode_str[] = {
    //{ OCI_DEFAULT, "OCI_DEFAULT" },
    { OCI_SPC_REINITIALIZE, "OCI_SPC_REINITIALIZE" },
    { OCI_SPC_STMTCACHE, "OCI_SPC_STMTCACHE" },
    { OCI_SPC_HOMOGENEOUS, "OCI_SPC_HOMOGENEOUS" },
    { OCI_SPC_NO_RLB, "OCI_SPC_NO_RLB" },
    { 0, 0}
  };
  char buffer[128] = {0};
  flags2str(mode, mode_str, buffer, 128);
  if (gory)
    tprintf("OCISessionPoolCreate(%.*s, %.*s, %.*s, sessMin=%u, sessMax=%u, sessIncr=%u, %s",
        connStrLen, (const char*) connStr,
        useridLen, (const char *) userid,
        passwordLen, (const char*) password,
        sessMin, sessMax, sessIncr,
        buffer+2
        );

  traceproc_trap("OCISessionPoolCreate", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCISessionPoolCreate)(
      envhp, errhp, spoolhp,
      poolName, poolNameLen,
      connStr, connStrLen,
      sessMin, sessMax, sessIncr,
      userid, useridLen,
      password, passwordLen,
      mode);

  STATS_END(count, FN_OCISESSIONPOOLCREATE, 0);
  traceproc_trap("OCISessionPoolCreate", "", false, false);

  tprintf(", %.*s)",
      poolNameLen ? (*poolNameLen) : 0,
      poolName ? ((const char*) *poolName) : "NULL"
      );

  print_retcode(ret);
  return ret;
}

sword OCISessionPoolDestroy (OCISPool *spoolhp,
      OCIError *errhp,
      ub4 mode)
{
  static const Flag_Str mode_str[] = {
    { OCI_DEFAULT, "OCI_DEFAULT" },
    { OCI_SPD_FORCE, "OCI_SPD_FORCE" },
    { 0, 0}
  };
  if (gory)
    tprintf("OCISessionPoolDestroy(%s)", flag2str(mode, mode_str));

  traceproc_trap("OCISessionPoolDestroy", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCISessionPoolDestroy)(spoolhp, errhp, mode);

  STATS_END(count, FN_OCISESSIONPOOLDESTROY, 0);
  traceproc_trap("OCISessionPoolDestroy", "", false, false);

  print_retcode(ret);
  return ret;
}

sword OCISessionGet (OCIEnv *envhp, OCIError *errhp, OCISvcCtx **svchp,
      OCIAuthInfo *authhp,
      OraText *poolName, ub4 poolName_len, 
      const OraText *tagInfo, ub4 tagInfo_len,
      OraText **retTagInfo, ub4 *retTagInfo_len,
      boolean *found, ub4 mode)
{
  static const Flag_Str mode_str[] = {
    //{ OCI_DEFAULT, "OCI_DEFAULT" },
    { OCI_SESSGET_CPOOL, "OCI_SESSGET_CPOOL" },
    { OCI_SESSGET_SPOOL, "OCI_SESSGET_SPOOL" },
    { OCI_SESSGET_CREDPROXY, "OCI_SESSGET_CREDPROXY" },
    { OCI_SESSGET_CREDEXT, "OCI_SESSGET_CREDEXT" },
    { OCI_SESSGET_PURITY_NEW, "OCI_SESSGET_PURITY_NEW" },
    { OCI_SESSGET_PURITY_SELF, "OCI_SESSGET_PURITY_SELF" },
    { OCI_SESSGET_SPOOL_MATCHANY, "OCI_SESSGET_SPOOL_MATCHANY" },
    { OCI_SESSGET_STMTCACHE, "OCI_SESSGET_STMTCACHE" },
    { 0, 0}
  };
  char buffer[128] = {0};
  flags2str(mode, mode_str, buffer, 128);
  if (gory)
    tprintf("OCISessionGet(%.*s, %.*s, %s",
        poolName_len, (const char*)poolName,
        tagInfo_len, (const char*)tagInfo,
        buffer+2
        );

  traceproc_trap("OCISessionGet", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCISessionGet)(envhp, errhp, svchp,
      authhp,
      poolName, poolName_len,
      tagInfo, tagInfo_len,
      retTagInfo, retTagInfo_len,
      found, mode);

  STATS_END(count, FN_OCISESSIONGET, 0);
  traceproc_trap("OCISessionGet", "", false, false);

  tprintf(", found=%s, %.*s)",
      found ? (*found ? "true" : "false") : "NULL",
      retTagInfo_len ? (*retTagInfo_len) : 0,
      retTagInfo ? ((const char*)*retTagInfo) : "NULL");

  print_retcode(ret);
  return ret;
}

sword OCISessionRelease (OCISvcCtx *svchp, OCIError *errhp,
      OraText *tag, ub4 tag_len,
      ub4 mode)
{
  static const Flag_Str mode_str[] = {
    { OCI_DEFAULT, "OCI_DEFAULT" },
    { OCI_SESSRLS_DROPSESS, "OCI_SESSRLS_DROPSESS" },
    { OCI_SESSRLS_RETAG, "OCI_SESSRLS_RETAG" },
    { 0, 0}
  };
  if (gory)
    tprintf("OCISessionRelease(%.*s, %s)",
        tag_len, (const char*) tag,
        flag2str(mode, mode_str)
        );

  traceproc_trap("OCISessionRelease", "", false, true);
  STATS_BEGIN(count);

  sword ret = (*fn_table.OCISessionRelease)(svchp, errhp,
      tag, tag_len,
      mode);

  STATS_END(count, FN_OCISESSIONRELEASE, 0);
  traceproc_trap("OCISessionRelease", "", false, false);

  print_retcode(ret);
  return ret;
}


INTERCEPT_SETUP(OCIEnvInit)
INTERCEPT_SETUP(OCIEnvCreate)
INTERCEPT_SETUP(OCIEnvNlsCreate)
INTERCEPT_SETUP(OCILogon)
INTERCEPT_SETUP(OCILogon2)
INTERCEPT_SETUP(OCIStmtPrepare)
INTERCEPT_SETUP(OCIStmtPrepare2)
INTERCEPT_SETUP(OCIBindByPos)
INTERCEPT_SETUP(OCIBindByName)
INTERCEPT_SETUP(OCIDefineByPos)
INTERCEPT_SETUP(OCITerminate)
INTERCEPT_SETUP(OCIStmtExecute)
INTERCEPT_SETUP(OCIStmtFetch)
INTERCEPT_SETUP(OCIStmtFetch2)
INTERCEPT_SETUP(OCITransCommit)
INTERCEPT_SETUP(OCIHandleAlloc)
INTERCEPT_SETUP(OCIHandleFree)
INTERCEPT_SETUP(OCIDescriptorAlloc)
INTERCEPT_SETUP(OCIDescriptorFree)
INTERCEPT_SETUP(OCIArrayDescriptorAlloc)
INTERCEPT_SETUP(OCIArrayDescriptorFree)
INTERCEPT_SETUP(OCISessionBegin)
INTERCEPT_SETUP(OCISessionEnd)
INTERCEPT_SETUP(OCIAttrGet)
INTERCEPT_SETUP(OCIAttrSet)
INTERCEPT_SETUP(OCIServerAttach)
INTERCEPT_SETUP(OCIServerDetach)
INTERCEPT_SETUP(OCIInitialize)
INTERCEPT_SETUP(OCILobWrite)
INTERCEPT_SETUP(OCIParamGet)
INTERCEPT_SETUP(OCIParamSet)
INTERCEPT_SETUP(OCITransStart)
INTERCEPT_SETUP(OCITransRollback)
INTERCEPT_SETUP(OCISessionGet)
INTERCEPT_SETUP(OCISessionPoolCreate)
INTERCEPT_SETUP(OCISessionPoolDestroy)
INTERCEPT_SETUP(OCISessionRelease)

void ocitrace_setup(bool trace_intercept, bool trace_gory, bool trace_sql,
    bool enable_stats, bool enable_leak_check)
{
  intercept = trace_intercept;
  if (trace_intercept) {
    sql = trace_sql;
    gory = trace_gory;
    count = enable_stats;
    leak_check = enable_leak_check;
  }
  int r = 0;
  r += setup_OCIEnvInit();
  r += setup_OCIEnvCreate();
  r += setup_OCIEnvNlsCreate();
  r += setup_OCILogon();
  r += setup_OCILogon2();
  r += setup_OCIStmtPrepare();
  r += setup_OCIStmtPrepare2();
  r += setup_OCIBindByPos();
  r += setup_OCIBindByName();
  r += setup_OCIDefineByPos();
  r += setup_OCITerminate();
  r += setup_OCIStmtExecute();
  r += setup_OCIStmtFetch();
  r += setup_OCIStmtFetch2();
  r += setup_OCITransCommit();
  r += setup_OCIHandleAlloc();
  r += setup_OCIHandleFree();
  r += setup_OCIDescriptorAlloc();
  r += setup_OCIDescriptorFree();
  r += setup_OCIArrayDescriptorAlloc();
  r += setup_OCIArrayDescriptorFree();
  r += setup_OCISessionBegin();
  r += setup_OCISessionEnd();
  r += setup_OCIAttrGet();
  r += setup_OCIAttrSet();
  r += setup_OCIServerAttach();
  r += setup_OCIServerDetach();
  r += setup_OCIInitialize();
  r += setup_OCILobWrite();
  r += setup_OCIParamGet();
  r += setup_OCIParamSet();
  r += setup_OCITransStart();
  r += setup_OCITransRollback();
  r += setup_OCISessionGet();
  r += setup_OCISessionPoolCreate();
  r += setup_OCISessionPoolDestroy();
  r += setup_OCISessionRelease();

  // do not exit because a non-oracle-linked fork'ed child also
  // triggers this 'error'
  //if (r) {
  //  fprintf(stderr, "Exiting because of previous dlsym() errors.\n");
  //  exit(23);
  //}
  int ret = stats_init(&stats, 0, FN_SIZE);
  IFTRUEEXIT(ret, 0, -1);
}

int ocitrace_pp_stats(const struct timespec *prog_time)
{
  int ret = stats_sum_up(&stats, 0, FN_SIZE, prog_time);
  IFTRUERET(ret, 0, ret);
  ret = stats_pp_fns(&stats, func_str, FN_SIZE);
  IFTRUERET(ret, 0, ret);
  return 0;
}

int ocitrace_finish()
{
  if (leak_check) {
    twalk(handle_tree, handle_tree_check_leaky);
    twalk(descriptor_tree, handle_tree_check_leaky);
    twalk(array_desc_tree, handle_tree_check_leaky);
  }
  // XXX perhaps destroy the tree ...
  return 0;
}


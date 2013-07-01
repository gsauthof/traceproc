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


#include "test_proc_basic.h"
#include <proc_util.h>

#include <callback.h>

#include <stdio.h>

#include "basic.h"

#define STMTS_SIZE 10
#define PARAS_SIZE 32

struct LScratch {
  bool overflowed;
  int marker;
  Statement stmts[STMTS_SIZE];
  Parameter paras[PARAS_SIZE];
  size_t pos;
  size_t ppos;
  char values[PARAS_SIZE][128];

};
typedef struct LScratch LScratch;

struct Scratch {
  LScratch before;
  LScratch after;
};
typedef struct Scratch Scratch;

static int before_fn(const Statement *stmt, void *user_ptr)
{
  Scratch *x = user_ptr;
  x->before.marker = 42;
  if (x->before.pos >= STMTS_SIZE) {
    x->before.overflowed = true;
    return 0;
  }
  x->before.stmts[x->before.pos] = *stmt;
  ++x->before.pos;
  return 0;
}
static int after_fn(const Statement *stmt,
      const struct sqlca *s,
      const struct oraca *oraca,
      void *user_ptr)
{
  Scratch *x = user_ptr;
  x->after.marker = 42;
  if (x->after.pos >= STMTS_SIZE) {
    x->after.overflowed = true;
    return 0;
  }
  x->after.stmts[x->after.pos] = *stmt;
  ++x->after.pos;
  return 0;
}

int parameter_fn(const Statement *stmt, const Parameter *p,
      bool before,
      void *user_ptr)
{
  Scratch *x = user_ptr;
  LScratch *l = before ? &x->before : &x->after;
  if (l->ppos >= PARAS_SIZE) {
    x->after.overflowed = true;
    return 0;
  }
  l->paras[l->ppos] = *p;
  memcpy(l->values[l->ppos], p->value, 128);
  ++l->ppos;
  return 0;
}

START_TEST(connect_01)
{
  unsigned id = 0;
  Traceproc_Callbacks callbacks = {
    .user_ptr  = (void*) 42,
    .before_fn = before_fn
  };
  int ret_tp = traceproc_register_callbacks(&callbacks, &id);
  ck_assert_int_eq(ret_tp, 0);
  ret_tp = traceproc_unregister_callbacks(id);
  ck_assert_int_eq(ret_tp, 0);
}
END_TEST

START_TEST(select_01)
{
  char out[32] = {0};
  unsigned id = 0;
  Scratch scratch = {0};
  Traceproc_Callbacks callbacks = {
    .user_ptr  = &scratch,
    .before_fn = before_fn,
    .after_fn = after_fn,
    .parameter_fn = parameter_fn
  };
  int ret_tp = traceproc_register_callbacks(&callbacks, &id);
  ck_assert_int_eq(ret_tp, 0);
  ck_assert_int_eq(ret_tp, 0);
  int ret = select_date("2013-03-23 22:50:44", out, 31);
  ck_assert_int_eq(ret, 0);
  ck_assert_int_eq(scratch.before.marker, 42);
  ck_assert_int_eq(scratch.after.marker, 42);
  ck_assert(scratch.before.stmts[0].type == SELECT);
  ck_assert(scratch.after.stmts[0].type == SELECT);

  ck_assert(!scratch.before.overflowed);
  ck_assert(!scratch.after.overflowed);

  const Statement *s = scratch.before.stmts;
  ck_assert_str_eq(s->type_str, "SELECT");
  ck_assert_int_eq(s->number_of_params, 2);
  ck_assert_int_eq(s->number_of_params_in, 1);
  ck_assert_int_eq(s->iterations, 1);
  ck_assert_str_eq(s->text, "select to_char((to_date(:b0,'YYYY-MM-DD HH24:MI:SS')+ interval '4' hour ),'YYYY-MM-DD HH24:MI:SS') into :b1  from dual ");
  ck_assert_int_eq(s->length, strlen(s->text));

  const Parameter *p = scratch.before.paras;
  const void *v = scratch.before.values;
  ck_assert_int_eq(p->pos, 0);
  ck_assert_int_eq(p->direction, PARA_IN);
  ck_assert_int_eq(p->indicator, 0);
  ck_assert_int_eq(p->type, STRING0);
  ck_assert_str_eq(v, "2013-03-23 22:50:44");

  p = scratch.after.paras+1;
  v = scratch.after.values+1;
  ck_assert_int_eq(p->pos, 1);
  ck_assert_int_eq(p->direction, PARA_OUT);
  ck_assert_int_eq(p->indicator, 0);
  ck_assert_int_eq(p->type, STRING0);
  ck_assert_str_eq(p->type_str, "STRING0");
  ck_assert_int_eq(p->length, 32);
  ck_assert_int_eq(p->size, 0);
  ck_assert_str_eq(v, "2013-03-24 02:50:44");

  ck_assert_str_eq("2013-03-24 02:50:44", out);

  ret_tp = traceproc_unregister_callbacks(id);
}
END_TEST

START_TEST(select_charz)
{
  char out[32] = {0};
  unsigned id = 0;
  Scratch scratch = {0};
  Traceproc_Callbacks callbacks = {
    .user_ptr  = &scratch,
    .before_fn = before_fn,
    .after_fn = after_fn,
    .parameter_fn = parameter_fn
  };
  int ret_tp = traceproc_register_callbacks(&callbacks, &id);
  ck_assert_int_eq(ret_tp, 0);
  ck_assert_int_eq(ret_tp, 0);
  int ret = select_date_charz("2013-03-23 22:50:44", out, 31);
  ck_assert_int_eq(ret, 0);
  ck_assert_int_eq(scratch.before.marker, 42);
  ck_assert_int_eq(scratch.after.marker, 42);
  ck_assert(scratch.before.stmts[0].type == SELECT);
  ck_assert(scratch.after.stmts[0].type == SELECT);

  ck_assert(!scratch.before.overflowed);
  ck_assert(!scratch.after.overflowed);

  const Statement *s = scratch.before.stmts;
  ck_assert_str_eq(s->type_str, "SELECT");
  ck_assert_int_eq(s->number_of_params, 2);
  ck_assert_int_eq(s->number_of_params_in, 1);
  ck_assert_int_eq(s->iterations, 1);
  ck_assert_str_eq(s->text, "select to_char((to_date(:b0,'YYYY-MM-DD HH24:MI:SS')+ interval '4' hour ),'YYYY-MM-DD HH24:MI:SS') into :b1  from dual ");
  ck_assert_int_eq(s->length, strlen(s->text));

  const Parameter *p = scratch.before.paras;
  const void *v = scratch.before.values;
  ck_assert_int_eq(p->pos, 0);
  ck_assert_int_eq(p->direction, PARA_IN);
  ck_assert_int_eq(p->indicator, 0);
  ck_assert_int_eq(p->type, STRING);
  ck_assert_str_eq(v, "2013-03-23 22:50:44");

  p = scratch.after.paras+1;
  v = scratch.after.values+1;
  ck_assert_int_eq(p->pos, 1);
  ck_assert_int_eq(p->direction, PARA_OUT);
  ck_assert_int_eq(p->indicator, 0);
  ck_assert_int_eq(p->type, STRING);
  ck_assert_str_eq(p->type_str, "STRING");
  ck_assert_int_eq(p->length, 32);
  ck_assert_int_eq(p->size, 0);
  ck_assert_str_eq(v, "2013-03-24 02:50:44            ");

  ck_assert_str_eq("2013-03-24 02:50:44            ", out);

  ret_tp = traceproc_unregister_callbacks(id);
}
END_TEST

START_TEST(select_varchar)
{
  char out[32] = {0};
  unsigned id = 0;
  Scratch scratch = {0};
  Traceproc_Callbacks callbacks = {
    .user_ptr  = &scratch,
    .before_fn = before_fn,
    .after_fn = after_fn,
    .parameter_fn = parameter_fn
  };
  int ret_tp = traceproc_register_callbacks(&callbacks, &id);
  ck_assert_int_eq(ret_tp, 0);
  ck_assert_int_eq(ret_tp, 0);
  unsigned short len = 0;
  int ret = select_date_varchar("2013-03-23 22:50:44", out, 31, &len);
  ck_assert_int_eq(ret, 0);
  ck_assert_int_eq(scratch.before.marker, 42);
  ck_assert_int_eq(scratch.after.marker, 42);
  ck_assert(scratch.before.stmts[0].type == SELECT);
  ck_assert(scratch.after.stmts[0].type == SELECT);

  ck_assert(!scratch.before.overflowed);
  ck_assert(!scratch.after.overflowed);

  const Statement *s = scratch.before.stmts;
  ck_assert_str_eq(s->type_str, "SELECT");
  ck_assert_int_eq(s->number_of_params, 2);
  ck_assert_int_eq(s->number_of_params_in, 1);
  ck_assert_int_eq(s->iterations, 1);
  ck_assert_str_eq(s->text, "select to_char((to_date(:b0,'YYYY-MM-DD HH24:MI:SS')+ interval '4' hour ),'YYYY-MM-DD HH24:MI:SS') into :b1  from dual ");
  ck_assert_int_eq(s->length, strlen(s->text));

  const Parameter *p = scratch.before.paras;
  const void *v = scratch.before.values;
  ck_assert_int_eq(p->pos, 0);
  ck_assert_int_eq(p->direction, PARA_IN);
  ck_assert_int_eq(p->indicator, 0);
  ck_assert_int_eq(p->type, STRING0);
  ck_assert_str_eq(v, "2013-03-23 22:50:44");

  p = scratch.after.paras+1;
  v = scratch.after.values+1;
  ck_assert_int_eq(p->pos, 1);
  ck_assert_int_eq(p->direction, PARA_OUT);
  ck_assert_int_eq(p->indicator, 0);
  ck_assert_int_eq(p->type, VARCHAR);
  ck_assert_str_eq(p->type_str, "VARCHAR");
  ck_assert_int_eq(p->length, 32+sizeof(short));
  ck_assert_int_eq(p->size, 0);
  char t[32] = {0};
  strncpy(t, v+sizeof(short), 19);
  ck_assert_str_eq(t, "2013-03-24 02:50:44");

  ck_assert_str_eq("2013-03-24 02:50:44", out);
  ck_assert_int_eq(19, len);

  ret_tp = traceproc_unregister_callbacks(id);
}
END_TEST

START_TEST(char_01)
{
  Scratch scratch = {0};
  Traceproc_Callbacks callbacks = {
    .user_ptr  = &scratch,
    .before_fn = before_fn,
    .after_fn = after_fn,
    .parameter_fn = parameter_fn
  };
  unsigned id = 0;
  int ret_tp = traceproc_register_callbacks(&callbacks, &id);
  ck_assert_int_eq(ret_tp, 0);

  char c = 'x';
  int ret = char_type('x', &c);
  ck_assert_int_eq(ret, 0);
  ck_assert(c == 'y');

  ck_assert(!scratch.before.overflowed);
  ck_assert(!scratch.after.overflowed);

  const Statement *s = scratch.before.stmts;
  ck_assert_int_eq(s->type, SELECT);
  ck_assert_str_eq(s->type_str, "SELECT");
  ck_assert_int_eq(s->number_of_params, 2);
  ck_assert_int_eq(s->number_of_params_in, 1);
  ck_assert_int_eq(s->iterations, 1);
  ck_assert_str_eq(s->text, "select CHR((ASCII(:b0)+1)) into :b1  from dual ");
  ck_assert_int_eq(s->length, strlen(s->text));

  const Parameter *p = scratch.before.paras;
  const void *v = scratch.before.values;
  ck_assert_int_eq(p->pos, 0);
  ck_assert_int_eq(p->direction, PARA_IN);
  ck_assert_int_eq(p->indicator, 0);
  ck_assert_int_eq(p->type, CHAR);
  ck_assert(*(char*)v == 'x');

  p = scratch.after.paras+1;
  v = scratch.after.values+1;
  ck_assert_int_eq(p->pos, 1);
  ck_assert_int_eq(p->direction, PARA_OUT);
  ck_assert_int_eq(p->indicator, 0);
  ck_assert_int_eq(p->type, CHAR);
  ck_assert_str_eq(p->type_str, "CHAR");
  ck_assert_int_eq(p->length, 1);
  ck_assert_int_eq(p->size, 0);
  ck_assert(*(char*)v == 'y');

  ret_tp = traceproc_unregister_callbacks(id);
  ck_assert_int_eq(ret_tp, 0);
}
END_TEST

START_TEST(syn_01)
{
  int ret = create_table();
  ck_assert_int_eq(ret, 0);
  drop_syn();


  Scratch scratch = {0};
  Traceproc_Callbacks callbacks = {
    .user_ptr  = &scratch,
    .before_fn = before_fn,
    .after_fn = after_fn,
    .parameter_fn = parameter_fn
  };
  unsigned id = 0;
  int ret_tp = traceproc_register_callbacks(&callbacks, &id);
  ck_assert_int_eq(ret_tp, 0);

  ret = create_syn();
  ck_assert_int_eq(ret, 0);

  const Statement *s = scratch.before.stmts;
  ck_assert_int_eq(s->type, CREATE);
  ck_assert_int_eq(s->number_of_params, 0);
  ck_assert_int_eq(s->number_of_params_in, 0);
  ck_assert_int_eq(s->iterations, 1);
  ck_assert_str_eq(s->text, "create SYNONYM traceproc_syn FOR traceproc_tbl");
  ck_assert_int_eq(s->length, strlen(s->text));

  s = scratch.after.stmts;
  ck_assert_int_eq(s->type, CREATE);

  ret_tp = traceproc_unregister_callbacks(id);
  ck_assert_int_eq(ret_tp, 0);
}
END_TEST

START_TEST(syn_02)
{
  Scratch scratch = {0};
  Traceproc_Callbacks callbacks = {
    .user_ptr  = &scratch,
    .before_fn = before_fn,
    .after_fn = after_fn,
    .parameter_fn = parameter_fn
  };
  unsigned id = 0;
  int ret_tp = traceproc_register_callbacks(&callbacks, &id);
  ck_assert_int_eq(ret_tp, 0);

  create_pub_syn();

  const Statement *s = scratch.before.stmts;
  ck_assert_int_eq(s->type, CREATE);
  ck_assert_int_eq(s->number_of_params, 0);
  ck_assert_int_eq(s->number_of_params_in, 0);
  ck_assert_int_eq(s->iterations, 1);
  ck_assert_str_eq(s->text, "create PUBLIC SYNONYM traceproc_pub_syn FOR traceproc_tbl");
  ck_assert_int_eq(s->length, strlen(s->text));

  s = scratch.after.stmts;
  ck_assert_int_eq(s->type, CREATE);
  ck_assert_int_eq(s->errorcode, -1031);

  ret_tp = traceproc_unregister_callbacks(id);
  ck_assert_int_eq(ret_tp, 0);
}
END_TEST

static void setup()
{
  int ret = proc_connect();
  ck_assert_int_eq(ret, 0);
}

static void teardown()
{
  int ret = proc_disconnect_commit();
  ck_assert_int_eq(ret, 0);
}

TCase *proc_basic_tc_create()
{
  TCase *tc = tcase_create("timespec");
  // alternative: tcase_add_unchecked_fixture ...
  tcase_add_checked_fixture(tc, setup, teardown);
  tcase_add_test(tc, connect_01);
  tcase_add_test(tc, select_01);
  tcase_add_test(tc, select_charz);
  tcase_add_test(tc, select_varchar);
  tcase_add_test(tc, char_01);
  tcase_add_test(tc, syn_01);
  tcase_add_test(tc, syn_02);
  return tc;
}



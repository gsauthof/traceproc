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


#include "test_parsesql.h"

#include <parsesql.h>

#include <stdlib.h>
#include <string.h>

// XXX local to lib
size_t sql_count_host_vars(const char *s);

START_TEST(count_01)
{
  size_t ret = sql_count_host_vars(":foo:bar, :x:y, :z");
  ck_assert_int_eq(ret, 3);
}
END_TEST

START_TEST(count_02)
{
  size_t ret = sql_count_host_vars("");
  ck_assert_int_eq(ret, 0);
}
END_TEST

START_TEST(count_03)
{
  size_t ret = sql_count_host_vars(":a ':a' :y");
  ck_assert_int_eq(ret, 2);
}
END_TEST

START_TEST(count_04)
{
  size_t ret = sql_count_host_vars("''");
  ck_assert_int_eq(ret, 0);
}
END_TEST

START_TEST(token_01)
{
  const char inp[] = "insert into example_tbl (str,n,f) values (:b0,:b1:b2,:b3) returning n into :b4:b5";
  SQL_Token *tokens = 0;
  size_t size = 0;
  int ret = sql_tokenize(inp, &tokens, &size);
  ck_assert_int_eq(ret, 0);
  ck_assert_int_eq(size, 10);
  struct st {
    const char *a;
    bool b;
  };
  typedef struct st st;
  const st src[] = {
    { "insert into example_tbl (str,n,f) values (", false},
    { ":b0", true },
    { ",", false },
    { ":b1:b2", true },
    { ",", false },
    { ":b3", true },
    { ") returning n into ", false },
    { ":b4:b5", true },
    { "", false },
    { 0, 0}
  };
  for (size_t i = 0; i<9; ++i) {
    ck_assert(tokens[i].host_var == src[i].b);
    char b[128] = {0};
    strncpy(b, tokens[i].begin, tokens[i].end - tokens[i].begin);
    ck_assert_str_eq(b, src[i].a);
  }
  ck_assert(!tokens[9].begin);
  ck_assert(!tokens[9].end);
  free(tokens);
}
END_TEST

START_TEST(token_02)
{
  const char inp[] = "";
  SQL_Token *tokens = 0;
  size_t size = 0;
  int ret = sql_tokenize(inp, &tokens, &size);
  ck_assert_int_eq(ret, 0);
  ck_assert(size>0);
  ck_assert(tokens[0].end - tokens[0].begin == 0);
  ck_assert(!*tokens[0].begin);
  ck_assert(!*tokens[0].end);
  ck_assert(!tokens[1].begin);
  ck_assert(!tokens[1].end);
  free(tokens);
}
END_TEST

START_TEST(token_03)
{
  const char inp[] = "insert into tbl (a, b) values (':x, :y, :z', :x:z);";
  SQL_Token *tokens = 0;
  size_t size = 0;
  int ret = sql_tokenize(inp, &tokens, &size);
  ck_assert_int_eq(ret, 0);
  ck_assert(size>3);
  struct st {
    const char *a;
    bool b;
  };
  typedef struct st st;
  const st src[] = {
    { "insert into tbl (a, b) values (':x, :y, :z', ", false },
    { ":x:z", true },
    { ");", false },
    { 0, 0}
  };
  for (size_t i = 0; i<3; ++i) {
    ck_assert(tokens[i].host_var == src[i].b);
    char b[128] = {0};
    strncpy(b, tokens[i].begin, tokens[i].end - tokens[i].begin);
    ck_assert_str_eq(b, src[i].a);
  }
  ck_assert(!tokens[3].begin);
  ck_assert(!tokens[3].end);
  free(tokens);
}
END_TEST

START_TEST(token_04)
{
  const char inp[] = "insert into tbl (a, b) values (':x, :y, :z, :x:z);";
  SQL_Token *tokens = 0;
  size_t size = 0;
  int ret = sql_tokenize(inp, &tokens, &size);
  ck_assert_int_eq(ret, 0);
  char b[128] = {0};
  strncpy(b, tokens[0].begin, tokens[0].end - tokens[0].begin);
  ck_assert_str_eq(b, inp);
  ck_assert(!tokens[1].begin);
  ck_assert(!tokens[1].end);
  ck_assert(size>1);
  free(tokens);
}
END_TEST

START_TEST(token_05)
{
  const char inp[] = "'foo bar'";
  SQL_Token *tokens = 0;
  size_t size = 0;
  int ret = sql_tokenize(inp, &tokens, &size);
  ck_assert_int_eq(ret, 0);
  char b[128] = {0};
  strncpy(b, tokens[0].begin, tokens[0].end - tokens[0].begin);
  ck_assert_str_eq(b, inp);
  ck_assert(!tokens[1].begin);
  ck_assert(!tokens[1].end);
  ck_assert(size>1);
  free(tokens);
}
END_TEST

START_TEST(token_06)
{
  const char inp[] = ":x:y";
  SQL_Token *tokens = 0;
  size_t size = 0;
  int ret = sql_tokenize(inp, &tokens, &size);
  ck_assert_int_eq(ret, 0);
  char b[128] = {0};
  strncpy(b, tokens[0].begin, tokens[0].end - tokens[0].begin);
  ck_assert_str_eq(b, inp);
  ck_assert(tokens[0].host_var);
  ck_assert(!tokens[2].begin);
  ck_assert(!tokens[2].end);
  ck_assert(size>1);
  free(tokens);
}
END_TEST

START_TEST(token_07)
{
  const char inp[] = "foo :xyz_id bar in :ab_id:ab_id_ind blah);";
  SQL_Token *tokens = 0;
  size_t size = 0;
  int ret = sql_tokenize(inp, &tokens, &size);
  ck_assert_int_eq(ret, 0);
  ck_assert(size>3);
  struct st {
    const char *a;
    bool b;
  };
  typedef struct st st;
  const st src[] = {
    { "foo ", false },
    { ":xyz_id", true },
    { " bar in ", false },
    { ":ab_id:ab_id_ind", true },
    { " blah);", false },
    { 0, 0}
  };
  for (size_t i = 0; i<5; ++i) {
    ck_assert(tokens[i].host_var == src[i].b);
    char b[128] = {0};
    strncpy(b, tokens[i].begin, tokens[i].end - tokens[i].begin);
    ck_assert_str_eq(b, src[i].a);
  }
  ck_assert(!tokens[5].begin);
  ck_assert(!tokens[5].end);
  free(tokens);
}
END_TEST

START_TEST(quote_01)
{
  char out[32] = {0};
  int ret = sql_quote_str("abc", 0, out, 32);
  ck_assert_int_eq(ret, 0);
  ck_assert_str_eq(out, "''");
}
END_TEST

START_TEST(quote_02)
{
  char out[32] = {0};
  int ret = sql_quote_str("abc'defg", 4, out, 32);
  ck_assert_int_eq(ret, 0);
  ck_assert_str_eq(out, "'abc'''");
}
END_TEST

START_TEST(quote_03)
{
  char out[32] = {0};
  const char inp[] = "Hello World";
  int ret = sql_quote_str(inp, strlen(inp), out, 32);
  ck_assert_int_eq(ret, 0);
  ck_assert_str_eq(out, "'Hello World'");
}
END_TEST

START_TEST(quote_04)
{
  char out[32] = {0};
  const char inp[] = "''";
  int ret = sql_quote_str(inp, 1, out, 32);
  ck_assert_int_eq(ret, 0);
  ck_assert_str_eq(out, "''''");
}
END_TEST

START_TEST(quote_05)
{
  char out[32] = {0};
  const char inp[] = "''";
  int ret = sql_quote_str(inp, 2, out, 32);
  ck_assert_int_eq(ret, 0);
  ck_assert_str_eq(out, "''''''");
}
END_TEST

START_TEST(pseudo_01)
{
  SQL_Token *tokens = 0;
  size_t size = 0;
  int ret = sql_pseudo_tokenize(3, &tokens, &size);
  ck_assert_int_eq(ret, 0);
  ck_assert(size>7);
  struct st {
    const char *a;
    bool b;
  };
  typedef struct st st;
  const st src[] = {
    { "( ", false },
    { ":XXX", true },
    { ", ", false },
    { ":XXX", true },
    { ", ", false },
    { ":XXX", true },
    { " )", false },
    { 0, 0}
  };
  for (size_t i = 0; i<7; ++i) {
    ck_assert(tokens[i].host_var == src[i].b);
    char b[128] = {0};
    strncpy(b, tokens[i].begin, tokens[i].end - tokens[i].begin);
    ck_assert_str_eq(b, src[i].a);
  }
  ck_assert(!tokens[8].begin);
  ck_assert(!tokens[8].end);
  free(tokens);
}
END_TEST

START_TEST(pseudo_02)
{
  SQL_Token *tokens = 0;
  size_t size = 0;
  int ret = sql_pseudo_tokenize(0, &tokens, &size);
  ck_assert_int_eq(ret, 0);
  ck_assert(size>2);
  struct st {
    const char *a;
    bool b;
  };
  typedef struct st st;
  const st src[] = {
    { "( ", false },
    { " )", false },
    { 0, 0}
  };
  for (size_t i = 0; i<2; ++i) {
    ck_assert(tokens[i].host_var == src[i].b);
    char b[128] = {0};
    strncpy(b, tokens[i].begin, tokens[i].end - tokens[i].begin);
    ck_assert_str_eq(b, src[i].a);
  }
  ck_assert(!tokens[2].begin);
  ck_assert(!tokens[2].end);
  free(tokens);
}
END_TEST

TCase *parsesql_tc_create()
{
  TCase *tc = tcase_create("parsesql");
  tcase_add_test(tc, count_01);
  tcase_add_test(tc, count_02);
  tcase_add_test(tc, count_03);
  tcase_add_test(tc, count_04);
  tcase_add_test(tc, token_01);
  tcase_add_test(tc, token_02);
  tcase_add_test(tc, token_03);
  tcase_add_test(tc, token_04);
  tcase_add_test(tc, token_05);
  tcase_add_test(tc, token_06);
  tcase_add_test(tc, token_07);
  tcase_add_test(tc, quote_01);
  tcase_add_test(tc, quote_02);
  tcase_add_test(tc, quote_03);
  tcase_add_test(tc, quote_04);
  tcase_add_test(tc, quote_05);
  tcase_add_test(tc, pseudo_01);
  tcase_add_test(tc, pseudo_02);
  return tc;
}

